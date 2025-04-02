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

#include "string"
#include "securec.h"
#include "hilog_wrapper.h"
#include "edm_errors.h"
#include "usb_dev_subscriber.h"
#include <cwchar>
#include <algorithm>
#include "driver_report_sys_event.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
constexpr uint32_t MAX_DEV_ID_SIZE = 100;
constexpr uint32_t ACT_DEVUP       = 0;
constexpr uint32_t ACT_DEVDOWN     = 1;
constexpr uint32_t SHIFT_16        = 16;
constexpr uint32_t SHIFT_32        = 32;
constexpr uint32_t USB_DEV_DESC_SIZE = 0x12;
constexpr int32_t DESCRIPTOR_TYPE_STRING = 3;
constexpr int32_t DESCRIPTOR_VALUE_START_OFFSET = 2;
constexpr int32_t HALF = 2;

static string ToDeviceDesc(const UsbDev& usbDev, const UsbDevDescLite& desc)
{
    char buffer[MAX_DEV_ID_SIZE];
    auto ret = sprintf_s(buffer, sizeof(buffer), "USB&BUS_%02X&DEV_%02X&PID_%04X&VID_%04X&CLASS_%02X",\
        usbDev.busNum, usbDev.devAddr, desc.idProduct, desc.idVendor, desc.bDeviceClass);
    if (ret < 0) {
        EDM_LOGE(MODULE_BUS_USB,  "ToDeivceDesc sprintf_s error. ret = %{public}d", ret);
        return string();
    }
    return string(buffer);
}

static uint32_t ToBusDeivceId(const UsbDev& usbDev)
{
    uint32_t devId = (usbDev.busNum << SHIFT_16) + usbDev.devAddr;
    return devId;
}

static uint64_t ToDdkDeviceId(const UsbDev& usbDev)
{
    return ((uint64_t)usbDev.busNum << SHIFT_32) + usbDev.devAddr;
}


void UsbDevSubscriber::Init(shared_ptr<IDevChangeCallback> callback, sptr<IUsbInterface> iusb,
    sptr<V1_1::IUsbDdk> iUsbDdk)
{
    this->iusb_ = iusb;
    this->callback_ = callback;
    this->iUsbDdk_ = iUsbDdk;
};

int32_t UsbDevSubscriber::GetInterfaceDescriptor(const UsbDev &usbDev,
    std::vector<UsbInterfaceDescriptor> &interfaceList)
{
    int32_t ret = EDM_NOK;
    if (this->iusb_ == nullptr || this->iUsbDdk_ == nullptr) {
        return EDM_ERR_INVALID_OBJECT;
    }
    uint8_t configIndex;
    ret = this->iusb_->GetConfig(usbDev, configIndex);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB,  "GetConfig fail, ret = %{public}d", ret);
        return ret;
    }

    uint64_t ddkDeviceId = ToDdkDeviceId(usbDev);
    std::vector<uint8_t> descriptor;
    ret = this->iUsbDdk_->GetConfigDescriptor(ddkDeviceId, configIndex, descriptor);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB, "GetConfigDescriptor fail, ret = %{public}d", ret);
        return ret;
    }
    UsbDdkConfigDescriptor *config = nullptr;
    ret = ParseUsbConfigDescriptor(descriptor, &config);
    if (ret != EDM_OK || config == nullptr) {
        FreeUsbConfigDescriptor(config);
        EDM_LOGE(MODULE_BUS_USB, "ParseUsbConfigDescriptor fail, ret = %{public}d", ret);
        return ret;
    }
    if (config->interface == nullptr) {
        FreeUsbConfigDescriptor(config);
        EDM_LOGE(MODULE_BUS_USB,  "UsbDdkInterface is null");
        return EDM_ERR_INVALID_OBJECT;
    }
    for (uint8_t i = 0; i < config->configDescriptor.bNumInterfaces; i++) {
        UsbDdkInterfaceDescriptor *interfaceDesc = config->interface[i].altsetting;
        if (interfaceDesc == nullptr) {
            FreeUsbConfigDescriptor(config);
            EDM_LOGE(MODULE_BUS_USB,  "UsbDdkInterfaceDescriptor is null");
            return EDM_ERR_INVALID_OBJECT;
        }
        interfaceList.push_back(interfaceDesc->interfaceDescriptor);
    }
    FreeUsbConfigDescriptor(config);
    return EDM_OK;
}

