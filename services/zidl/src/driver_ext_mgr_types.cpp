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
bool ErrMsg::Marshalling(MessageParcel &parcel) const
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

bool ErrMsg::UnMarshalling(MessageParcel &parcel, ErrMsg &data)
{
    int32_t err = 0;
    if (!parcel.ReadInt32(err)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read errCode");
        return false;
    }
    data.errCode = static_cast<UsbErrCode>(err);

    if (!parcel.ReadString(data.msg)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read msg");
        return false;
    }
    return true;
}

bool DeviceData::Marshalling(MessageParcel &parcel) const
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

bool USBDevice::Marshalling(MessageParcel &parcel) const
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

std::shared_ptr<DeviceData> DeviceData::UnMarshalling(MessageParcel &parcel)
{
    uint32_t busTypeData = 0;
    if (!parcel.ReadUint32(busTypeData)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read busType");
        return nullptr;
    }

    BusType busType = static_cast<BusType>(busTypeData);
    if (busType == BusType::BUS_TYPE_INVALID) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}u", busTypeData);
        return nullptr;
    }

    // if you need to extend the DeviceData type, please add code to read it here
    std::shared_ptr<DeviceData> device;
    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            device = USBDevice::UnMarshalling(parcel);
            break;
        }
        default:
            break;
    }

    if (device != nullptr) {
        device->busType = busType;
    }
    return device;
}

std::string DeviceData::Dump()
{
    std::stringstream os;
    os << "{busType:" << busType << ", ";
    os << "deviceId:" << deviceId << ", ";
    os << "descripton:" << descripton << "}";
    return os.str();
}

