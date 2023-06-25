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
#include "common_timer_errors.h"
#include "driver_extension_controller.h"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace ExternalDeviceManager {
constexpr uint32_t UNLOAD_SA_TIMER_INTERVAL = 30 * 1000;
IMPLEMENT_SINGLE_INSTANCE(ExtDeviceManager);

void ExtDeviceManager::PrintMatchDriverMap()
{
    if (!bundleMatchMap_.empty()) {
        EDM_LOGD(MODULE_DEV_MGR, " bundleInfo map size[%{public}zu]", bundleMatchMap_.size());
        for (auto iter : bundleMatchMap_) {
            for (auto iterId = iter.second.begin(); iterId != iter.second.end(); ++iterId) {
                uint64_t deviceId = *iterId;
                EDM_LOGD(MODULE_DEV_MGR, "print match map info[%{public}s], deviceId %{public}016" PRIX64 "",
                    iter.first.c_str(), deviceId);
            }
        }
        EDM_LOGD(MODULE_DEV_MGR, "ExtDeviceManager print driver match map success");
    }
}

int32_t ExtDeviceManager::Init()
{
    EDM_LOGD(MODULE_DEV_MGR, "ExtDeviceManager Init success");
    int32_t ret = DriverPkgManager::GetInstance().RegisterOnBundleUpdate(UpdateBundleStatusCallback);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "register bundle status callback fail");
        return EDM_NOK;
    }
    return EDM_OK;
}

string ExtDeviceManager::GetBundleName(string &bundleInfo) const
{
    string::size_type pos = bundleInfo.find(stiching_);
    if (pos == string::npos) {
        EDM_LOGI(MODULE_DEV_MGR, "bundleInfo not find stiching name");
        return "";
    }

    return bundleInfo.substr(0, pos);
}

string ExtDeviceManager::GetAbilityName(string &bundleInfo) const
{
    string::size_type pos = bundleInfo.find(stiching_);
    if (pos == string::npos) {
        EDM_LOGI(MODULE_DEV_MGR, "bundleInfo not find stiching name");
        return "";
    }

    return bundleInfo.substr(pos + stiching_.length());
}

int32_t ExtDeviceManager::AddDevIdOfBundleInfoMap(shared_ptr<Device> device, string &bundleInfo)
{
    if (bundleInfo.empty() || device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "bundleInfo or device is null");
        return EDM_NOK;
    }

    // update bundle info
    lock_guard<mutex> lock(bundleMatchMapMutex_);
    auto pos = bundleMatchMap_.find(bundleInfo);
    uint64_t deviceId = device->GetDeviceInfo()->GetDeviceId();
    if (pos == bundleMatchMap_.end()) {
        unordered_set<uint64_t> tmpSet;
        tmpSet.emplace(deviceId);
        bundleMatchMap_.emplace(bundleInfo, tmpSet);
        EDM_LOGD(MODULE_DEV_MGR, "bundleMap emplace New driver, add deviceId %{public}016" PRIX64 "", deviceId);
    } else {
        auto pairRet = pos->second.emplace(deviceId);
        // Check whether the deviceId matches the driver
        if (!pairRet.second || pos->second.size() > 1) {
            EDM_LOGD(MODULE_DEV_MGR, "bundleMap had existed driver, add deviceId %{public}016" PRIX64 "", deviceId);
            PrintMatchDriverMap();
        }
    }

    // start ability
    PrintMatchDriverMap();
    return EDM_OK;
}

int32_t ExtDeviceManager::RemoveDevIdOfBundleInfoMap(shared_ptr<Device> device, string &bundleInfo)
{
    if (bundleInfo.empty() || device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "bundleInfo or device is null");
        return EDM_NOK;
    }

    // update bundle info
    lock_guard<mutex> lock(bundleMatchMapMutex_);
    auto pos = bundleMatchMap_.find(bundleInfo);
    if (pos == bundleMatchMap_.end()) {
        EDM_LOGI(MODULE_DEV_MGR, "not find bundleInfo from map");
        return EDM_OK;
    }

    // If the number of devices is greater than one, only the device erase
    uint64_t deviceId = device->GetDeviceInfo()->GetDeviceId();
    if (pos->second.size() > 1) {
        pos->second.erase(deviceId);
        EDM_LOGD(MODULE_DEV_MGR, "bundleMap existed driver, remove deviceId %{public}016" PRIX64 "", deviceId);
        PrintMatchDriverMap();
        return EDM_OK;
    }

    EDM_LOGD(MODULE_DEV_MGR, "bundleMap remove bundleInfo[%{public}s]", bundleInfo.c_str());
    bundleMatchMap_.erase(pos);

    PrintMatchDriverMap();
    return EDM_OK;
}

