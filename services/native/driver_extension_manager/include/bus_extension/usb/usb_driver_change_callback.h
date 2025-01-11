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
#ifndef USB_DRIVER_CHANGE_CALLBACK_H
#define USB_DRIVER_CHANGE_CALLBACK_H

#include <memory>
#include "ext_object.h"
#include "idriver_change_callback.h"
#include "v1_1/iusb_ddk.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace OHOS::HDI::Usb::Ddk;
class UsbDriverChangeCallback final : public IDriverChangeCallback {
public:
    UsbDriverChangeCallback(sptr<V1_1::IUsbDdk> &iUsbDdk) : iUsbDdk_(iUsbDdk) {}
    virtual void OnDriverMatched(const std::shared_ptr<DriverInfo> &driverInfo) override;
    virtual void OnDriverRemoved(const std::shared_ptr<DriverInfo> &driverInfo) override;

private:
    sptr<V1_1::IUsbDdk> iUsbDdk_ = nullptr;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // USB_DRIVER_CHANGE_CALLBACK_H