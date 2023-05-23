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

#ifndef USB_DEV_SUBSCRIBER_H
#define USB_DEV_SUBSCRIBER_H
#include "ibus_extension.h"
#include "usb_device_info.h"
#include "v1_0/iusbd_subscriber.h"
#include "v1_0/iusb_interface.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace OHOS::HDI::Usb::V1_0;

class UsbDevSubscriber : public IUsbdSubscriber {
public:
    map<uint32_t, shared_ptr<UsbDeviceInfo>> deviceInfos;
    void Init(shared_ptr<IDevChangeCallback> callback, sptr<IUsbInterface> iusb);
    int32_t DeviceEvent(const USBDeviceInfo &info) override;
    int32_t PortChangedEvent(const PortInfo &info) override;
    string ToString(void);
private:
    shared_ptr<IDevChangeCallback> callback;
    sptr<IUsbInterface> iusb;
    int OnDeviceConnect(const UsbDev &usbDev);
    int OnDeviceDisconnect(const UsbDev &usbDev);
};
}
}
#endif