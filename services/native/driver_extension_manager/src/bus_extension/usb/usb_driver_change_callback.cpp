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

#include "usb_driver_change_callback.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "usb_driver_info.h"

namespace OHOS {
namespace ExternalDeviceManager {
void UsbDriverChangeCallback::OnDriverMatched(const std::shared_ptr<DriverInfo> &driverInfo)
{
    if (driverInfo == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "driverInfo is nullptr");
        return;
    }
    if (this->iUsbDdk_ == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "iUsbDdk_ is nullptr");
        return;
    }
    V1_1::DriverAbilityInfo driverAbilityInfo;
    driverAbilityInfo.driverUid = driverInfo->GetDriverUid();
    std::shared_ptr<UsbDriverInfo> usbDriverInfo = std::static_pointer_cast<UsbDriverInfo>(driverInfo->GetInfoExt());
    if (usbDriverInfo == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "usbDriverInfo is nullptr");
        return;
    }
    driverAbilityInfo.vids = usbDriverInfo->GetVendorIds();
    this->iUsbDdk_->UpdateDriverInfo(driverAbilityInfo);
}

void UsbDriverChangeCallback::OnDriverRemoved(const std::shared_ptr<DriverInfo> &driverInfo)
{
    if (driverInfo == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "driverInfo is nullptr");
        return;
    }
    if (this->iUsbDdk_ == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "iUsbDdk_ is nullptr");
        return;
    }
    this->iUsbDdk_->RemoveDriverInfo(driverInfo->GetDriverUid());
}
} // namespace ExternalDeviceManager
} // namespace OHOS