int32_t ExtDeviceManager::RemoveAllDevIdOfBundleInfoMap(shared_ptr<Device> device, string &bundleInfo)
{
    if (device == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "device is null");
        return EDM_ERR_INVALID_PARAM;
    }
    // update bundle info
    lock_guard<mutex> lock(bundleMatchMapMutex_);
    auto pos = bundleMatchMap_.find(bundleInfo);
    if (pos == bundleMatchMap_.end()) {
        return EDM_OK;
    }

    bundleMatchMap_.erase(pos);
    return EDM_OK;
}

int32_t ExtDeviceManager::AddBundleInfo(enum BusType busType, const string &bundleName, const string &abilityName)
{
    if (busType <= BUS_TYPE_INVALID || busType > BUS_TYPE_TEST) {
        EDM_LOGE(MODULE_DEV_MGR, "busType para invalid");
        return EDM_ERR_INVALID_PARAM;
    }

    if (bundleName.empty() || abilityName.empty()) {
        EDM_LOGE(MODULE_DEV_MGR, "BundleInfo para invalid");
        return EDM_ERR_INVALID_PARAM;
    }

    // iterate over device, find bundleInfo and ability status
    lock_guard<mutex> lock(deviceMapMutex_);

    // find device
    if (deviceMap_.count(busType) == 0) {
        EDM_LOGD(MODULE_DEV_MGR, "deviceMap_ not bustype[%{public}d], wait to plug device", busType);
        return EDM_OK;
    }

    string bundleInfo = bundleName + stiching_ + abilityName;
    unordered_map<uint64_t, shared_ptr<Device>> &map = deviceMap_[busType];

    for (auto iter : map) {
        shared_ptr<Device> device = iter.second;

        // iterate over device by bustype
        auto bundleInfoNames = DriverPkgManager::GetInstance().QueryMatchDriver(device->GetDeviceInfo());
        if (bundleInfoNames == nullptr) {
            EDM_LOGD(MODULE_DEV_MGR, "deviceId[%{public}016" PRIX64 "], not find driver",
                device->GetDeviceInfo()->GetDeviceId());
            continue;
        }

        if (bundleName.compare(bundleInfoNames->bundleName) == 0 &&
            abilityName.compare(bundleInfoNames->abilityName) == 0) {
            device->AddBundleInfo(bundleInfo);
            int32_t ret = AddDevIdOfBundleInfoMap(device, bundleInfo);
            if (ret != EDM_OK) {
                EDM_LOGE(MODULE_DEV_MGR,
                "deviceId[%{public}016" PRIX64 "] start driver extension ability[%{public}s] fail[%{public}d]",
                device->GetDeviceInfo()->GetDeviceId(), GetAbilityName(bundleInfo).c_str(), ret);
            }
        }
    }

    return EDM_OK;
}

int32_t ExtDeviceManager::RemoveBundleInfo(enum BusType busType, const string &bundleName, const string &abilityName)
{
    if (busType <= BUS_TYPE_INVALID || busType >= BUS_TYPE_TEST) {
        EDM_LOGE(MODULE_DEV_MGR, "busType para invalid");
        return EDM_ERR_INVALID_PARAM;
    }

    if (bundleName.empty() || abilityName.empty()) {
        EDM_LOGE(MODULE_DEV_MGR, "BundleInfo para invalid");
        return EDM_ERR_INVALID_PARAM;
    }

    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.count(busType) == 0) {
        EDM_LOGD(MODULE_DEV_MGR, "deviceMap_ not bustype[%{public}d], wait to plug device", busType);
        return EDM_OK;
    }

    // iterate over device, remove bundleInfo
    string bundleInfo = bundleName + stiching_ + abilityName;
    unordered_map<uint64_t, shared_ptr<Device>> &deviceMap = deviceMap_[busType];

    for (auto iter : deviceMap) {
        shared_ptr<Device> device = iter.second;

        // iterate over device by bustype
        if (bundleInfo.compare(device->GetBundleInfo()) == 0) {
            device->RemoveBundleInfo(); // update device
            int32_t ret = RemoveAllDevIdOfBundleInfoMap(device, bundleInfo);
            if (ret != EDM_OK) {
                EDM_LOGE(MODULE_DEV_MGR,
                "deviceId[%{public}016" PRIX64 "] stop driver extension ability[%{public}s] fail[%{public}d]",
                device->GetDeviceInfo()->GetDeviceId(), GetAbilityName(bundleInfo).c_str(), ret);
            }
        }
    }
    return EDM_OK;
}

