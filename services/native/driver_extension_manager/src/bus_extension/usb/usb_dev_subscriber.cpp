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

#include "string"
#include "securec.h"
#include "usbd_type.h"
#include "hilog_wrapper.h"
#include "usb_dev_subscriber.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
constexpr uint32_t MAX_DEV_ID_SIZE = 100;
constexpr uint32_t ACT_DEVUP       = 0;
constexpr uint32_t ACT_DEVDOWN     = 1;
constexpr uint32_t SHIFT_16        = 16;

static string ToDeviceDesc(const UsbDev& usbDev, const UsbdDeviceDescriptor& desc)
{
    char buffer[MAX_DEV_ID_SIZE];
    auto ret = sprintf_s(buffer, sizeof(buffer), "USB&BUS_%02X&DEV_%02X&PID_%04X&VID_%04X&CLASS_%02X",\
        usbDev.busNum, usbDev.devAddr, desc.idProduct, desc.idVendor, desc.bDeviceClass);
    if (ret != 0) {
        EDM_LOGE(MODULE_BUS_USB,  "ToBusDeivceId sprintf_s error. ret = %d", ret);
        return string();
    }
    return string(buffer);
}

static uint32_t ToBusDeivceId(const UsbDev& usbDev)
{
    uint32_t devId = (usbDev.busNum << SHIFT_16) + usbDev.devAddr;
    return devId;
}


void UsbDevSubscriber::Init(shared_ptr<IDevChangeCallback> callback, sptr<IUsbInterface> iusb)
{
    this->iusb = iusb;
    this->callback = callback;
};

int UsbDevSubscriber::OnDeviceConnect(const UsbDev &usbDev)
{
    int ret = 0;
    if (this->iusb == nullptr) {
        return -1;
    }
    vector<uint8_t> descData;
    ret = this->iusb->GetDeviceDescriptor(usbDev, descData);
    if (ret != 0) {
        EDM_LOGE(MODULE_BUS_USB,  "GetDeviceDescriptor fail, ret = %d\n", ret);
        return -1;
    }
    uint8_t *buffer = descData.data();
    uint32_t length = descData.size();
    if (length == 0) {
        EDM_LOGE(MODULE_BUS_USB,  "GetRawDescriptor failed len=%d busNum:%d devAddr:%d",\
            length, usbDev.busNum, usbDev.devAddr);
        return -1;
    }
    UsbdDeviceDescriptor deviceDescriptor = *(reinterpret_cast<const UsbdDeviceDescriptor *>(buffer));
    if (deviceDescriptor.bLength != sizeof(UsbdDeviceDescriptor)) {
        EDM_LOGE(MODULE_BUS_USB,  "UsbdDeviceDescriptor size error");
        return -1;
    }
    string desc = ToDeviceDesc(usbDev, deviceDescriptor);
    uint32_t busDevId = ToBusDeivceId(usbDev);
    auto usbDevInfo = make_shared<UsbDeviceInfo>(busDevId, desc);

    usbDevInfo->bcdUSB = deviceDescriptor.bcdUSB;
    usbDevInfo->idProduct = deviceDescriptor.idProduct;
    usbDevInfo->idVendor = deviceDescriptor.idVendor;
    usbDevInfo->deviceClass = deviceDescriptor.bDeviceClass;

    this->deviceInfos[busDevId] = usbDevInfo;
    if (this->callback != nullptr) {
        this->callback->OnDeviceAdd(usbDevInfo);
    }
    EDM_LOGD(MODULE_BUS_USB,  "OnDeviceConnect:");
    EDM_LOGD(MODULE_BUS_USB,  "%s", desc.c_str());
    return 0;
};

int UsbDevSubscriber::OnDeviceDisconnect(const UsbDev &usbDev)
{
    uint32_t busDevId = ToBusDeivceId(usbDev);
    if (this->callback != nullptr) {
        auto deviceInfo = this->deviceInfos[busDevId];
        if (deviceInfo != nullptr) {
            this->callback->OnDeviceRemove(deviceInfo);
        } else {
            EDM_LOGW(MODULE_BUS_USB,  "no dev in map, busDevId=%08X \n", busDevId);
        }
    }
    this->deviceInfos.erase(busDevId);
    return 0;
}

int32_t UsbDevSubscriber::DeviceEvent(const USBDeviceInfo &usbDevInfo)
{
    UsbDev usbDev = {usbDevInfo.busNum, usbDevInfo.devNum};
    int32_t ret = 0;
    if (usbDevInfo.status == ACT_DEVUP) {
        ret = this->OnDeviceConnect(usbDev);
    } else if (usbDevInfo.status == ACT_DEVDOWN) {
        ret = this->OnDeviceDisconnect(usbDev);
    } else {
        EDM_LOGW(MODULE_BUS_USB,  "status not support, %d \n", usbDevInfo.status);
    }
    EDM_LOGD(MODULE_BUS_USB,  "ret = %d, %s", ret, this->ToString().c_str());
    return ret;
}

int32_t UsbDevSubscriber::PortChangedEvent(const PortInfo &usbDevInfo)
{
    return 0;
}

string UsbDevSubscriber::ToString(void)
{
    string str = "DeviceInfos: Device count:" + to_string(this->deviceInfos.size()) + "\n";
    int i = 0;
    for (auto it = this->deviceInfos.begin(); it != deviceInfos.end(); it++) {
        str += "[" +to_string(i++) + "]" + to_string(it->first) + "\n";
    }
    return str;
}
}
}