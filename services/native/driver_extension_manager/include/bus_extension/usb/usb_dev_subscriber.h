/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifdef EXTDEVMGR_USB_PASS_THROUGH
#include "v2_0/iusb_host_interface.h"
#else
#include "v1_0/iusbd_subscriber.h"
#include "v1_0/iusb_interface.h"
#endif // EXTDEVMGR_USB_PASS_THROUGH
#include "v1_1/iusb_ddk.h"
#include "usb_config_desc_parser.h"
namespace OHOS {
namespace ExternalDeviceManager {
#ifdef EXTDEVMGR_USB_PASS_THROUGH
using namespace OHOS::HDI::Usb::V2_0;
#else
using namespace OHOS::HDI::Usb::V1_0;
#endif // EXTDEVMGR_USB_PASS_THROUGH
using namespace OHOS::HDI::Usb::Ddk;

struct UsbDevDescLite {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__((packed));

class UsbDevSubscriber : public IUsbdSubscriber {
public:
#ifdef EXTDEVMGR_USB_PASS_THROUGH
    void Init(shared_ptr<IDevChangeCallback> callback, sptr<IUsbHostInterface> iusb, sptr<V1_1::IUsbDdk> iUsbDdk);
#else
    void Init(shared_ptr<IDevChangeCallback> callback, sptr<IUsbInterface> iusb, sptr<V1_1::IUsbDdk> iUsbDdk);
#endif // EXTDEVMGR_USB_PASS_THROUGH
    int32_t DeviceEvent(const USBDeviceInfo &info) override;
    int32_t PortChangedEvent(const PortInfo &info) override;
private:
    shared_ptr<IDevChangeCallback> callback_;
#ifdef EXTDEVMGR_USB_PASS_THROUGH
    sptr<IUsbHostInterface> iusb_;
#else
    sptr<IUsbInterface> iusb_;
#endif // EXTDEVMGR_USB_PASS_THROUGH
    sptr<V1_1::IUsbDdk> iUsbDdk_;
    int32_t OnDeviceConnect(const UsbDev &usbDev);
    int32_t OnDeviceDisconnect(const UsbDev &usbDev);
    int32_t GetInterfaceDescriptor(const UsbDev &usbDev, std::vector<UsbInterfaceDescriptor> &interfaceList);
    std::string GetDevStringVal(const UsbDev &usbDev, uint8_t idx);
    void SetUsbDevInfoValue(const UsbDevDescLite &deviceDescriptor, shared_ptr<UsbDeviceInfo> &usbDevInfo,
        std::string snNum);
};
}
}
#endif