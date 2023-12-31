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

#ifndef DRIVER_EXTENSION_MANAGER_TYPES_H
#define DRIVER_EXTENSION_MANAGER_TYPES_H

#include <memory>
#include <string>

#include "emit_event_types.h"
#include "ext_object.h"
#include "message_parcel.h"

namespace OHOS {
namespace ExternalDeviceManager {
struct ErrMsg {
    ErrMsg(UsbErrCode code = UsbErrCode::EDM_NOK, const std::string &message = "") : errCode(code), msg(message) {}

    inline bool IsOk() const
    {
        return errCode == UsbErrCode::EDM_OK;
    }

    bool Marshalling(MessageParcel &parcel) const;
    static bool UnMarshalling(MessageParcel &parcel, ErrMsg &data);

    UsbErrCode errCode;
    std::string msg;
};

class DeviceData {
public:
    virtual ~DeviceData() = default;

    virtual bool Marshalling(MessageParcel &parcel) const;
    static std::shared_ptr<DeviceData> UnMarshalling(MessageParcel &parcel);
    virtual std::string Dump();

    BusType busType;
    uint64_t deviceId;
    std::string descripton;
};

class USBDevice : public DeviceData {
public:
    virtual ~USBDevice() = default;

    bool Marshalling(MessageParcel &parcel) const override;
    static std::shared_ptr<DeviceData> UnMarshalling(MessageParcel &parcel);
    std::string Dump() override;

    uint16_t productId;
    uint16_t vendorId;
};

constexpr uint32_t MAX_EMIT_ITEM_NUM = 20;
bool EmitItemMarshalling(int32_t deviceId, const std::vector<EmitItem> &items, MessageParcel &parcel);
std::optional<std::vector<EmitItem>> EmitItemUnMarshalling(MessageParcel &parcel, int32_t &deviceId);
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_TYPES_H