int32_t ExtDeviceManager::UpdateBundleInfo(enum BusType busType, const string &bundleName, const string &abilityName)
{
    // stop ability of device and reset bundleInfo of device
    int32_t ret = RemoveBundleInfo(busType, bundleName, abilityName);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "remove bundle info fail");
        return EDM_NOK;
    }

    // iterate over device, add bundleInfo and start ability
    ret = AddBundleInfo(busType, bundleName, abilityName);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "add bundle info fail");
        return EDM_NOK;
    }

    return EDM_OK;
}

int32_t ExtDeviceManager::UpdateBundleStatusCallback(int32_t bundleStatus, int32_t busType,
    const string &bundleName, const string &abilityName)
{
    int32_t ret = EDM_NOK;
    if (bundleStatus < BUNDLE_ADDED || bundleStatus > BUNDLE_REMOVED) {
        EDM_LOGE(MODULE_DEV_MGR, "bundleStatus para invalid");
        return EDM_ERR_INVALID_PARAM;
    }

    // add bundle
    if (bundleStatus == BUNDLE_ADDED) {
        ret = ExtDeviceManager::GetInstance().AddBundleInfo((enum BusType)busType, bundleName, abilityName);
        if (ret != EDM_OK) {
            EDM_LOGE(MODULE_DEV_MGR, "callback add bundle info fail");
        }
        return ret;
    }

    // update bundle
    if (bundleStatus == BUNDLE_UPDATED) {
        ret = ExtDeviceManager::GetInstance().UpdateBundleInfo((enum BusType)busType, bundleName, abilityName);
        if (ret != EDM_OK) {
            EDM_LOGE(MODULE_DEV_MGR, "callback update bundle info fail");
        }
        return ret;
    }

    // remove bundle
    ret = ExtDeviceManager::GetInstance().RemoveBundleInfo((enum BusType)busType, bundleName, abilityName);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "callback update bundle info fail");
    }

    return ret;
}

int32_t ExtDeviceManager::RegisterDevice(shared_ptr<DeviceInfo> devInfo)
{
    BusType type = devInfo->GetBusType();
    uint64_t deviceId = devInfo->GetDeviceId();

    shared_ptr<Device> device;
    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(type) != deviceMap_.end()) {
        unordered_map<uint64_t, shared_ptr<Device>> &map = deviceMap_[type];
        if (map.find(deviceId) != map.end()) {
            // device has been registered and need to connect
            EDM_LOGI(MODULE_DEV_MGR, "device has been registered, deviceId is %{public}016" PRIx64 "", deviceId);
        }
    }
    EDM_LOGD(MODULE_DEV_MGR, "begin to register device, deviceId is %{public}016" PRIx64 "", deviceId);

    // driver match
    string bundleInfo = device->GetBundleInfo();
    // if device does not have a matching driver, match driver here
    if (bundleInfo.empty()) {
        auto bundleInfoNames = DriverPkgManager::GetInstance().QueryMatchDriver(devInfo);
        if (bundleInfoNames != nullptr) {
            bundleInfo = bundleInfoNames->bundleName + stiching_ + bundleInfoNames->abilityName;
            device->AddBundleInfo(bundleInfo);
        }
    }
    // register device
    deviceMap_[type].emplace(deviceId, device);
    EDM_LOGD(MODULE_DEV_MGR, "successfully registered device, deviceId = %{public}016" PRIx64 "", deviceId);
    unloadSelftimer_.Unregister(unloadSelftimerId_);

    // match driver failed, waitting to install driver package
    if (bundleInfo.empty()) {
        EDM_LOGD(MODULE_DEV_MGR,
            "deviceId %{public}016" PRIX64 "match driver failed, waitting to install ext driver package", deviceId);
        return EDM_OK;
    }

    int32_t ret = AddDevIdOfBundleInfoMap(device, bundleInfo);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "deviceId[%{public}016" PRIX64 "] update bundle info map failed[%{public}d]", deviceId,
            ret);
        return EDM_NOK;
    }
    EDM_LOGD(MODULE_DEV_MGR, "successfully match driver[%{public}s], deviceId is %{public}016" PRIx64 "",
        bundleInfo.c_str(), deviceId);

    return ret;
}

