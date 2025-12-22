/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>
#include <map>

#include "ohos.driver.deviceManager.impl.hpp"
#include "remote_object_taihe_ani.h"
#include "ohos.driver.deviceManager.impl.h"
#include "ohos.driver.deviceManager.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"
#include "edm_errors.h"

using namespace taihe;
using namespace ohos::driver::deviceManager;
namespace OHOS {
namespace ExternalDeviceManager {
constexpr int32_t ANI_SCOPE_SIZE = 16;
constexpr uint16_t TWO_PARAMETERS = 2;

static const std::map<int32_t, std::string> ERROR_MESSAGES = {
    {SERVICE_EXCEPTION,  "ExternalDeviceManager service exception."},
    {PERMISSION_DENIED,  "Permission denied."},
    {PERMISSION_NOT_SYSTEM_APP,  "Permission denied. A non-system application cannot call a system API."},
    {PARAMETER_ERROR,  "The parameter check failed."},
    {SERVICE_EXCEPTION_NEW, "ExternalDeviceManager service exception."},
    {SERVICE_NOT_ALLOW_ACCESS, "Driver does not allow application access."},
    {SERVICE_NOT_BOUND, "There is no binding relationship between the application and the driver."}
};
static std::mutex mapMutex;
static std::map<uint64_t, OHOS::sptr<AsyncData>> g_callbackMap = {};
static OHOS::ExternalDeviceManager::DriverExtMgrClient &g_edmClient =
    OHOS::ExternalDeviceManager::DriverExtMgrClient::GetInstance();
static OHOS::sptr<DeviceManagerCallback> g_edmCallback = new (std::nothrow) DeviceManagerCallback {};
static thread_local std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler = nullptr;

static bool SendEventToMainThread(const std::function<void()> func)
{
    EDM_LOGD(MODULE_DEV_MGR, "SendEventToMainThread begin");
    if (func == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "func is nullptr!");
        return false;
    }

    if (!mainHandler) {
        auto runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            EDM_LOGE(MODULE_DEV_MGR, "get main event runner failed!");
            return false;
        }
        mainHandler = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    mainHandler->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::HIGH, {});
    EDM_LOGD(MODULE_DEV_MGR, "SendEventToMainThread end");
    return true;
}

void AsyncData::DeleteNapiRef()
{
    if (env == nullptr) {
        return;
    }
    if (onDisconnect) {
        ani_env *env_now;
        vm->GetEnv(ANI_VERSION_1, &env_now);
        ani_status ret = env_now->GlobalReference_Delete(onDisconnect);
        if (ret != ANI_OK) {
            EDM_LOGE(MODULE_DEV_MGR, "GlobalReference_Delete failed. ret=%{public}d", ret);
        }
        env = nullptr;
        vm = nullptr;
        onDisconnect = nullptr;
    }
}

static ani_object GetCallbackResult(ani_env *env, uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj)
{
    ani_long id = deviceId;
    ani_ref remoteObj;
    if (drvExtObj == nullptr) {
        env->GetUndefined(&remoteObj);
        EDM_LOGE(MODULE_DEV_MGR, "Remote obj is null.");
    } else {
        EDM_LOGI(MODULE_DEV_MGR, "Remote obj create.");
        remoteObj = ANI_ohos_rpc_CreateJsRemoteObject(env, drvExtObj);
    }

    ani_object result {};

    ani_class cls;
    const char *clsName = "@ohos.driver.deviceManager.deviceManager.RemoteDeviceDriver_inner";
    if (ANI_OK != env->FindClass(clsName, &cls)) {
        EDM_LOGE(MODULE_DEV_MGR, "FindClass '%{public}s' failed", clsName);
        return result;
    }

    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        EDM_LOGE(MODULE_DEV_MGR, "Class_FindMethod 'constructor' failed");
        return result;
    }

    if (ANI_OK != env->Object_New(cls, ctor, &result, id, remoteObj)) {
        EDM_LOGE(MODULE_DEV_MGR, "Object_New failed");
        return result;
    }
    return result;
}

static std::optional<std::string> GetNapiError(int32_t errorCode)
{
    auto iter = ERROR_MESSAGES.find(errorCode);
    if (iter != ERROR_MESSAGES.end()) {
        return iter->second;
    }
    return std::nullopt;
}

