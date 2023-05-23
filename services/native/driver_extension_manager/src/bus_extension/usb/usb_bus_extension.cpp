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
#include "string_ex.h"
#include "sstream"
#include "iostream"

#include "hilog_wrapper.h"
#include "ibus_extension.h"
#include "usb_dev_subscriber.h"
#include "usb_driver_info.h"
#include "usb_device_info.h"
#include "usb_bus_extension.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;

UsbBusExtension::UsbBusExtension()
{
    this->subScriber = nullptr;
    this->usbInterface = nullptr;
}

UsbBusExtension::~UsbBusExtension()
{
    if (this->usbInterface != nullptr && this->subScriber != nullptr) {
        this->usbInterface->UnbindUsbdSubscriber(this->subScriber);
    }
}

void UsbBusExtension::SetUsbInferface(sptr<IUsbInterface> iusb)
{
    this->usbInterface = iusb;
}

int UsbBusExtension::SetDevChangeCallback(shared_ptr<IDevChangeCallback> devCallback)
{
    if (this->usbInterface == nullptr) {
        this->usbInterface = IUsbInterface::Get();
        if (this->usbInterface == nullptr) {
            EDM_LOGE(MODULE_BUS_USB,  "get IUsbInterface error");
            return -1;
        }
        EDM_LOGD(MODULE_BUS_USB,  "get usbInferface sucess");
    }
    if (this->subScriber == nullptr) {
        this->subScriber = new UsbDevSubscriber();
        if (this->subScriber == nullptr) {
            EDM_LOGE(MODULE_BUS_USB,  "get usbDevSubscriber error");
            return -1;
        }
        EDM_LOGD(MODULE_BUS_USB,  "get subScriber sucess");
    }

    this->subScriber->Init(devCallback, usbInterface);
    this->usbInterface->BindUsbdSubscriber(subScriber);
    return 0;
};

bool UsbBusExtension::MatchDriver(const DriverInfo &driver, const DeviceInfo &device)
{
    if (LowerStr(driver.bus) != "usb") {
        EDM_LOGW(MODULE_BUS_USB,  "driver bus not support by this module [UsbBusExtension]");
        return false;
    }
    if (device.GetBusType() != BusType::BUS_TYPE_USB) {
        EDM_LOGW(MODULE_BUS_USB,  "deivce type not support");
        return false;
    }
    UsbDriverInfo *usbDriverInfo = static_cast<UsbDriverInfo *>(driver.driverInfoExt.get());
    UsbDeviceInfo *usbDeviceInfo = static_cast<UsbDeviceInfo *>(&const_cast<DeviceInfo&>(device));
    if (usbDriverInfo == nullptr || usbDeviceInfo == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "static_cast error, the usbDriverInfo or usbDeviceInfo is nullptr");
        return false;
    }

    auto vidFind = find(usbDriverInfo->vids.begin(), usbDriverInfo->vids.end(), usbDeviceInfo->idVendor);
    if (vidFind == usbDriverInfo->vids.end()) {
        EDM_LOGI(MODULE_BUS_USB,  "vid not match\n");
        return false;
    }
    auto pidFind = find(usbDriverInfo->pids.begin(), usbDriverInfo->pids.end(), usbDeviceInfo->idProduct);
    if (pidFind == usbDriverInfo->pids.end()) {
        EDM_LOGI(MODULE_BUS_USB,  "pid not match\n");
        return false;
    }
    EDM_LOGI(MODULE_BUS_USB,  "Driver and Device match sucess\n");
    return true;
}

shared_ptr<DriverInfoExt> UsbBusExtension::ParseDriverInfo(const vector<Metadata> &metadata)
{
    shared_ptr<UsbDriverInfo> usbDriverInfo = make_shared<UsbDriverInfo>();
    if (usbDriverInfo == nullptr) {
        EDM_LOGE(MODULE_BUS_USB,  "creat UsbDriverInfo obj fail\n");
        return nullptr;
    }
    for (auto meta : metadata) {
        if (LowerStr(meta.name) == "pid") {
            usbDriverInfo->pids = this->ParseCommaStrToVectorUint16(meta.value);
        } else if (LowerStr(meta.name) == "vid") {
            usbDriverInfo->vids = this->ParseCommaStrToVectorUint16(meta.value);
        }
    }
    return usbDriverInfo;
}

vector<uint16_t> UsbBusExtension::ParseCommaStrToVectorUint16(const string &str)
{
    vector<uint16_t> ret;
    stringstream ss(str);
    string s;
    int num;
    while (getline(ss, s, ',')) {
        stringstream(s) >> num;
        ret.push_back(num);
    }
    if (ret.size() == 0) {
        EDM_LOGW(MODULE_BUS_USB,  "parse error, size 0, str:%s.", str.c_str());
    } else {
        EDM_LOGD(MODULE_BUS_USB,  "parse sucess, size %lu, str:%s", ret.size(), str.c_str());
    }

    return ret;
}
}
}