std::shared_ptr<DeviceData> USBDevice::UnMarshalling(MessageParcel &parcel)
{
    // the busType has been read
    std::shared_ptr<USBDevice> device = std::make_shared<USBDevice>();
    if (!parcel.ReadUint64(device->deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceId");
        return nullptr;
    }

    if (!parcel.ReadString(device->descripton)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read descripton");
        return nullptr;
    }

    if (!parcel.ReadUint16(device->productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read productId");
        return nullptr;
    }

    if (!parcel.ReadUint16(device->vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read vendorId");
        return nullptr;
    }

    return device;
}

bool EmitItemMarshalling(int32_t deviceId, const std::vector<Hid_EmitItem> &items, MessageParcel &parcel)
{
    if (!parcel.WriteUint32(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write device id");
        return false;
    }

    if (!parcel.WriteUint32(static_cast<uint32_t>(items.size()))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vector size");
        return false;
    }

    for (auto &ele : items) {
        if (!parcel.WriteUint16(ele.type)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write type");
            return false;
        }

        if (!parcel.WriteUint16(ele.code)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write code");
            return false;
        }
        if (!parcel.WriteUint32(ele.value)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write value");
            return false;
        }
    }

    return true;
}

std::optional<std::vector<Hid_EmitItem>> EmitItemUnMarshalling(MessageParcel &parcel, int32_t &deviceId)
{
    uint32_t id = 0;
    if (!parcel.ReadUint32(id)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read device id");
        return std::nullopt;
    }
    deviceId = static_cast<int32_t>(id);

    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read size");
        return std::nullopt;
    }

    if (size > MAX_EMIT_ITEM_NUM) {
        EDM_LOGE(MODULE_DEV_MGR, "size out of range");
        return std::nullopt;
    }

    std::vector<Hid_EmitItem> items;
    Hid_EmitItem item;
    for (uint32_t i = 0; i < size; ++i) {
        if (!parcel.ReadUint16(item.type)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read type");
            return std::nullopt;
        }

        if (!parcel.ReadUint16(item.code)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read code");
            return std::nullopt;
        }

        if (!parcel.ReadUint32(item.value)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read value");
            return std::nullopt;
        }
        items.push_back(item);
    }
    return items;
}

bool HidDeviceMarshalling(Hid_Device *hidDevice, MessageParcel &parcel)
{
    if (!parcel.WriteString(static_cast<std::string>(hidDevice->deviceName))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceName");
        return false;
    }

    if (!parcel.WriteUint16(hidDevice->vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write vendorId");
        return false;
    }

    if (!parcel.WriteUint16(hidDevice->productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write productId");
        return false;
    }

    if (!parcel.WriteUint16(hidDevice->version)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write version");
        return false;
    }

    if (!parcel.WriteUint16(hidDevice->bustype)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write bustype");
        return false;
    }

    if (!parcel.WriteUint16(hidDevice->propLength)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write properties length");
        return false;
    }

    for (uint16_t i = 0; i < hidDevice->propLength; ++i) {
        uint32_t ele = static_cast<uint32_t>(hidDevice->properties[i]);
        if (!parcel.WriteUint32(ele)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write properties");
            return false;
        }
    }

    return true;
}

std::optional<Hid_Device> HidDeviceUnMarshalling(MessageParcel &parcel)
{
    Hid_Device hidDevice;
    std::string str;
    if (!parcel.ReadString(str)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read deviceName");
        return std::nullopt;
    }
    hidDevice.deviceName = (char *)str.c_str();

    if (!parcel.ReadUint16(hidDevice.vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read vendorId");
        return std::nullopt;
    }

    if (!parcel.ReadUint16(hidDevice.productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read productId");
        return std::nullopt;
    }

    if (!parcel.ReadUint16(hidDevice.version)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read version");
        return std::nullopt;
    }

    if (!parcel.ReadUint16(hidDevice.bustype)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read bustype");
        return std::nullopt;
    }

    if (!parcel.ReadUint16(hidDevice.propLength)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read properties length");
        return std::nullopt;
    }

    if (hidDevice.propLength > MAX_HID_DEVICE_PROP_LEN) {
        EDM_LOGE(MODULE_DEV_MGR, "properties length out of range");
        return std::nullopt;
    }
    
    uint32_t property = 0;
    Hid_DeviceProp *hidDeviceProp = new Hid_DeviceProp[MAX_HID_DEVICE_PROP_LEN];
    for (uint16_t i = 0; i < hidDevice.propLength; ++i) {
        if (!parcel.ReadUint32(property)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read properties");
            return std::nullopt;
        }
        hidDeviceProp[i] = static_cast<Hid_DeviceProp>(property);
    }
    hidDevice.properties = hidDeviceProp;

    return hidDevice;
}

bool HidEventPropertiesMarshalling(Hid_EventProperties *hidEventProperties, MessageParcel &parcel)
{
    uint16_t hidEventTypesLen = hidEventProperties->hidEventTypes.length;
    if (!parcel.WriteUint16(hidEventTypesLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write hidEventTypes length");
        return false;
    }

    for (uint16_t i = 0; i < hidEventTypesLen; ++i) {
        uint32_t ele = static_cast<uint32_t>(hidEventProperties->hidEventTypes.hidEventType[i]);
        if (!parcel.WriteUint32(ele)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidEventTypes");
            return false;
        }
    }

    if (!HidKeysOrAxisPropertiesMarshalling(hidEventProperties, parcel)) {
        return false;
    }

    uint16_t hidMiscellaneousLen = hidEventProperties->hidMiscellaneous.length;
    if (!parcel.WriteUint16(hidMiscellaneousLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write hidMiscellaneous length");
        return false;
    }
    
    for (uint16_t i = 0; i < hidMiscellaneousLen; ++i) {
        uint32_t ele = static_cast<uint32_t>(hidEventProperties->hidMiscellaneous.hidMscEvent[i]);
        if (!parcel.WriteUint32(ele)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidMiscellaneous");
            return false;
        }
    }

    if (!HidAbsValueMarshalling(hidEventProperties, parcel)) {
        return false;
    }

    return true;
}

bool HidKeysOrAxisPropertiesMarshalling(Hid_EventProperties *hidEventProperties, MessageParcel &parcel)
{
    uint16_t hidKeysLen = hidEventProperties->hidKeys.length;
    if (!parcel.WriteUint16(hidKeysLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write hidKeys length");
        return false;
    }
    
    for (uint16_t i = 0; i < hidKeysLen; ++i) {
        uint32_t ele = static_cast<uint32_t>(hidEventProperties->hidKeys.hidKeyCode[i]);
        if (!parcel.WriteUint32(ele)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidKeys");
            return false;
        }
    }

    uint16_t hidAbsLen = hidEventProperties->hidAbs.length;
    if (!parcel.WriteUint16(hidAbsLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write hidAbs length");
        return false;
    }
    
    for (uint16_t i = 0; i < hidAbsLen; ++i) {
        uint32_t ele = static_cast<uint32_t>(hidEventProperties->hidAbs.hidAbsAxes[i]);
        if (!parcel.WriteUint32(ele)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidAbs");
            return false;
        }
    }

    uint16_t hidRelBitsLen = hidEventProperties->hidRelBits.length;
    if (!parcel.WriteUint16(hidRelBitsLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write hidRelBits length");
        return false;
    }
    
    for (uint16_t i = 0; i < hidRelBitsLen; ++i) {
        uint32_t ele = static_cast<uint32_t>(hidEventProperties->hidRelBits.hidRelAxes[i]);
        if (!parcel.WriteUint32(ele)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidRelBits");
            return false;
        }
    }

    return true;
}

bool HidAbsValueMarshalling(Hid_EventProperties *hidEventProperties, MessageParcel &parcel)
{
    const int absLength = 64;
    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.WriteInt32(hidEventProperties->hidAbsMax[i])) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidAbsMax");
            return false;
        }
    }

    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.WriteInt32(hidEventProperties->hidAbsMin[i])) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidAbsMin");
            return false;
        }
    }

    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.WriteInt32(hidEventProperties->hidAbsFuzz[i])) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidAbsFuzz");
            return false;
        }
    }

    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.WriteInt32(hidEventProperties->hidAbsFlat[i])) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to write hidAbsFlat");
            return false;
        }
    }
    
    return true;
}

