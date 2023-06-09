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
#ifndef DEVICE_MANAGER_ETX_DEVICE_MGR_H
#define DEVICE_MANAGER_ETX_DEVICE_MGR_H
#include <memory>
#include <mutex>
#include <list>
#include <unordered_map>

#include "ext_object.h"
namespace OHOS {
namespace ExternalDeviceManager {
class Device final {
public:
    Device(std::shared_ptr<DeviceInfo> info) : info_(info) {};

    bool HasDriver() const;
    std::shared_ptr<DeviceInfo> GetDeviceInfo() const
    {
        return info_;
    }

private:
    std::shared_ptr<DriverInfo> driver_;
    std::shared_ptr<DeviceInfo> info_;
};

class ExtDeviceManager final {
    DECLARE_DELAYED_SINGLETON(ExtDeviceManager)
public:
    int32_t Init();
    int32_t RegisterDevice(std::shared_ptr<DeviceInfo> devInfo);
    void UnRegisterDevice(const std::shared_ptr<DeviceInfo> devInfo);

private:
    std::unordered_map<BusType, std::list<std::shared_ptr<Device>>> deviceMap_;
    std::mutex deviceMapMutex_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DEVICE_MANAGER_ETX_DEVICE_MGR_H