static ani_object ConvertToBusinessError(ani_env *env, const ErrMsg &errMsg)
{
    ani_ref businessError = nullptr;
    env->GetUndefined(&businessError);
    if (errMsg.IsOk()) {
        return reinterpret_cast<ani_object>(businessError);
    }

    auto msgString = GetNapiError(SERVICE_EXCEPTION);
    if (!msgString) {
        return reinterpret_cast<ani_object>(businessError);
    }

    ani_object errorObject = nullptr;
    EDM_LOGD(MODULE_DEV_MGR, "Begin ThrowBusinessError.");
    static const char *errorClsName = "@ohos.base.BusinessError";
    ani_class cls {};
    if (ANI_OK != env->FindClass(errorClsName, &cls)) {
        EDM_LOGE(MODULE_DEV_MGR, "find class BusinessError %{public}s failed", errorClsName);
        return reinterpret_cast<ani_object>(businessError);
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", ":", &ctor)) {
        EDM_LOGE(MODULE_DEV_MGR, "find method BusinessError.constructor failed");
        return reinterpret_cast<ani_object>(businessError);
    }

    if (ANI_OK != env->Object_New(cls, ctor, &errorObject)) {
        EDM_LOGE(MODULE_DEV_MGR, "create BusinessError object failed");
        return reinterpret_cast<ani_object>(businessError);
    }

    ani_double aniErrCode = static_cast<ani_double>(SERVICE_EXCEPTION);
    ani_string errMsgStr;
    if (ANI_OK != env->String_NewUTF8(msgString->c_str(), msgString->size(), &errMsgStr)) {
        EDM_LOGE(MODULE_DEV_MGR, "convert errMsg to ani_string failed");
        return errorObject;
    }
    if (ANI_OK != env->Object_SetFieldByName_Double(errorObject, "code", aniErrCode)) {
        EDM_LOGE(MODULE_DEV_MGR, "set error code failed");
        return errorObject;
    }
    if (ANI_OK != env->Object_SetPropertyByName_Ref(errorObject, "message", errMsgStr)) {
        EDM_LOGE(MODULE_DEV_MGR, "set error message failed");
        return errorObject;
    }

    return errorObject;
}

static ani_object ConvertToObjectDeviceId(ani_env *env, const uint64_t deviceId)
{
    ani_object retObject = nullptr;
    ani_long aniDeviceId = deviceId;
    ani_class cls {};
    if (ANI_OK != env->FindClass("std.core.Long", &cls)) {
        EDM_LOGE(MODULE_DEV_MGR, "find class Long Lstd/core/Long failed");
        return retObject;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "l:", &ctor)) {
        EDM_LOGE(MODULE_DEV_MGR, "find method Long.constructor failed");
        return retObject;
    }
    if (ANI_OK != env->Object_New(cls, ctor, &retObject, aniDeviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "create Long object failed");
    }
    return retObject;
}

ErrCode DeviceManagerCallback::OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg)
{
    EDM_LOGI(MODULE_DEV_MGR, "bind device callback: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device OnConnect is null");
        return EDM_NOK;
    }

    auto data = g_callbackMap[deviceId];
    if (!errMsg.IsOk()) {
        g_callbackMap.erase(deviceId);
    }
    auto task = [data, drvExtObj, errMsg]() {
        EDM_LOGE(MODULE_DEV_MGR, "OnConnect async task is run.");
        ani_env *env = nullptr;
        ani_options aniArgs {0, nullptr};
        int32_t ret = data->vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env);
        if (ANI_ERROR == ret) {
            if (ANI_OK != data->vm->GetEnv(ANI_VERSION_1, &env)) {
                EDM_LOGE(MODULE_DEV_MGR, "GetEnv failed");
            }
        }
        if (ANI_OK != env->CreateLocalScope(ANI_SCOPE_SIZE)) {
            EDM_LOGE(MODULE_DEV_MGR, "CreateLocalScope failed");
            if (ret == ANI_OK) {
                data->vm->DetachCurrentThread();
            }
        }
        ani_object result = GetCallbackResult(env, data->deviceId, drvExtObj);
        ani_object err = ConvertToBusinessError(env, errMsg);
        if (data->bindDeferred != nullptr) {
            if (errMsg.IsOk()) {
                env->PromiseResolver_Resolve(data->bindDeferred, result);
            } else {
                env->PromiseResolver_Reject(data->bindDeferred, reinterpret_cast<ani_error>(err));
            }
            EDM_LOGD(MODULE_DEV_MGR, "bind device promise finish.");
        }
        env->DestroyLocalScope();
        if (ret == ANI_OK) {
            data->vm->DetachCurrentThread();
        }
    };
    if (!SendEventToMainThread(task)) {
        EDM_LOGE(MODULE_DEV_MGR, "OnConnect send event failed.");
        return EDM_NOK;
    }
    return EDM_OK;
}

