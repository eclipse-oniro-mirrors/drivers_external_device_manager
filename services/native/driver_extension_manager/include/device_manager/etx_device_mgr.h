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
#include <unordered_set>

#include "ext_object.h"
#include "single_instance.h"
#include "timer.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;

class Device final {
public:
    Device(shared_ptr<DeviceInfo> info) : info_(info) {};
    bool HasDriver() const
    {
        return !bundleInfo_.empty();
    };

    shared_ptr<DeviceInfo> GetDeviceInfo() const
    {
        return info_;
    }

    void AddBundleInfo(const string &bundleInfo)
    {
        bundleInfo_ = bundleInfo;
    }

    void RemoveBundleInfo()
    {
        bundleInfo_.clear();
    }

    string GetBundleInfo() const
    {
        return bundleInfo_;
    }
private:
    string bundleInfo_;
    shared_ptr<DriverInfo> driver_;
    shared_ptr<DeviceInfo> info_;
};

class ExtDeviceManager final {
    DECLARE_SINGLE_INSTANCE_BASE(ExtDeviceManager);

public:
    ~ExtDeviceManager() = default;
    int32_t Init();
    int32_t RegisterDevice(shared_ptr<DeviceInfo> devInfo);
    int32_t UnRegisterDevice(const shared_ptr<DeviceInfo> devInfo);
    vector<shared_ptr<DeviceInfo>> QueryDevice(const BusType busType);
    static int32_t UpdateBundleStatusCallback(int32_t bundleStatus, int32_t busType,
        const string &bundleName, const string &abilityName);
private:
    ExtDeviceManager() = default;
    void PrintMatchDriverMap();
    string GetBundleName(string &bundleInfo) const;
    string GetAbilityName(string &bundleInfo) const;
    int32_t AddDevIdOfBundleInfoMap(uint64_t deviceId, string &bundleInfo);
    int32_t RemoveDevIdOfBundleInfoMap(uint64_t deviceId, string &bundleInfo);
    int32_t RemoveAllDevIdOfBundleInfoMap(string &bundleInfo);
    int32_t AddBundleInfo(enum BusType busType, const string &bundleName, const string &abilityName);
    int32_t RemoveBundleInfo(enum BusType busType, const string &bundleName, const string &abilityName);
    int32_t UpdateBundleInfo(enum BusType busType, const string &bundleName, const string &abilityName);
    void UnLoadSelf(void);
    size_t GetTotalDeviceNum(void) const;
    string stiching_ = "_stiching_";
    unordered_map<BusType, unordered_map<uint64_t, shared_ptr<Device>>> deviceMap_;
    unordered_map<string, unordered_set<uint64_t>> bundleMatchMap_; // driver matching table
    mutex deviceMapMutex_;
    mutex bundleMatchMapMutex_;
    Utils::Timer unloadSelftimer_ {"unLoadSelfTimer"};
    uint32_t unloadSelftimerId_ {UINT32_MAX};
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DEVICE_MANAGER_ETX_DEVICE_MGR_H