int32_t UsbDevSubscriber::OnDeviceConnect(const UsbDev &usbDev)
{
    std::shared_ptr<ExtDevEvent> eventPtr = std::make_shared<ExtDevEvent>();
    std::string interfaceName = std::string(__func__);
    int32_t ret = 0;
    if (this->iusb_ == nullptr) {
        return EDM_ERR_INVALID_OBJECT;
    }
    ret = this->iusb_->OpenDevice(usbDev);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB, "OpenDevice failed, ret = %{public}d", ret);
        ExtDevReportSysEvent::SetEventValue(interfaceName, GET_DEVICE_INFO, EDM_ERR_IO, eventPtr);
        return EDM_ERR_IO;
    }
    vector<uint8_t> descData;
    ret = this->iusb_->GetDeviceDescriptor(usbDev, descData);
    if (ret != EDM_OK || descData.empty()) {
        EDM_LOGE(MODULE_BUS_USB, "GetDeviceDescriptor failed, ret = %{public}d", ret);
        (void)this->iusb_->CloseDevice(usbDev);
        ExtDevReportSysEvent::SetEventValue(interfaceName, GET_DEVICE_INFO, EDM_ERR_IO, eventPtr);
        return EDM_ERR_IO;
    }
    UsbDevDescLite deviceDescriptor = *(reinterpret_cast<const UsbDevDescLite *>(descData.data()));
    if (deviceDescriptor.bLength != USB_DEV_DESC_SIZE) {
        EDM_LOGE(MODULE_BUS_USB,  "UsbdDeviceDescriptor size error");
        (void)this->iusb_->CloseDevice(usbDev);
        return EDM_ERR_USB_ERR;
    }
    auto usbDevInfo = make_shared<UsbDeviceInfo>(ToBusDeivceId(usbDev), ToDeviceDesc(usbDev, deviceDescriptor));
    SetUsbDevInfoValue(deviceDescriptor, usbDevInfo, GetDevStringVal(usbDev, deviceDescriptor.iSerialNumber));
    ret = GetInterfaceDescriptor(usbDev, usbDevInfo->interfaceDescList_);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB,  "GetInterfaceDescriptor fail, ret = %{public}d", ret);
        (void)this->iusb_->CloseDevice(usbDev);
        eventPtr = ExtDevReportSysEvent::ExtDevEventInit(usbDevInfo, nullptr, eventPtr);
        ExtDevReportSysEvent::SetEventValue(interfaceName, GET_DEVICE_INFO, ret, eventPtr);
        return ret;
    }
    eventPtr = ExtDevReportSysEvent::ExtDevEventInit(usbDevInfo, nullptr, eventPtr);
    ExtDevReportSysEvent::DeviceMapInsert(usbDevInfo->GetDeviceId(), eventPtr);
    (void)this->iusb_->CloseDevice(usbDev);
    if (this->callback_ != nullptr) {
        this->callback_->OnDeviceAdd(usbDevInfo);
    }
    ExtDevReportSysEvent::SetEventValue(interfaceName, GET_DEVICE_INFO, EDM_OK, eventPtr);
    return EDM_OK;
};

int32_t UsbDevSubscriber::OnDeviceDisconnect(const UsbDev &usbDev)
{
    EDM_LOGD(MODULE_BUS_USB,  "OnDeviceDisconnect enter");
    std::shared_ptr<ExtDevEvent> eventPtr = std::make_shared<ExtDevEvent>();
    std::string interfaceName = std::string(__func__);
    if (this->callback_ != nullptr) {
        uint32_t busDevId = ToBusDeivceId(usbDev);
        auto deviceInfo = make_shared<UsbDeviceInfo>(busDevId);
        if (deviceInfo != nullptr) {
            eventPtr = ExtDevReportSysEvent::DeviceEventReport(deviceInfo->GetDeviceId());
            this->callback_->OnDeviceRemove(deviceInfo);
            ExtDevReportSysEvent::DeviceMapErase(deviceInfo->GetDeviceId());
            ExtDevReportSysEvent::SetEventValue(interfaceName, GET_DEVICE_INFO, EDM_OK, eventPtr);
        } else {
            EDM_LOGE(MODULE_BUS_USB,  "deviceInfo is nullptr");
            ExtDevReportSysEvent::SetEventValue(interfaceName, GET_DEVICE_INFO, EDM_NOK, eventPtr);
        }
    }
    return 0;
}