ErrCode DeviceManagerCallback::OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGI(MODULE_DEV_MGR, "device onDisconnect: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device callback map is null");
        return EDM_NOK;
    }

    auto data = g_callbackMap[deviceId];
    g_callbackMap.erase(deviceId);
    if (data->onDisconnect == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "device callback is null");
        return EDM_NOK;
    }
    data->IncStrongRef(nullptr);
    auto task = [data, errMsg]() {
        EDM_LOGD(MODULE_DEV_MGR, "OnDisconnect async task is run.");
        ani_env *env = nullptr;
        ani_options aniArgs {0, nullptr};
        int32_t ret = data->vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env);
        if (ret != ANI_OK && data->vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
            EDM_LOGE(MODULE_DEV_MGR, "Failed to get JNI environment.");
            data->DecStrongRef(nullptr);
            return;
        }
        if (ANI_OK != env->CreateLocalScope(ANI_SCOPE_SIZE)) {
            EDM_LOGE(MODULE_DEV_MGR, "Failed to create local scope.");
            data->DecStrongRef(nullptr);
            if (ret == ANI_OK) {
                data->vm->DetachCurrentThread();
            }
            return;
        }
        ani_ref argv[] = {ConvertToBusinessError(env, errMsg), ConvertToObjectDeviceId(env, data->deviceId)};
        ani_ref result;
        auto fnObj = reinterpret_cast<ani_fn_object>(data->onDisconnect);
        auto callRet = env->FunctionalObject_Call(fnObj, TWO_PARAMETERS, argv, &result);
        EDM_LOGD(MODULE_DEV_MGR, "OnDisconnect callback finish ret: %{public}u", callRet);
        env->DestroyLocalScope();
        data->DecStrongRef(nullptr);
        if (ret == ANI_OK) {
            data->vm->DetachCurrentThread();
        }
    };
    if (!SendEventToMainThread(task)) {
        EDM_LOGE(MODULE_DEV_MGR, "OnDisconnect send event failed.");
        data->DecStrongRef(nullptr);
        return EDM_NOK;
    }
    return EDM_OK;
}

ErrCode DeviceManagerCallback::OnUnBind(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGI(MODULE_DEV_MGR, "unbind device callback: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device unbind map is null");
        return EDM_NOK;
    }

    auto asyncData = g_callbackMap[deviceId];
    if (asyncData == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "device unbind is null");
        return EDM_NOK;
    }
    asyncData->unBindErrMsg = errMsg;
    return EDM_OK;
}

static ohos::driver::deviceManager::DeviceUnion ConvertToDevice(std::shared_ptr<DeviceData> &deviceData)
{
    if (deviceData->busType == OHOS::ExternalDeviceManager::BusType::BUS_TYPE_USB) {
        std::shared_ptr<OHOS::ExternalDeviceManager::USBDevice> usb =
            std::static_pointer_cast<OHOS::ExternalDeviceManager::USBDevice>(deviceData);
        auto taiheUsbDevice = ohos::driver::deviceManager::USBDevice{
            {
                std::move(ohos::driver::deviceManager::BusType::key_t::USB),
                std::move(deviceData->deviceId),
                std::move(deviceData->descripton)
            },
            std::move(usb->vendorId),
            std::move(usb->productId)
        };
        return ohos::driver::deviceManager::DeviceUnion(
            ::taihe::static_tag<::ohos::driver::deviceManager::DeviceUnion::tag_t::t1>, taiheUsbDevice);
    } else {
        auto taiheDevice = ohos::driver::deviceManager::Device{
            std::move(ohos::driver::deviceManager::BusType::key_t(deviceData->busType)),
            std::move(deviceData->deviceId),
            std::move(deviceData->descripton)
        };
        return ohos::driver::deviceManager::DeviceUnion(
            ::taihe::static_tag<::ohos::driver::deviceManager::DeviceUnion::tag_t::t>, taiheDevice);
    }
}
array<ohos::driver::deviceManager::DeviceUnion> queryDevices(optional_view<int32_t> busType)
{
    EDM_LOGI(MODULE_DEV_MGR, "queryDevices start");
    bool isBusTypeSet = busType.has_value();
    int32_t busTypeVal = isBusTypeSet ? busType.value() : OHOS::ExternalDeviceManager::BusType::BUS_TYPE_USB;
    EDM_LOGI(MODULE_DEV_MGR, "bus type is %{public}d", busTypeVal);
    std::vector<std::shared_ptr<DeviceData>> devices;
    std::vector<ohos::driver::deviceManager::DeviceUnion> resultArray;
    UsbErrCode retCode = g_edmClient.QueryDevice(busTypeVal, devices);
    if (retCode != UsbErrCode::EDM_OK) {
        if (retCode == UsbErrCode::EDM_ERR_NO_PERM) {
            set_business_error(PERMISSION_DENIED, "queryDevice: no permission");
        } else {
            EDM_LOGD(MODULE_DEV_MGR, "queryDevices error code: %{public}d", retCode);
            set_business_error(SERVICE_EXCEPTION, "Query device service fail");
        }
        return array<ohos::driver::deviceManager::DeviceUnion>(resultArray);
    }
    for (auto &deviceItem : devices) {
        resultArray.push_back(ConvertToDevice(deviceItem));
    }
    EDM_LOGI(MODULE_DEV_MGR, "query device finish");
    return array<ohos::driver::deviceManager::DeviceUnion>(resultArray);
}

