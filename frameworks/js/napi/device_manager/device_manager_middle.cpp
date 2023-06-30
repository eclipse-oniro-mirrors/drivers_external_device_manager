/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <unistd.h>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#include <map>
#include <vector>
#include <uv.h>

#include "device_manager_middle.h"

namespace OHOS {
namespace ExternalDeviceManager {
constexpr int32_t PARAM_COUNT_1 = 1;
constexpr int32_t PARAM_COUNT_2 = 2;
constexpr int32_t PARAM_COUNT_3 = 3;
constexpr uint64_t MAX_JS_NUMBER = 9007199254740991;

static const std::map<int32_t, std::string> ERROR_MESSAGES = {
    {SERVICE_EXCEPTION,  "Service exception."},
    {PERMISSION_DENIED,  "Permission denied."},
    {PARAMETER_ERROR,  "The parameter invalid."},
};

static std::mutex mapMutex;
static std::map<uint64_t, sptr<AsyncData>> g_callbackMap = {};
static DriverExtMgrClient &g_edmClient = DriverExtMgrClient::GetInstance();
static sptr<DeviceManagerCallback> g_edmCallback = new (std::nothrow) DeviceManagerCallback {};

static napi_value ConvertToBusinessError(const napi_env &env, const ErrMsg &errMsg);
static napi_value ConvertToJsDeviceId(const napi_env &env, uint64_t deviceId);
static napi_value GetCallbackResult(const napi_env &env, uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj);

static void BindDeviceWorkCb(uv_work_t *work, int status)
{
    if (work == nullptr) {
        return;
    }
    sptr<AsyncData> data(reinterpret_cast<AsyncData *>(work->data));
    data->DecStrongRef(nullptr);
    delete work;

    napi_value result = GetCallbackResult(data->env, data->deviceId, data->drvExtObj);
    napi_value err = ConvertToBusinessError(data->env, data->errMsg);
    if (data->bindCallback != nullptr) {
        napi_value callback;
        napi_get_reference_value(data->env, data->bindCallback, &callback);
        napi_value argv[PARAM_COUNT_2] = {err, result};
        napi_value callResult;
        napi_call_function(data->env, nullptr, callback, PARAM_COUNT_2, argv, &callResult);
        EDM_LOGI(MODULE_DEV_MGR, "bind device callback finish.");
    } else if (data->bindDeferred != nullptr) {
        if (data->errMsg.IsOk()) {
            napi_resolve_deferred(data->env, data->bindDeferred, result);
        } else {
            napi_reject_deferred(data->env, data->bindDeferred, err);
        }
        EDM_LOGI(MODULE_DEV_MGR, "bind device promise finish.");
    }
}

static void UnbindDeviceWorkCb(uv_work_t *work, int status)
{
    if (work == nullptr) {
        return;
    }
    sptr<AsyncData> data(reinterpret_cast<AsyncData *>(work->data));
    data->DecStrongRef(nullptr);
    delete work;

    napi_value err = ConvertToBusinessError(data->env, data->errMsg);
    napi_value result = ConvertToJsDeviceId(data->env, data->deviceId);
    if (data->unbindCallback != nullptr) {
        napi_value callback;
        NAPI_CALL_RETURN_VOID(data->env, napi_get_reference_value(data->env, data->unbindCallback, &callback));

        napi_value argv[PARAM_COUNT_2] = {err, result};
        napi_value callResult;
        napi_call_function(data->env, nullptr, callback, PARAM_COUNT_2, argv, &callResult);
        EDM_LOGI(MODULE_DEV_MGR, "unbind device callback finish.");
    } else if (data->bindDeferred != nullptr) {
        if (data->errMsg.IsOk()) {
            napi_resolve_deferred(data->env, data->unbindDeferred, result);
        } else {
            napi_reject_deferred(data->env, data->unbindDeferred, err);
        }
        EDM_LOGI(MODULE_DEV_MGR, "unbind device promise finish.");
    }
}

void DeviceManagerCallback::OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg)
{
    EDM_LOGE(MODULE_DEV_MGR, "bind device callback: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device: %{public}lu OnConnect is null", deviceId);
        return;
    }

