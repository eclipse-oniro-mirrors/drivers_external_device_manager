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
#include "driver_ext_mgr_types.h"
#include <sstream>

#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
constexpr uint64_t MAX_INTERFACE_DESC_SIZE = 512;

bool ErrMsg::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(errCode)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write errCode");
        return false;
    }

    if (!parcel.WriteString(msg)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write msg");
        return false;
    }
    return true;
}

ErrMsg* ErrMsg::Unmarshalling(Parcel &data)
{
    ErrMsg *errMsg = new (std::nothrow) ErrMsg;
    if (errMsg == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to create ErrMsg");
        return nullptr;
    }
    errMsg->errCode = static_cast<UsbErrCode>(data.ReadInt32());
    errMsg->msg = data.ReadString();
    
    return errMsg;
}

bool DeviceData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(static_cast<uint32_t>(busType))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write busType");
        return false;
    }

    if (!parcel.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
        return false;
    }

    if (!parcel.WriteString(descripton)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write descripton");
        return false;
    }

    return true;
}

bool USBDevice::Marshalling(Parcel &parcel) const
{
    if (!DeviceData::Marshalling(parcel)) {
        return false;
    }

    if (!parcel.WriteUint16(productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write productId");
        return false;
    }

    if (!parcel.WriteUint16(vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vendorId");
        return false;
    }

    return true;
}

DeviceData* DeviceData::Unmarshalling(Parcel &data)
{
    BusType busType = static_cast<BusType>(data.ReadUint32());
    if (busType == BusType::BUS_TYPE_INVALID) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}u", busType);
        return nullptr;
    }
    DeviceData *deviceData = nullptr;
    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            deviceData = USBDevice::Unmarshalling(data);
            break;
        }
        default:
            break;
    }

    if (deviceData != nullptr) {
        deviceData->busType = busType;
    }

    return deviceData;
}

std::string DeviceData::Dump()
{
    std::stringstream os;
    os << "{busType:" << busType << ", ";
    os << "deviceId:" << deviceId << ", ";
    os << "descripton:" << descripton << "}";
    return os.str();
}

USBDevice* USBDevice::Unmarshalling(Parcel &data)
{
    USBDevice *usbDevice = new (std::nothrow) USBDevice;
    if (usbDevice == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to create usbDevice");
        return nullptr;
    }

    usbDevice->deviceId = data.ReadUint64();
    usbDevice->descripton = data.ReadString();
    usbDevice->productId = data.ReadUint16();
    usbDevice->vendorId = data.ReadUint16();

    return usbDevice;
}

std::string USBDevice::Dump()
{
    std::stringstream os;
    os << "{busType:" << busType << ", ";
    os << "deviceId:" << deviceId << ", ";
    os << "descripton:" << descripton << ", ";
    os << "productId:" << productId << ", ";
    os << "vendorId:" << vendorId << "}";
    return os.str();
}

bool DeviceInfoData::Marshalling(Parcel &parcel) const
{
    EDM_LOGD(MODULE_DEV_MGR, "DeviceInfoData Marshalling enter");
    if (!parcel.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
        return false;
    }

    if (!parcel.WriteBool(isDriverMatched)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write isDriverMatched");
        return false;
    }

    if (!parcel.WriteString(driverUid)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write driverUid");
        return false;
    }
    return true;
}

bool USBDeviceInfoData::Marshalling(Parcel &parcel) const
{
    EDM_LOGD(MODULE_DEV_MGR, "USBDeviceInfoData Marshalling enter");
    if (!DeviceInfoData::Marshalling(parcel)) {
        return false;
    }

    if (!parcel.WriteUint16(productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write productId");
        return false;
    }

    if (!parcel.WriteUint16(vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vendorId");
        return false;
    }

    EDM_LOGD(MODULE_DEV_MGR, "interfaceDescList size: %{public}zu", interfaceDescList.size());
    if (!parcel.WriteUint64(static_cast<uint64_t>(interfaceDescList.size()))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write interfaceDescList size");
        return false;
    }
    
    for (auto &desc : interfaceDescList) {
        if (desc == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "invalid usb interface desc");
            return false;
        }

        if (!desc->Marshalling(parcel)) {
            return false;
        }
    }

    return true;
}

bool USBInterfaceDesc::Marshalling(Parcel &parcel) const
{
    EDM_LOGD(MODULE_DEV_MGR, "USBInterfaceDesc Marshalling enter");
    if (!parcel.WriteUint8(bInterfaceNumber)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bInterfaceNumber");
        return false;
    }

    if (!parcel.WriteUint8(bClass)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bClass");
        return false;
    }

    if (!parcel.WriteUint8(bSubClass)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bSubClass");
        return false;
    }

    if (!parcel.WriteUint8(bProtocol)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bProtocol");
        return false;
    }
    return true;
}

DeviceInfoData* DeviceInfoData::Unmarshalling(Parcel &data)
{
    uint64_t deviceId = 0;
    if (!data.ReadUint64(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceId");
        return nullptr;
    }

    BusType busType = DeviceInfoData::GetBusTypeByDeviceId(deviceId);
    if (busType <= BusType::BUS_TYPE_INVALID || busType >= BusType::BUS_TYPE_MAX) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}u", busType);
        return nullptr;
    }

    bool isDriverMatched = false;
    if (!data.ReadBool(isDriverMatched)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read isDriverMatched");
        return nullptr;
    }

    std::string driverUid = "";
    if (!data.ReadString(driverUid)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read driverUid");
        return nullptr;
    }

    DeviceInfoData *deviceInfoData = nullptr;
    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            deviceInfoData = USBDeviceInfoData::Unmarshalling(data);
            break;
        }
        default:
            break;
    }
    if (deviceInfoData != nullptr) {
        deviceInfoData->deviceId = deviceId;
        deviceInfoData->isDriverMatched = isDriverMatched;
        deviceInfoData->driverUid = driverUid;
    }
    return deviceInfoData;
}