static ohos::driver::deviceManager::USBInterfaceDesc ParseToUSBInterfaceDesc(
    std::shared_ptr<OHOS::ExternalDeviceManager::USBInterfaceDesc> &usbInterfaceDesc)
{
    return {
        .bInterfaceNumber = usbInterfaceDesc->bInterfaceNumber,
        .bClass = usbInterfaceDesc->bClass,
        .bSubClass = usbInterfaceDesc->bSubClass,
        .bProtocol = usbInterfaceDesc->bProtocol,
    };
}

static ohos::driver::deviceManager::DeviceInfoUnion ConvertToDeviceInfo(std::shared_ptr<DeviceInfoData> &deviceInfoData)
{
    EDM_LOGD(MODULE_DEV_MGR, "ConvertToDeviceInfo start");
    auto busType = DeviceInfoData::GetBusTypeByDeviceId(deviceInfoData->deviceId);
    if (busType == OHOS::ExternalDeviceManager::BusType::BUS_TYPE_USB) {
        std::shared_ptr<USBDeviceInfoData> usbDeviceInfo = std::static_pointer_cast<USBDeviceInfoData>(deviceInfoData);
        optional<string> driverUid;
        if (deviceInfoData->isDriverMatched) {
            driverUid = optional<string>(std::in_place, deviceInfoData->driverUid);
        } else {
            driverUid = optional<string>(std::nullopt);
        }
        std::vector<ohos::driver::deviceManager::USBInterfaceDesc> taiheInterfaceDesc;
        for (auto &interfaceDesc : usbDeviceInfo->interfaceDescList) {
            taiheInterfaceDesc.push_back(ParseToUSBInterfaceDesc(interfaceDesc));
        }
        array<::ohos::driver::deviceManager::USBInterfaceDesc> structDescList(taiheInterfaceDesc);
        auto taiheDeviceInfoData = ohos::driver::deviceManager::USBDeviceInfo{
            {
                std::move(deviceInfoData->deviceId),
                std::move(deviceInfoData->isDriverMatched),
                std::move(driverUid),
            },
            std::move(usbDeviceInfo->vendorId),
            std::move(usbDeviceInfo->productId),
            std::move(structDescList),
        };
        return ohos::driver::deviceManager::DeviceInfoUnion(
            ::taihe::static_tag<::ohos::driver::deviceManager::DeviceInfoUnion::tag_t::t1>, taiheDeviceInfoData);
    } else {
        auto driverUid = deviceInfoData->driverUid.empty() ? optional<string>(std::nullopt) :
            optional<string>(std::in_place_t{}, deviceInfoData->driverUid);
        auto taiheDeviceInfoData = ohos::driver::deviceManager::DeviceInfo{
            std::move(deviceInfoData->deviceId),
            std::move(deviceInfoData->isDriverMatched),
            std::move(driverUid)
        };
        return ohos::driver::deviceManager::DeviceInfoUnion(
            ::taihe::static_tag<::ohos::driver::deviceManager::DeviceInfoUnion::tag_t::t>, taiheDeviceInfoData);
    }
}