int32_t ExtDeviceManager::UnRegisterDevice(const shared_ptr<DeviceInfo> devInfo)
{
    BusType type = devInfo->GetBusType();
    uint64_t deviceId = devInfo->GetDeviceId();
    shared_ptr<Device> device;
    string bundleInfo;

    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(type) != deviceMap_.end()) {
        unordered_map<uint64_t, shared_ptr<Device>> &map = deviceMap_[type];
        if (map.find(deviceId) != map.end()) {
            device = map[deviceId];
            bundleInfo = map[deviceId]->GetBundleInfo();
            map.erase(deviceId);
            EDM_LOGI(MODULE_DEV_MGR, "successfully unregistered device, deviceId is %{public}016" PRIx64 "", deviceId);
            UnLoadSelf();
        }
    }

    if (bundleInfo.empty()) {
        EDM_LOGD(MODULE_DEV_MGR, "deviceId %{public}016" PRIX64 " bundleInfo is empty", deviceId);
        return EDM_OK;
    }

    int32_t ret = RemoveDevIdOfBundleInfoMap(device, bundleInfo);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "deviceId[%{public}016" PRIX64 "] remove bundleInfo map failed[%{public}d]",
            deviceId, ret);
        return ret;
    }

    EDM_LOGI(MODULE_DEV_MGR, "successfully remove bundleInfo, deviceId %{public}016" PRIx64 ", bundleInfo[%{public}s]",
        deviceId, bundleInfo.c_str());

    return EDM_OK;
}

vector<shared_ptr<DeviceInfo>> ExtDeviceManager::QueryDevice(const BusType busType)
{
    vector<shared_ptr<DeviceInfo>> devInfoVec;

    lock_guard<mutex> lock(deviceMapMutex_);
    if (deviceMap_.find(busType) == deviceMap_.end()) {
        EDM_LOGE(MODULE_DEV_MGR, "no device is found and busType %{public}d is invalid", busType);
        return devInfoVec;
    }

    unordered_map<uint64_t, shared_ptr<Device>> map = deviceMap_[busType];
    for (auto device : map) {
        devInfoVec.emplace_back(device.second->GetDeviceInfo());
    }
    EDM_LOGD(MODULE_DEV_MGR, "find %{public}zu device of busType %{public}d", devInfoVec.size(), busType);

    return devInfoVec;
}

size_t ExtDeviceManager::GetTotalDeviceNum(void) const
{
    // Please do not add lock. This will be called in the UnRegisterDevice.
    size_t totalNum = 0;
    for (auto &[_, device] : deviceMap_) {
        totalNum += device.size();
    }
    EDM_LOGD(MODULE_DEV_MGR, "total device num is %{public}zu", totalNum);
    return totalNum;
}

void ExtDeviceManager::UnLoadSelf(void)
{
    unloadSelftimer_.Unregister(unloadSelftimerId_);
    unloadSelftimer_.Shutdown();
    if (GetTotalDeviceNum() != 0) {
        EDM_LOGI(MODULE_DEV_MGR, "not need unload");
        return;
    }

    if (auto ret = unloadSelftimer_.Setup(); ret != Utils::TIMER_ERR_OK) {
        EDM_LOGE(MODULE_DEV_MGR, "set up timer failed %{public}u", ret);
        return;
    }

    auto task = []() {
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "get samgr failed");
            return;
        }

        auto ret = samgrProxy->UnloadSystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID);
        if (ret != EDM_OK) {
            EDM_LOGE(MODULE_DEV_MGR, "unload failed");
        }
    };
    unloadSelftimerId_ = unloadSelftimer_.Register(task, UNLOAD_SA_TIMER_INTERVAL, true);
}
} // namespace ExternalDeviceManager
} // namespace OHOS