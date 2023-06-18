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

#include "etx_device_mgr.h"
#include "cinttypes"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace ExternalDeviceManager {
IMPLEMENT_SINGLE_INSTANCE(ExtDeviceManager);

int32_t ExtDeviceManager::Init()
{
    EDM_LOGD(MODULE_DEV_MGR, "ExtDeviceManager Init start");
    return EDM_OK;
}

int32_t ExtDeviceManager::RegisterDevice(std::shared_ptr<DeviceInfo> devInfo)
{
    BusType type = devInfo->GetBusType();
    uint64_t deviceId = devInfo->GetDeviceId();
    std::lock_guard<std::mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(type) != deviceMap_.end()) {
        std::unordered_map<uint64_t, std::shared_ptr<Device>> &map = deviceMap_[type];
        if (map.find(deviceId) != map.end()) {
            EDM_LOGI(MODULE_DEV_MGR, "device has been registered, deviceId is %{public}016" PRIx64 "", deviceId);
            return EDM_OK;
        }
    }
    std::shared_ptr<Device> device = std::make_shared<Device>(devInfo);
    deviceMap_[type].emplace(deviceId, device);
    EDM_LOGD(MODULE_DEV_MGR, "successfully registered device, deviceId is %{public}016" PRIx64 "", deviceId);
    DriverPkgManager::GetInstance();
    return EDM_OK;
}

int32_t ExtDeviceManager::UnRegisterDevice(const std::shared_ptr<DeviceInfo> devInfo)
{
    BusType type = devInfo->GetBusType();
    uint64_t deviceId = devInfo->GetDeviceId();
    std::lock_guard<std::mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(type) != deviceMap_.end()) {
        std::unordered_map<uint64_t, std::shared_ptr<Device>> &map = deviceMap_[type];
        if (map.find(deviceId) != map.end()) {
            map.erase(deviceId);
            EDM_LOGI(MODULE_DEV_MGR, "successfully unregistered device, deviceId is %{public}016" PRIx64 "", deviceId);
            return EDM_OK;
        }
    }
    EDM_LOGD(MODULE_DEV_MGR, "device has been unregistered, deviceId is %{public}016" PRIx64 "", deviceId);
    return EDM_OK;
}

std::vector<std::shared_ptr<DeviceInfo>> ExtDeviceManager::QueryDevice(const BusType busType)
{
    std::vector<std::shared_ptr<DeviceInfo>> devInfoVec;
    std::lock_guard<std::mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(busType) == deviceMap_.end()) {
        EDM_LOGE(MODULE_DEV_MGR, "no device is found or busType %{public}d is invalid", busType);
        return devInfoVec;
    }
    std::unordered_map<uint64_t, std::shared_ptr<Device>> map = deviceMap_[busType];
    for (auto device : map) {
        devInfoVec.push_back(device.second->GetDeviceInfo());
    }
    EDM_LOGD(MODULE_DEV_MGR, "find %{public}zu device of busType %{public}d", devInfoVec.size(), busType);
    return devInfoVec;
}
} // namespace ExternalDeviceManager
} // namespace OHOS