    auto asyncData = g_callbackMap[deviceId];
    if (!errMsg.IsOk()) {
        g_callbackMap.erase(deviceId);
    }
    uv_loop_t* loop = nullptr;
    NAPI_CALL_RETURN_VOID(asyncData->env, napi_get_uv_event_loop(asyncData->env, &loop));
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "new work fail");
        return;
    }
    asyncData->drvExtObj = drvExtObj;
    asyncData->errMsg = errMsg;
    asyncData->IncStrongRef(nullptr);
    work->data = asyncData.GetRefPtr();
    auto ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, BindDeviceWorkCb);
    if (ret != 0) {
        delete work;
        asyncData->DecStrongRef(nullptr);
    }
}

void DeviceManagerCallback::OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGE(MODULE_DEV_MGR, "device onDisconnect: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device: %{public}lu onDisconnect map is null", deviceId);
        return;
    }

    auto asyncData = g_callbackMap[deviceId];
    g_callbackMap.erase(deviceId);
    if (asyncData->onDisconnect == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "device: %{public}lu onDisconnect is null", deviceId);
        return;
    }
    uv_loop_t* loop = nullptr;
    NAPI_CALL_RETURN_VOID(asyncData->env, napi_get_uv_event_loop(asyncData->env, &loop));
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "new work fail");
        return;
    }
    asyncData->errMsg = errMsg;
    asyncData->IncStrongRef(nullptr);
    work->data = asyncData.GetRefPtr();
    auto ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        if (work == nullptr) {
            return;
        }
        sptr<AsyncData> data(reinterpret_cast<AsyncData*>(work->data));
        data->DecStrongRef(nullptr);
        delete work;
        
        napi_value callback;
        NAPI_CALL_RETURN_VOID(data->env, napi_get_reference_value(data->env, data->onDisconnect, &callback));

        napi_value err = ConvertToBusinessError(data->env, data->errMsg);
        napi_value result = ConvertToJsDeviceId(data->env, data->deviceId);
        napi_value argv[PARAM_COUNT_2] = {err, result};
        napi_value callResult;
        napi_call_function(data->env, nullptr, callback, PARAM_COUNT_2, argv, &callResult);
        EDM_LOGI(MODULE_DEV_MGR, "onDisconnect callback finish.");
    });
    if (ret != 0) {
        delete work;
        asyncData->DecStrongRef(nullptr);
    }
}

void DeviceManagerCallback::OnUnBind(uint64_t deviceId, const ErrMsg &errMsg)
{
    EDM_LOGI(MODULE_DEV_MGR, "unbind device callback: %{public}016" PRIX64, deviceId);
    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        EDM_LOGE(MODULE_DEV_MGR, "device unbind map is null");
        return;
    }

    auto asyncData = g_callbackMap[deviceId];
    g_callbackMap.erase(deviceId);
    if (asyncData->unbindCallback == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "device unbind is null");
        return;
    }
    uv_loop_t* loop = nullptr;
    NAPI_CALL_RETURN_VOID(asyncData->env, napi_get_uv_event_loop(asyncData->env, &loop));
    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "new work fail");
        return;
    }
    asyncData->errMsg = errMsg;
    asyncData->IncStrongRef(nullptr);
    work->data = asyncData.GetRefPtr();
    auto ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, UnbindDeviceWorkCb);
    if (ret != 0) {
        delete work;
        asyncData->DecStrongRef(nullptr);
    }
}

static bool IsMatchType(const napi_env &env, const napi_value &value, const napi_valuetype &type)
{
    napi_valuetype paramType = napi_undefined;
    napi_typeof(env, value, &paramType);
    return paramType == type;
}

static napi_value GetCallbackResult(const napi_env &env, uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj)
{
    napi_value id = ConvertToJsDeviceId(env, deviceId);

    napi_value remoteObj;
    if (drvExtObj == nullptr) {
        napi_get_undefined(env, &remoteObj);
        EDM_LOGE(MODULE_DEV_MGR, "Remote obj is null.");
    } else {
        napi_create_object(env, &remoteObj);
        drvExtObj->IncStrongRef(nullptr);
        auto drvExtPtr = drvExtObj.GetRefPtr();
        napi_wrap(env, remoteObj, drvExtPtr, [](napi_env env, void *data, void *hint) {
            sptr<IRemoteObject> drvExt(reinterpret_cast<IRemoteObject*>(data));
            drvExt->DecStrongRef(nullptr);
        },
        nullptr, nullptr);
    }

    napi_value result;
    napi_create_object(env, &result);
    napi_set_named_property(env, result, "deviceId", id);
    napi_set_named_property(env, result, "remote", remoteObj);

    return result;
}

