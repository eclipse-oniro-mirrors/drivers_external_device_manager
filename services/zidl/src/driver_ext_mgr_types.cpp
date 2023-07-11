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

bool EmitItemMarshalling(int32_t deviceId, const std::vector<EmitItem> &items, MessageParcel &parcel)
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

std::optional<std::vector<EmitItem>> EmitItemUnMarshalling(MessageParcel &parcel, int32_t &deviceId)
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

    std::vector<EmitItem> items;
    EmitItem item;
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