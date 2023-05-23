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
#include <unordered_map>
#include "singleton.h"
namespace OHOS {
namespace ExternalDeviceManager {
enum BusType : uint32_t {
    BUS_TYPE_INVALID = 0,
    BUS_TYPE_USB = 1,
};

struct DriverInfoExt {
public:
    virtual ~DriverInfoExt() = default;
    virtual int Serialize(std::string &str) = 0;
    virtual int UnSerialize(const std::string &str) = 0;
};

struct DriverInfo : public DriverInfoExt {
public:
    std::string bus;
    std::string vendor;
    std::string version;
    std::shared_ptr<DriverInfoExt> driverInfoExt;
    int Serialize(std::string &str) override;
    int UnSerialize(const std::string &str) override;
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

private:
    friend class DevChangeCallback;
    union DevInfo {
        uint64_t deviceId;
        struct {
            uint32_t busDeviceId;
            BusType busType;
        } devBusInfo;
    } devInfo_;
    std::string description_ {""};
};

class ExtDeviceManager;
class IDevChangeCallback {
public:
    virtual ~IDevChangeCallback();
    virtual int32_t OnDeviceAdd(std::shared_ptr<DeviceInfo> device);
    virtual int32_t OnDeviceRemove(std::shared_ptr<DeviceInfo> device);
};

class DevChangeCallback final : public IDevChangeCallback {
public:
    DevChangeCallback(BusType busType, std::shared_ptr<ExtDeviceManager> extDevMgr)
        : busType_(busType), extDevMgr_(extDevMgr) {};
    int32_t OnDeviceAdd(std::shared_ptr<DeviceInfo> device) override;
    int32_t OnDeviceRemove(std::shared_ptr<DeviceInfo> device) override;

private:
    BusType busType_;
    std::shared_ptr<ExtDeviceManager> extDevMgr_;
};

class IBusExtension;

class BusExtensionCore {
    DECLARE_DELAYED_SINGLETON(BusExtensionCore)
public:
    int32_t Init();
    int32_t Register(BusType busType, std::shared_ptr<IBusExtension> busExtension);

private:
    std::unordered_map<BusType, std::shared_ptr<IBusExtension>> busExtensions_;
    const uint32_t MAX_BUS_EXTENSIONS = 100;
};

// bus extension should register by __attribute__ ((constructor)) when loading so
template <typename BusExtension>
void RegisterBusExtension(BusType busType)
{
    DelayedSingleton<BusExtensionCore>::GetInstance()->Register(\
        busType, std::make_shared<BusExtension>());
}
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // EXT_OBJECT_H