static napi_value CreateBusinessError(const napi_env &env, const int32_t errCode, const std::string &errMessage)
{
    napi_value businessError = nullptr;
    napi_value code = nullptr;
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &code));
    NAPI_CALL(env, napi_create_string_utf8(env, errMessage.c_str(), NAPI_AUTO_LENGTH, &msg));
    napi_create_error(env, nullptr, msg, &businessError);
    napi_set_named_property(env, businessError, "code", code);
    return businessError;
}

static napi_value ConvertToBusinessError(const napi_env &env, const ErrMsg &errMsg)
{
    napi_value businessError = nullptr;
    if (errMsg.IsOk()) {
        napi_get_undefined(env, &businessError);
        return businessError;
    }

    napi_value code = nullptr;
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, SERVICE_EXCEPTION, &code));
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.msg.c_str(), NAPI_AUTO_LENGTH, &msg));
    napi_create_error(env, nullptr, msg, &businessError);
    napi_set_named_property(env, businessError, "code", code);
    return businessError;
}

static napi_value ConvertToJsDeviceId(const napi_env &env, uint64_t deviceId)
{
    napi_value result;
    if (deviceId > MAX_JS_NUMBER) {
        NAPI_CALL(env, napi_create_bigint_uint64(env, deviceId, &result));
    } else {
        NAPI_CALL(env, napi_create_int64(env, deviceId, &result));
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

void ThrowErr(const napi_env &env, const int32_t errCode, const std::string &printMsg)
{
    EDM_LOGE(MODULE_DEV_MGR, "message: %{public}s, code: %{public}d", printMsg.c_str(), errCode);
    auto msg = GetNapiError(errCode);
    if (!msg) {
        EDM_LOGE(MODULE_DEV_MGR, "errCode: %{public}d is invalid", errCode);
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    napi_value error = CreateBusinessError(env, errCode, msg.value());
    napi_throw(env, error);
    napi_close_handle_scope(env, scope);
}

static napi_value ConvertDeviceToJsDevice(napi_env& env, std::shared_ptr<DeviceData> device)
{
    napi_value object;
    napi_value value;
    NAPI_CALL(env, napi_create_object(env, &object));
    NAPI_CALL(env, napi_create_int32(env, device->busType, &value));
    NAPI_CALL(env, napi_set_named_property(env, object, "busType", value));
    value = ConvertToJsDeviceId(env, device->deviceId);
    NAPI_CALL(env, napi_set_named_property(env, object, "deviceId", value));
    if (device->busType == BusType::BUS_TYPE_USB) {
        std::shared_ptr<USBDevice> usb = std::static_pointer_cast<USBDevice>(device);
        NAPI_CALL(env, napi_create_uint32(env, usb->vendorId, &value));
        NAPI_CALL(env, napi_set_named_property(env, object, "vendorId", value));
        NAPI_CALL(env, napi_create_uint32(env, usb->productId, &value));
        NAPI_CALL(env, napi_set_named_property(env, object, "productId", value));
    }

    return object;
}

static napi_value QueryDevices(napi_env env, napi_callback_info info)
{
    EDM_LOGI(MODULE_DEV_MGR, "queryDevices start");
    size_t argc = PARAM_COUNT_1;
    napi_value argv[PARAM_COUNT_1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    int32_t busType = BusType::BUS_TYPE_USB;
    if (argc > 0 && IsMatchType(env, argv[0], napi_number)) {
        NAPI_CALL(env, napi_get_value_int32(env, argv[0], &busType));
        EDM_LOGI(MODULE_DEV_MGR, "bus type is %{public}d", busType);
    }

    std::vector<std::shared_ptr<DeviceData>> devices;
    if (g_edmClient.QueryDevice(busType, devices) != UsbErrCode::EDM_OK) {
        ThrowErr(env, PARAMETER_ERROR, "Query device service fail");
        return nullptr;
    }

    napi_value resultArray;
    NAPI_CALL(env, napi_create_array(env, &resultArray));
    for (size_t index = 0; index < devices.size(); index++) {
        napi_value element = ConvertDeviceToJsDevice(env, devices[index]);
        NAPI_CALL(env, napi_set_element(env, resultArray, index, element));
    }
    EDM_LOGI(MODULE_DEV_MGR, "device count:%{public}" PRIu32, devices.size());

    return resultArray;
}

static bool ParseDeviceId(const napi_env& env, const napi_value& value, uint64_t* deviceId)
{
    napi_valuetype type;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), false);
    if (type == napi_bigint) {
        bool lossless;
        NAPI_CALL_BASE(env, napi_get_value_bigint_uint64(env, value, deviceId, &lossless), false);
    } else if (type == napi_number) {
        int64_t temp;
        NAPI_CALL_BASE(env, napi_get_value_int64(env, value, &temp), false);
        *deviceId = static_cast<uint64_t>(temp);
    } else {
        return false;
    }
    return true;
}

static napi_value BindDevice(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_COUNT_3;
    napi_value argv[PARAM_COUNT_3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < PARAM_COUNT_2) {
        ThrowErr(env, PARAMETER_ERROR, "bindDevice parameter count not match");
        return nullptr;
    }

    uint64_t deviceId;
    if (!ParseDeviceId(env, argv[0], &deviceId)) {
        ThrowErr(env, PARAMETER_ERROR, "deviceid type error");
        return nullptr;
    }
    EDM_LOGI(MODULE_DEV_MGR, "Enter bindDevice:%{public}016" PRIX64, deviceId);
 
    if (!IsMatchType(env, argv[1], napi_function)) {
        ThrowErr(env, PARAMETER_ERROR, "onDisconnect param is error");
        return nullptr;
    }

    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_edmClient.BindDevice(deviceId, g_edmCallback) != UsbErrCode::EDM_OK) {
        ThrowErr(env, SERVICE_EXCEPTION, "bindDevice service failed");
        return nullptr;
    }

    sptr<AsyncData> data = new (std::nothrow) AsyncData {};
    if (data == nullptr) {
        ThrowErr(env, PARAMETER_ERROR, "malloc callback data fail");
        return nullptr;
    }
    data->env = env;
    data->deviceId = deviceId;
    NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &data->onDisconnect));
    napi_value promise = nullptr;
    if (argc > PARAM_COUNT_2 && IsMatchType(env, argv[PARAM_COUNT_2], napi_function)) {
        NAPI_CALL(env, napi_create_reference(env, argv[PARAM_COUNT_2], 1, &data->bindCallback));
    } else {
        NAPI_CALL(env, napi_create_promise(env, &data->bindDeferred, &promise));
    }
    g_callbackMap[data->deviceId] = data;

    return promise;
}