BusType DeviceInfoData::GetBusTypeByDeviceId(uint64_t deviceId)
{
    return static_cast<BusType>(deviceId & 0x00000000FFFFFFF);
}

USBDeviceInfoData* USBDeviceInfoData::Unmarshalling(Parcel &data)
{
    USBDeviceInfoData *usbDeviceInfo = new (std::nothrow) USBDeviceInfoData;
    if (usbDeviceInfo == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to create usbDeviceInfo");
        return nullptr;
    }
    usbDeviceInfo->productId = data.ReadUint16();
    usbDeviceInfo->vendorId = data.ReadUint16();
    uint64_t interfaceDescSize = data.ReadUint64();
    if (interfaceDescSize > MAX_INTERFACE_DESC_SIZE) {
        EDM_LOGE(MODULE_DEV_MGR, "interfaceDescSize is out of range");
        delete usbDeviceInfo;
        return nullptr;
    }
    for (uint64_t i = 0; i < interfaceDescSize; i++) {
        std::shared_ptr<USBInterfaceDesc> desc = USBInterfaceDesc::Unmarshalling(data);
        if (desc == nullptr) {
            delete usbDeviceInfo;
            return nullptr;
        }
        usbDeviceInfo->interfaceDescList.push_back(desc);
    }
    return usbDeviceInfo;
}

std::shared_ptr<USBInterfaceDesc> USBInterfaceDesc::Unmarshalling(Parcel &data)
{
    std::shared_ptr<USBInterfaceDesc> desc = std::make_shared<USBInterfaceDesc>();
    desc->bInterfaceNumber = data.ReadUint8();
    desc->bClass = data.ReadUint8();
    desc->bSubClass = data.ReadUint8();
    desc->bProtocol = data.ReadUint8();
    return desc;
}

bool DriverInfoData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(static_cast<uint32_t>(busType))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write busType");
        return false;
    }

    if (!parcel.WriteString(driverUid)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write driverUid");
        return false;
    }

    if (!parcel.WriteString(driverName)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write driverName");
        return false;
    }

    if (!parcel.WriteString(bundleSize)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bundleSize");
        return false;
    }

    if (!parcel.WriteString(version)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write version");
        return false;
    }

    if (!parcel.WriteString(description)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write description");
        return false;
    }

    return true;
}

bool USBDriverInfoData::Marshalling(Parcel &parcel) const
{
    if (!DriverInfoData::Marshalling(parcel)) {
        return false;
    }

    if (!parcel.WriteUInt16Vector(pids)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write pids");
        return false;
    }

    if (!parcel.WriteUInt16Vector(vids)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vids");
        return false;
    }

    return true;
}

DriverInfoData* DriverInfoData::Unmarshalling(Parcel &data)
{
    BusType busType = static_cast<BusType>(data.ReadUint32());
    DriverInfoData *driverInfoData = nullptr;
    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            driverInfoData = USBDriverInfoData::Unmarshalling(data);
            break;
        }
        default:
            break;
    }
    return driverInfoData;
}

USBDriverInfoData* USBDriverInfoData::Unmarshalling(Parcel &data)
{
    USBDriverInfoData *usbdriverInfoData = new (std::nothrow) USBDriverInfoData;
    if (usbdriverInfoData == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to create usbDriverInfoData");
        return nullptr;
    }

    usbdriverInfoData->busType = BusType::BUS_TYPE_USB;
    usbdriverInfoData->driverUid = data.ReadString();
    usbdriverInfoData->driverName = data.ReadString();
    usbdriverInfoData->bundleSize = data.ReadString();
    usbdriverInfoData->version = data.ReadString();
    usbdriverInfoData->description = data.ReadString();
    data.ReadUInt16Vector(&usbdriverInfoData->pids);
    data.ReadUInt16Vector(&usbdriverInfoData->vids);

    return usbdriverInfoData;
}
} // namespace ExternalDeviceManager
} // namespace OHOS