array<ohos::driver::deviceManager::DeviceInfoUnion> queryDeviceInfo(optional_view<uint64_t> deviceId)
{
    EDM_LOGD(MODULE_DEV_MGR, "queryDeviceInfo start");
    std::vector<std::shared_ptr<DeviceInfoData>> deviceInfos;
    int32_t ret;
    bool isDeviceIdSet = deviceId.has_value();
    if (isDeviceIdSet) {
        ret = g_edmClient.QueryDeviceInfo(deviceId.value(), deviceInfos);
    } else {
        ret = g_edmClient.QueryDeviceInfo(deviceInfos);
    }
    std::vector<ohos::driver::deviceManager::DeviceInfoUnion> resultArray;
    if (ret != UsbErrCode::EDM_OK) {
        if (ret == UsbErrCode::EDM_ERR_NOT_SYSTEM_APP) {
            set_business_error(PERMISSION_NOT_SYSTEM_APP, "queryDeviceInfo: none system app");
        } else if (ret == UsbErrCode::EDM_ERR_NO_PERM) {
            set_business_error(PERMISSION_DENIED, "queryDeviceInfo: no permission");
        } else {
            set_business_error(SERVICE_EXCEPTION_NEW, "Query device info service fail");
        }
        return array<ohos::driver::deviceManager::DeviceInfoUnion>(resultArray);
    }

    for (auto &deviceInfoItem : deviceInfos) {
        resultArray.push_back(ConvertToDeviceInfo(deviceInfoItem));
    }

    return array<ohos::driver::deviceManager::DeviceInfoUnion>(resultArray);
}

static ohos::driver::deviceManager::DriverInfoUnion ConvertToDriverInfo(std::shared_ptr<DriverInfoData> &driverInfoData)
{
    EDM_LOGD(MODULE_DEV_MGR, "ConvertToDriverInfo start");
    if (driverInfoData->busType == OHOS::ExternalDeviceManager::BusType::BUS_TYPE_USB) {
        std::shared_ptr<USBDriverInfoData> usbDriverInfo = std::static_pointer_cast<USBDriverInfoData>(driverInfoData);

        std::vector<int32_t> pids;
        for (auto pidItem : usbDriverInfo->pids) {
            pids.push_back(pidItem);
        }
        array<int32_t> pidList(pids);

        std::vector<int32_t> vids;
        for (auto vidItem : usbDriverInfo->vids) {
            vids.push_back(vidItem);
        }
        array<int32_t> vidList(vids);

        auto taiheUSBDriverInfoData = ohos::driver::deviceManager::USBDriverInfo{
            {
                std::move(ohos::driver::deviceManager::BusType::key_t::USB),
                std::move(driverInfoData->driverUid),
                std::move(driverInfoData->driverName),
                std::move(driverInfoData->version),
                std::move(driverInfoData->bundleSize),
                std::move(driverInfoData->description),
            },
            std::move(pidList),
            std::move(vidList),
        };
        return ohos::driver::deviceManager::DriverInfoUnion(
            ::taihe::static_tag<::ohos::driver::deviceManager::DriverInfoUnion::tag_t::t1>, taiheUSBDriverInfoData);
    } else {
        auto taiheDriverInfoData = ohos::driver::deviceManager::DriverInfo{
            std::move(ohos::driver::deviceManager::BusType::key_t(driverInfoData->busType)),
            std::move(driverInfoData->driverUid),
            std::move(driverInfoData->driverName),
            std::move(driverInfoData->version),
            std::move(driverInfoData->bundleSize),
            std::move(driverInfoData->description),
        };
        return ohos::driver::deviceManager::DriverInfoUnion(
            ::taihe::static_tag<::ohos::driver::deviceManager::DriverInfoUnion::tag_t::t>, taiheDriverInfoData);
    }
}

