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
#ifndef EXT_OBJECT_H
#define EXT_OBJECT_H

#include <memory>
#include <string>
#include "edm_errors.h"
namespace OHOS {
namespace ExternalDeviceManager {
enum BusType : uint32_t {
    BUS_TYPE_INVALID = 0,
    BUS_TYPE_USB = 1,
    BUS_TYPE_MAX,
    BUS_TYPE_TEST,
};

class DrvBundleStateCallback;
class DriverInfoExt {
public:
    virtual ~DriverInfoExt() = default;
    virtual int32_t Serialize(std::string &str) = 0;
    virtual int32_t UnSerialize(const std::string &str) = 0;
};

class DriverInfo : public DriverInfoExt {
public:
    int32_t Serialize(std::string &str) override;
    int32_t UnSerialize(const std::string &str) override;
    std::string GetBusName() const
    {
        return bus_;
    }
    std::shared_ptr<DriverInfoExt> GetInfoExt() const
    {
        return driverInfoExt_;
    }
private:
    friend class DrvBundleStateCallback;
    std::string bus_;
    std::string vendor_;
    std::string version_;
    std::shared_ptr<DriverInfoExt> driverInfoExt_;
};

class DeviceInfo {
public:
    DeviceInfo(uint32_t busDeviceId, const std::string &description = "") : description_(description)
    {
        devInfo_.devBusInfo.busDeviceId = busDeviceId;
    }
    virtual ~DeviceInfo() = default;
    BusType GetBusType() const
    {
        return devInfo_.devBusInfo.busType;
    }
    uint64_t GetDeviceId() const
    {
        return devInfo_.deviceId;
    }
    const std::string& GetDeviceDescription() const
    {
        return description_;
    }

private:
    union DevInfo {
        uint64_t deviceId;
        struct {
            uint32_t busDeviceId;
            BusType busType;
        } devBusInfo;
    } devInfo_;
    std::string description_ {""};
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // EXT_OBJECT_H