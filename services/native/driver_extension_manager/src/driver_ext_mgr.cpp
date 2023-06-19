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

#include "driver_ext_mgr.h"
#include "bus_extension_core.h"
#include "dev_change_callback.h"
#include "driver_ext_mgr_callback_death_recipient.h"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "etx_device_mgr.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace ExternalDeviceManager {
const bool G_REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DriverExtMgr>::GetInstance().get());

DriverExtMgr::DriverExtMgr() : SystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, true) {}
DriverExtMgr::~DriverExtMgr() {}

void DriverExtMgr::OnStart()
{
    int32_t ret;
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStart");
    ret = DriverPkgManager::GetInstance().Init();
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "DriverPkgManager Init failed %{public}d", ret);
    }
    ret = ExtDeviceManager::GetInstance().Init();
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "ExtDeviceManager Init failed %{public}d", ret);
    }
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    ret = BusExtensionCore::GetInstance().Init(callback);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "BusExtensionCore Init failed %{public}d", ret);
    }
    if (!Publish(AsObject())) {
        EDM_LOGE(MODULE_DEV_MGR, "OnStart register to system ability manager failed.");
        return;
    }
}

void DriverExtMgr::OnStop()
{
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStop");
    delete &(DriverPkgManager::GetInstance());
    delete &(ExtDeviceManager::GetInstance());
    delete &(BusExtensionCore::GetInstance());
}

int DriverExtMgr::Dump(int fd, const std::vector<std::u16string> &args)
{
    return 0;
}

UsbErrCode DriverExtMgr::QueryDevice(uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices)
{
    if (busType == BusType::BUS_TYPE_INVALID) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}d", static_cast<int32_t>(busType));
        return UsbErrCode::EDM_ERR_INVALID_PARAM;
    }

    std::vector<std::shared_ptr<DeviceInfo>> deviceInfos =
        ExtDeviceManager::GetInstance().QueryDevice(static_cast<BusType>(busType));
    for (const auto &deviceInfo : deviceInfos) {
        switch (deviceInfo->GetBusType()) {
            case BusType::BUS_TYPE_USB: {
                std::shared_ptr<USBDevice> device = std::make_shared<USBDevice>();
                device->busType = deviceInfo->GetBusType();
                device->deviceId = deviceInfo->GetDeviceId();
                device->descripton = deviceInfo->GetDeviceDescription();
                device->productId = "";
                device->vendorId = "";
                devices.push_back(device);
                break;
            }
            default: {
                break;
            }
        }
    }

    return UsbErrCode::EDM_OK;
}

static bool RegisteDeathRecipient(const sptr<IDriverExtMgrCallback> &connectCallback)
{
    sptr<DriverExtMgrCallbackDeathRecipient> deathRecipient = new DriverExtMgrCallbackDeathRecipient();
    return connectCallback->AsObject()->AddDeathRecipient(deathRecipient);
}

UsbErrCode DriverExtMgr::BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(connectCallbackMutex);
    auto connectCallbackListIter = connectCallbackMap.find(deviceId);
    if (connectCallbackListIter == connectCallbackMap.end()) {
        if (!RegisteDeathRecipient(connectCallback)) {
            EDM_LOGE(MODULE_DEV_MGR, "%{public}s failed to add death recipient", __func__);
            return UsbErrCode::EDM_NOK;
        }
        connectCallbackMap[deviceId].push_back(connectCallback);
        connectCallback->OnConnect(0, nullptr, {UsbErrCode::EDM_ERR_NOT_SUPPORT, ""});
        connectCallback->OnDisconnect(0, {UsbErrCode::EDM_ERR_NOT_SUPPORT, ""});
        return UsbErrCode::EDM_OK;
    }

    auto &connectCallbackList = connectCallbackListIter->second;
    auto iter = std::find_if(connectCallbackList.begin(), connectCallbackList.end(),
        [&connectCallback](const sptr<IDriverExtMgrCallback> &element) {
            return connectCallback->AsObject() == element->AsObject();
        });

    if (iter != connectCallbackList.end()) {
        (*iter)->OnConnect(0, nullptr, {UsbErrCode::EDM_ERR_NOT_SUPPORT, ""});
        (*iter)->OnDisconnect(0, {UsbErrCode::EDM_ERR_NOT_SUPPORT, ""});
        return UsbErrCode::EDM_OK;
    }

    if (!RegisteDeathRecipient(connectCallback)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s failed to add death recipient", __func__);
        return UsbErrCode::EDM_NOK;
    }
    connectCallbackList.push_back(connectCallback);
    connectCallback->OnConnect(0, nullptr, {UsbErrCode::EDM_ERR_NOT_SUPPORT, ""});
    connectCallback->OnDisconnect(0, {UsbErrCode::EDM_ERR_NOT_SUPPORT, ""});
    return UsbErrCode::EDM_OK;
}

UsbErrCode DriverExtMgr::UnBindDevice(uint64_t deviceId)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);

    std::lock_guard<std::mutex> lock(connectCallbackMutex);
    auto connectCallbackListIter = connectCallbackMap.find(deviceId);
    if (connectCallbackListIter == connectCallbackMap.end()) {
        return UsbErrCode::EDM_ERR_INVALID_OBJECT;
    }

    auto &connectCallbackList = connectCallbackListIter->second;
    for (const auto &connectCallback : connectCallbackList) {
        connectCallback->OnUnBind(deviceId, {UsbErrCode::EDM_ERR_NOT_SUPPORT, ""});
    }
    connectCallbackMap.erase(deviceId);

    return UsbErrCode::EDM_OK;
}

void DriverExtMgr::DeleteConnectCallback(const wptr<IRemoteObject> &remote)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);

    std::lock_guard<std::mutex> lock(connectCallbackMutex);
    for (auto &connectCallbackListPair : connectCallbackMap) {
        auto &connectCallbackList = connectCallbackListPair.second;
        auto iter = std::find_if(connectCallbackList.begin(), connectCallbackList.end(),
            [&remote](const sptr<IDriverExtMgrCallback> &element) {
                return remote == element->AsObject();
            });
        if (iter != connectCallbackList.end()) {
            connectCallbackList.erase(iter);
            break;
        }
    }
}
} // namespace ExternalDeviceManager
} // namespace OHOS