int32_t UsbDevSubscriber::DeviceEvent(const USBDeviceInfo &usbDevInfo)
{
    EDM_LOGD(MODULE_BUS_USB,  "DeviceEvent enter");
    UsbDev usbDev = {usbDevInfo.busNum, usbDevInfo.devNum};
    int32_t ret = 0;
    if (usbDevInfo.status == ACT_DEVUP) {
        ret = this->OnDeviceConnect(usbDev);
    } else if (usbDevInfo.status == ACT_DEVDOWN) {
        ret = this->OnDeviceDisconnect(usbDev);
    } else {
        EDM_LOGW(MODULE_BUS_USB,  "status not support, %{public}d \n", usbDevInfo.status);
    }
    return ret;
}

int32_t UsbDevSubscriber::PortChangedEvent(const PortInfo &usbDevInfo)
{
    return 0;
}

std::string UsbDevSubscriber::GetDevStringVal(const UsbDev &usbDev, uint8_t idx)
{
    std::string strDesc = " ";
    std::vector<uint8_t> serial;

    if (idx == 0) {
        return strDesc;
    }

    auto ret = this->iusb_->GetStringDescriptor(usbDev, idx, serial);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_BUS_USB, "GetStringDescriptor failed, ret = %{public}d", ret);
        (void)this->iusb_->CloseDevice(usbDev);
        return strDesc;
    }
    
    size_t length = serial.size();
    if ((length < DESCRIPTOR_VALUE_START_OFFSET) || (serial[1] != DESCRIPTOR_TYPE_STRING)) {
        EDM_LOGE(MODULE_BUS_USB, "type or length error, len:%{public}zu", length);
        return strDesc;
    }

    uint16_t *tbuf = new (std::nothrow) uint16_t[length + 1]();
    if (tbuf == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "new failed");
        return strDesc;
    }

    for (uint32_t i = 0; i < length - DESCRIPTOR_VALUE_START_OFFSET; ++i) {
        tbuf[i] = serial[i + DESCRIPTOR_VALUE_START_OFFSET];
    }
    size_t bufLen = (length - DESCRIPTOR_VALUE_START_OFFSET) / HALF;
    size_t wstrLen = wcslen((wchar_t*)tbuf) <= bufLen ? wcslen((wchar_t*)tbuf) : bufLen;
    std::wstring wstr(reinterpret_cast<wchar_t *>(tbuf), wstrLen);
    strDesc = std::string(wstr.begin(), wstr.end());
    EDM_LOGE(MODULE_BUS_USB, "getString idx:%{public}d length:%{public}zu, str: %{public}s",
        idx, strDesc.length(), strDesc.c_str());
    delete[] tbuf;
    return strDesc;
}

void UsbDevSubscriber::SetUsbDevInfoValue(const UsbDevDescLite &deviceDescriptor,
    shared_ptr<UsbDeviceInfo> &usbDevInfo, std::string snNum)
{
    usbDevInfo->bcdUSB_ = deviceDescriptor.bcdUSB;
    usbDevInfo->idProduct_ = deviceDescriptor.idProduct;
    usbDevInfo->idVendor_ = deviceDescriptor.idVendor;
    usbDevInfo->deviceClass_ = deviceDescriptor.bDeviceClass;
    usbDevInfo->deviceSubClass_ = deviceDescriptor.bDeviceSubClass;
    usbDevInfo->deviceProtocol_ = deviceDescriptor.bDeviceProtocol;
    usbDevInfo->snNum_ = snNum;
}
}
}