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
    if (parcel.WriteUint32(static_cast<uint32_t>(busType))) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write busType");
        return false;
    }

    if (parcel.WriteUint64(deviceId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write deviceId");
        return false;
    }

    if (parcel.WriteString(descripton)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write descripton");
        return false;
    }

    return true;
}

bool USBDevice::Marshalling(MessageParcel &parcel) const
{
    if (DeviceData::Marshalling(parcel)) {
        return false;
    }

    if (parcel.WriteString(productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to write productId");
        return false;
    }

    if (parcel.WriteString(vendorId)) {
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

    if (!parcel.ReadString(device->productId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read productId");
        return nullptr;
    }

    if (!parcel.ReadString(device->vendorId)) {
        EDM_LOGE(MODULE_DEV_MGR, "failed to read vendorId");
        return nullptr;
    }

    return device;
}
} // namespace ExternalDeviceManager
} // namespace OHOS