static napi_value UnbindDevice(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_COUNT_2;
    napi_value argv[PARAM_COUNT_2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < PARAM_COUNT_1) {
        ThrowErr(env, PARAMETER_ERROR, "Param count error");
        return nullptr;
    }

    uint64_t deviceId;
    if (!ParseDeviceId(env, argv[0], &deviceId)) {
        ThrowErr(env, PARAMETER_ERROR, "deviceid type error");
        return nullptr;
    }
    EDM_LOGI(MODULE_DEV_MGR, "Enter unbindDevice:%{public}016" PRIX64, deviceId);

    std::lock_guard<std::mutex> mapLock(mapMutex);
    if (g_callbackMap.count(deviceId) == 0) {
        ThrowErr(env, PARAMETER_ERROR, "unbind map is null");
        return nullptr;
    }

    if (g_edmClient.UnBindDevice(deviceId) != UsbErrCode::EDM_OK) {
        ThrowErr(env, SERVICE_EXCEPTION, "unbindDevice service failed");
        return nullptr;
    }
    auto data = g_callbackMap[deviceId];
    napi_value promise = nullptr;
    if (argc > PARAM_COUNT_1 && IsMatchType(env, argv[1], napi_function)) {
        NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &data->unbindCallback));
    } else {
        NAPI_CALL(env, napi_create_promise(env, &data->unbindDeferred, &promise));
    }

    return promise;
}

static napi_value EnumBusTypeConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data));
    return thisArg;
}

static void CreateEnumBusType(napi_env env, napi_value exports)
{
    napi_value usb = nullptr;
    napi_create_int32(env, 1, &usb);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("USB", usb),
    };

    napi_value result = nullptr;
    napi_define_class(env, "BusType", NAPI_AUTO_LENGTH, EnumBusTypeConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);
    napi_set_named_property(env, exports, "BusType", result);
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value ExtDeviceManagerInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("queryDevices", QueryDevices),
        DECLARE_NAPI_FUNCTION("bindDevice", BindDevice),
        DECLARE_NAPI_FUNCTION("unbindDevice", UnbindDevice),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    CreateEnumBusType(env, exports);

    return exports;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_moduleManager = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ExtDeviceManagerInit,
    .nm_modname = "driver.deviceManager",
    .nm_priv = nullptr,
    .reserved = {nullptr}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_moduleManager);
}
}
}