std::optional<Hid_EventProperties> HidEventPropertiesUnMarshalling(MessageParcel &parcel)
{
    Hid_EventProperties hidEventProperties;
    
    if (!HidEventTypeOrKeysPropertiesUnMarshalling(parcel, hidEventProperties)) {
        return std::nullopt;
    }

    if (!HidAxisPropertiesUnMarshalling(parcel, hidEventProperties)) {
        return std::nullopt;
    }

    uint16_t hidMiscellaneousLen = 0;
    if (!parcel.ReadUint16(hidMiscellaneousLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read hidMiscellaneous length");
        return std::nullopt;
    }
    hidEventProperties.hidMiscellaneous.length = hidMiscellaneousLen;

    if (hidMiscellaneousLen > MAX_HID_MISC_EVENT_LEN) {
        EDM_LOGE(MODULE_DEV_MGR, "hidMiscellaneous length out of range");
        return std::nullopt;
    }

    uint32_t hidMiscellaneous = 0;
    Hid_MscEvent *hidMscEvent = new Hid_MscEvent[MAX_HID_MISC_EVENT_LEN];
    for (uint16_t i = 0; i < hidMiscellaneousLen; ++i) {
        if (!parcel.ReadUint32(hidMiscellaneous)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidMiscellaneous");
            return std::nullopt;
        }
        hidMscEvent[i] = static_cast<Hid_MscEvent>(hidMiscellaneous);
    }
    hidEventProperties.hidMiscellaneous.hidMscEvent = hidMscEvent;

    ;
    if (!HidAbsValueUnMarshalling(parcel, hidEventProperties)) {
        return std::nullopt;
    }

    return hidEventProperties;
}

bool HidEventTypeOrKeysPropertiesUnMarshalling(MessageParcel &parcel, Hid_EventProperties &hidEventProperties)
{
    uint16_t hidEventTypesLen = 0;
    if (!parcel.ReadUint16(hidEventTypesLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read hidEventTypes length");
        return false;
    }
    hidEventProperties.hidEventTypes.length = hidEventTypesLen;

    if (hidEventTypesLen > MAX_HID_EVENT_TYPES_LEN) {
        EDM_LOGE(MODULE_DEV_MGR, "hidEventTypes length out of range");
        return false;
    }

    uint32_t hidEventType = 0;
    Hid_EventType *hidEventTypes = new Hid_EventType[MAX_HID_EVENT_TYPES_LEN];
    for (uint16_t i = 0; i < hidEventTypesLen; ++i) {
        if (!parcel.ReadUint32(hidEventType)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidEventTypes");
            return false;
        }
        hidEventTypes[i] = static_cast<Hid_EventType>(hidEventType);
    }
    hidEventProperties.hidEventTypes.hidEventType = hidEventTypes;

    uint16_t hidKeysLen = 0;
    if (!parcel.ReadUint16(hidKeysLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read hidKeys length");
        return false;
    }
    hidEventProperties.hidKeys.length = hidKeysLen;

    if (hidKeysLen > MAX_HID_KEYS_LEN) {
        EDM_LOGE(MODULE_DEV_MGR, "hidKeys length out of range");
        return false;
    }

    uint32_t hidKey = 0;
    Hid_KeyCode *hidKeyCode = new Hid_KeyCode[MAX_HID_KEYS_LEN];
    for (uint16_t i = 0; i < hidKeysLen; ++i) {
        if (!parcel.ReadUint32(hidKey)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidKeys");
            return false;
        }
        hidKeyCode[i] = static_cast<Hid_KeyCode>(hidKey);
    }
    hidEventProperties.hidKeys.hidKeyCode = hidKeyCode;

    return true;
}

bool HidAxisPropertiesUnMarshalling(MessageParcel &parcel, Hid_EventProperties &hidEventProperties)
{
    uint16_t hidAbsLen = 0;
    if (!parcel.ReadUint16(hidAbsLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read hidAbs length");
        return false;
    }
    hidEventProperties.hidAbs.length = hidAbsLen;

    if (hidAbsLen > MAX_HID_ABS_LEN) {
        EDM_LOGE(MODULE_DEV_MGR, "hidAbs length out of range");
        return false;
    }

    uint32_t hidAbs = 0;
    Hid_AbsAxes *hidAbsAxes = new Hid_AbsAxes[MAX_HID_ABS_LEN];
    for (uint16_t i = 0; i < hidAbsLen; ++i) {
        if (!parcel.ReadUint32(hidAbs)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidAbs");
            return false;
        }
        hidAbsAxes[i] = static_cast<Hid_AbsAxes>(hidAbs);
    }
    hidEventProperties.hidAbs.hidAbsAxes = hidAbsAxes;

    uint16_t hidRelBitsLen = 0;
    if (!parcel.ReadUint16(hidRelBitsLen)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read hidRelBits length");
        return false;
    }
    hidEventProperties.hidRelBits.length = hidRelBitsLen;

    if (hidRelBitsLen > MAX_HID_REL_BITS_LEN) {
        EDM_LOGE(MODULE_DEV_MGR, "hidRelBits length out of range");
        return false;
    }

    uint32_t hidRelBit = 0;
    Hid_RelAxes *hidRelAxes = new Hid_RelAxes[MAX_HID_REL_BITS_LEN];
    for (uint16_t i = 0; i < hidRelBitsLen; ++i) {
        if (!parcel.ReadUint32(hidRelBit)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidRelBits");
            return false;
        }
        hidRelAxes[i] = static_cast<Hid_RelAxes>(hidRelBit);
    }
    hidEventProperties.hidRelBits.hidRelAxes = hidRelAxes;

    return true;
}

bool HidAbsValueUnMarshalling(MessageParcel &parcel, Hid_EventProperties &hidEventProperties)
{
    const int absLength = 64;
    int32_t absMax = 0;
    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.ReadInt32(absMax)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidAbsMax");
            return false;
        }
        hidEventProperties.hidAbsMax[i] = absMax;
    }

    int32_t absMin = 0;
    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.ReadInt32(absMin)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidAbsMin");
            return false;
        }
        hidEventProperties.hidAbsMin[i] = absMin;
    }

    int32_t absFuzz = 0;
    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.ReadInt32(absFuzz)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidAbsFuzz");
            return false;
        }
        hidEventProperties.hidAbsFuzz[i] = absFuzz;
    }

    int32_t absFlat = 0;
    for (uint16_t i = 0; i < absLength; ++i) {
        if (!parcel.ReadInt32(absFlat)) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to read hidAbsFlat");
            return false;
        }
        hidEventProperties.hidAbsFlat[i] = absFlat;
    }

    return true;
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
} // namespace ExternalDeviceManager
} // namespace OHOS