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

class DriverInfo {
public:
    virtual ~DriverInfo();

private:
    std::string bundleName_;
    std::string abilityName_;
    std::string vendor_;
    std::string version_;
    BusType busType_;
};

class DeviceInfo {
public:
    DeviceInfo(uint32_t busDeviceId, const std::string &description = "") : description_(description)
    {
        devInfo_.devBusInfo.busDeviceId = busDeviceId;
    }
    virtual ~DeviceInfo();

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
class DevChangeCallback final {
public:
    DevChangeCallback(BusType busType, std::shared_ptr<ExtDeviceManager> extDevMgr)
        : busType_(busType), extDevMgr_(extDevMgr) {};
    int32_t OnDeviceAdd(std::shared_ptr<DeviceInfo> device);
    int32_t OnDeviceRemove(std::shared_ptr<DeviceInfo> device);

private:
    BusType busType_;
    std::shared_ptr<ExtDeviceManager> extDevMgr_;
};

class IBusExtension {
public:
    virtual ~IBusExtension() = 0;
    virtual std::shared_ptr<DriverInfo> ParseDriverInfo() = 0;
    virtual bool MatchDriver(std::shared_ptr<DeviceInfo> device, std::shared_ptr<DriverInfo> driver) = 0;
    virtual void SetDevChangeCallback(DevChangeCallback &cb);

private:
};

class BusExtensionCore {
public:
    DECLARE_DELAYED_SINGLETON(BusExtensionCore)
    int32_t Init();
    int32_t RegisterBusExtension(BusType busType, std::shared_ptr<IBusExtension> busExtension);

private:
    std::unordered_map<BusType, std::shared_ptr<IBusExtension>> busExtensions_;
    const uint32_t MAX_BUS_EXTENSIONS = 100;
};

// bus extension should register by __attribute__ ((constructor)) when loading so
template <typename BusExtension>
void RegisterBustension(BusType busType)
{
    DelayedSingleton<BusExtensionCore>::GetInstance()->RegisterBusExtension(busType, std::make_shared<BusExtension>());
}
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // EXT_OBJECT_H