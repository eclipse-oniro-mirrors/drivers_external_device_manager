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
#include "driver_extension_controller.h"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "emit_event_manager.h"
#include "etx_device_mgr.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "usb_device_info.h"

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
    BusExtensionCore::GetInstance().LoadBusExtensionLibs();
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
    delete &(DriverExtensionController::GetInstance());
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
                std::shared_ptr<UsbDeviceInfo> usbDeviceInfo = std::static_pointer_cast<UsbDeviceInfo>(deviceInfo);
                std::shared_ptr<USBDevice> device = std::make_shared<USBDevice>();
                device->busType = usbDeviceInfo->GetBusType();
                device->deviceId = usbDeviceInfo->GetDeviceId();
                device->descripton = usbDeviceInfo->GetDeviceDescription();
                device->productId = usbDeviceInfo->GetProductId();
                device->vendorId = usbDeviceInfo->GetVendorId();
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

UsbErrCode DriverExtMgr::BindDevice(uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    return static_cast<UsbErrCode>(ExtDeviceManager::GetInstance().ConnectDevice(deviceId, connectCallback));
}

UsbErrCode DriverExtMgr::UnBindDevice(uint64_t deviceId)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    return static_cast<UsbErrCode>(ExtDeviceManager::GetInstance().DisConnectDevice(deviceId));
}

UsbErrCode DriverExtMgr::CreateDevice(uint32_t maxX, uint32_t maxY, uint32_t maxPressure)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    return static_cast<UsbErrCode>(EmitEventManager::GetInstance().CreateDevice(maxX, maxY, maxPressure));
}

UsbErrCode DriverExtMgr::EmitEvent(int32_t deviceId, const std::vector<EmitItem> &items)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    return static_cast<UsbErrCode>(EmitEventManager::GetInstance().EmitEvent(deviceId, items));
}

UsbErrCode DriverExtMgr::DestroyDevice(void)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    return static_cast<UsbErrCode>(EmitEventManager::GetInstance().DestroyDevice());
}
} // namespace ExternalDeviceManager
} // namespace OHOS
