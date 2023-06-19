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

#ifndef USB_BUS_EXTENSION_H
#define USB_BUS_EXTENSION_H
#include "ibus_extension.h"
#include "usb_device_info.h"
#include "v1_0/iusb_interface.h"
#include "usb_dev_subscriber.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS::HDI::Usb::V1_0;
class UsbBusExtension : public IBusExtension {
public:
    UsbBusExtension();
    ~UsbBusExtension();
    int32_t SetDevChangeCallback(shared_ptr<IDevChangeCallback> callback) override;
    bool MatchDriver(const DriverInfo &driver, const DeviceInfo &device) override;
    shared_ptr<DriverInfoExt> ParseDriverInfo(const vector<Metadata> &metadata) override;
    shared_ptr<DriverInfoExt> GetNewDriverInfoExtObject() override;
    void SetUsbInferface(sptr<IUsbInterface> iusb);

private:
    sptr<UsbDevSubscriber> subScriber_ = nullptr;
    sptr<IUsbInterface> usbInterface_ = nullptr; // in usb HDI;
    vector<uint16_t> ParseCommaStrToVectorUint16(const string &str);
};
}
}
#endif