array<ohos::driver::deviceManager::DriverInfoUnion> queryDriverInfo(optional_view<string> driverUid)
{
    EDM_LOGD(MODULE_DEV_MGR, "queryDriverInfo start");
    std::vector<std::shared_ptr<DriverInfoData>> driverInfos;
    std::vector<ohos::driver::deviceManager::DriverInfoUnion> resultArray;
    int32_t ret;
    bool isDriverUidSet = driverUid.has_value();
    if (isDriverUidSet) {
        ret = g_edmClient.QueryDriverInfo(driverUid.value().c_str(), driverInfos);
    } else {
        ret = g_edmClient.QueryDriverInfo(driverInfos);
    }

    if (ret != UsbErrCode::EDM_OK) {
        if (ret == UsbErrCode::EDM_ERR_NOT_SYSTEM_APP) {
            set_business_error(PERMISSION_NOT_SYSTEM_APP, "queryDriverInfo: none system app");
        } else if (ret == UsbErrCode::EDM_ERR_NO_PERM) {
            set_business_error(PERMISSION_DENIED, "queryDriverInfo: no permission");
        } else {
            set_business_error(SERVICE_EXCEPTION_NEW, "Query driver info service fail");
        }
        return array<ohos::driver::deviceManager::DriverInfoUnion>(resultArray);
    }

    for (auto &driverInfoItem : driverInfos) {
        resultArray.push_back(ConvertToDriverInfo(driverInfoItem));
    }
    return array<ohos::driver::deviceManager::DriverInfoUnion>(resultArray);
}

ani_object BindDriverWithDeviceIdSync([[maybe_unused]] ani_env *env, ani_long deviceId, ani_object onDisconnect)
{
    EDM_LOGI(MODULE_DEV_MGR, "Enter BindDriverWithDeviceIdSync:%{public}016" PRIX64, static_cast<uint64_t>(deviceId));
    std::lock_guard<std::mutex> mapLock(mapMutex);
    UsbErrCode retCode = g_edmClient.BindDriverWithDeviceId(deviceId, g_edmCallback);
    if (retCode != UsbErrCode::EDM_OK) {
        if (retCode == UsbErrCode::EDM_ERR_NO_PERM) {
            set_business_error(PERMISSION_DENIED, "bindDevice: no permission");
        } else if (retCode == UsbErrCode::EDM_ERR_SERVICE_NOT_ALLOW_ACCESS) {
            set_business_error(SERVICE_NOT_ALLOW_ACCESS, "bindDevice: service not allowed");
        } else {
            set_business_error(SERVICE_EXCEPTION_NEW, "bindDevice service failed");
        }
        return nullptr;
    }

    ani_vm *vm = nullptr;
    if (ANI_OK != env->GetVM(&vm)) {
        EDM_LOGE(MODULE_DEV_MGR, "GetVM failed.");
        return nullptr;
    }
    sptr<AsyncData> data = new (std::nothrow) AsyncData(vm, env);
    if (data == nullptr) {
        set_business_error(PARAMETER_ERROR, "malloc callback data fail");
        return nullptr;
    }

    data->env = env;
    data->deviceId = deviceId;
    if (ANI_OK != env->GlobalReference_Create(reinterpret_cast<ani_ref>(onDisconnect), &data->onDisconnect)) {
        set_business_error(PARAMETER_ERROR, "GlobalReference_Create failed");
        return nullptr;
    }

    ani_object promise;
    env->Promise_New(&data->bindDeferred, &promise);
    g_callbackMap[data->deviceId] = data;
    return promise;
}

int32_t UnbindDriverWithDeviceIdSync(uint64_t deviceId)
{
    EDM_LOGI(MODULE_DEV_MGR, "Enter unbindDevice:%{public}016" PRIX64, deviceId);
    UsbErrCode retCode = g_edmClient.UnbindDriverWithDeviceId(deviceId);
    if (retCode != UsbErrCode::EDM_OK) {
        if (retCode == UsbErrCode::EDM_ERR_NO_PERM) {
            set_business_error(PERMISSION_DENIED, "unbindDevice: no permission");
        } else if (retCode == UsbErrCode::EDM_ERR_SERVICE_NOT_BOUND) {
            set_business_error(SERVICE_NOT_BOUND, "unbindDevice: there is no binding relationship");
        } else {
            set_business_error(SERVICE_EXCEPTION_NEW, "unbindDevice service failed");
        }
    }
    return retCode;
}
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_queryDevices(OHOS::ExternalDeviceManager::queryDevices);
TH_EXPORT_CPP_API_queryDeviceInfo(OHOS::ExternalDeviceManager::queryDeviceInfo);
TH_EXPORT_CPP_API_queryDriverInfo(OHOS::ExternalDeviceManager::queryDriverInfo);
TH_EXPORT_CPP_API_UnbindDriverWithDeviceIdSync(OHOS::ExternalDeviceManager::UnbindDriverWithDeviceIdSync);
// NOLINTEND
