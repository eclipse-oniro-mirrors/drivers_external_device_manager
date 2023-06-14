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

#include <dlfcn.h>
#include <sstream>

#include "cinttypes"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "ibus_extension.h"
#include "etx_device_mgr.h"

#ifdef __aarch64__
static constexpr const char *BUS_EXTENSION_SO_PATH = "/system/lib64";
#else
static constexpr const char *BUS_EXTENSION_SO_PATH = "/system/lib";
#endif
static constexpr const char *HDI_SO_SUFFIX = ".z.so";
static constexpr const char *HDI_SO_PREFIX = "lib";
static constexpr const char *USB_BUS_EXTENSION = "bus_extension";

namespace OHOS {
namespace ExternalDeviceManager {
IMPLEMENT_SINGLE_INSTANCE(BusExtensionCore);
IMPLEMENT_SINGLE_INSTANCE(ExtDeviceManager);

static void LoadLib()
{
    for (BusType i = BUS_TYPE_USB; i < BUS_TYPE_MAX; i = (BusType)(i + 1)) {
        std::ostringstream libPath;
        libPath << BUS_EXTENSION_SO_PATH << "/" << HDI_SO_PREFIX;
        switch (i) {
            case BUS_TYPE_USB:
                libPath << USB_BUS_EXTENSION;
                break;
            default:
                EDM_LOGE(MODULE_DEV_MGR, "invalid bus type");
                continue;
        }
        libPath << HDI_SO_SUFFIX;
        void *handler = dlopen(libPath.str().c_str(), RTLD_LAZY);
        if (handler == nullptr) {
            EDM_LOGE(MODULE_DEV_MGR, "failed to dlopen  %{public}s, %{public}s", libPath.str().c_str(), dlerror());
            continue;
        }
    }
}

int32_t BusExtensionCore::Init()
{
    LoadLib();
    int ret = EDM_OK;
    for (auto &iter : busExtensions_) {
        std::shared_ptr<DevChangeCallback> callback =
            std::make_shared<DevChangeCallback>(iter.first, ExtDeviceManager::GetInstance());
        if (iter.second->SetDevChangeCallback(callback) != EDM_OK) {
            ret = EDM_NOK;
            EDM_LOGE(MODULE_DEV_MGR, "busExtension init failed, busType is %{public}d", iter.first);
        }
        EDM_LOGD(MODULE_DEV_MGR, "busExtension init successfully, busType is %{public}d", iter.first);
    }
    return ret;
}

int32_t BusExtensionCore::Register(BusType busType, std::shared_ptr<IBusExtension> busExtension)
{
    if (busExtensions_.size() == MAX_BUS_EXTENSIONS) {
        EDM_LOGE(MODULE_DEV_MGR, "the maximum number of busextension registrations has been reached");
        return EDM_NOK;
    }
    if (busExtensions_.count(busType) > 0) {
        EDM_LOGI(MODULE_DEV_MGR, "busType %{public}d has been registered", busType);
        return EDM_OK;
    }
    busExtensions_.insert(std::make_pair(busType, busExtension));
    EDM_LOGD(MODULE_DEV_MGR, "busType %{public}d register successfully", busType);
    return EDM_OK;
}

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
    if (deviceMap_.count(type) > 0) {
        std::list<std::shared_ptr<Device>> &list = deviceMap_[type];
        std::list<std::shared_ptr<Device>>::iterator iter;
        for (iter = list.begin(); iter != list.end(); iter++) {
            std::shared_ptr<Device> device = *iter;
            if (device->GetDeviceInfo()->GetDeviceId() == deviceId) {
                EDM_LOGI(MODULE_DEV_MGR, "device has been registered, deviceId is %{public}016" PRIx64 "", deviceId);
                return EDM_OK;
            }
        }
    }
    std::shared_ptr<Device> device = std::make_shared<Device>(devInfo);
    deviceMap_[type].push_back(device);
    EDM_LOGD(MODULE_DEV_MGR, "successfully registered device, deviceId is %{public}016" PRIx64 "", deviceId);
    // driver match
    return EDM_OK;
}

void ExtDeviceManager::UnRegisterDevice(const std::shared_ptr<DeviceInfo> devInfo)
{
    BusType type = devInfo->GetBusType();
    uint64_t deviceId = devInfo->GetDeviceId();
    std::lock_guard<std::mutex> lock(deviceMapMutex_);
    if (deviceMap_.count(type) > 0) {
        std::list<std::shared_ptr<Device>> &list = deviceMap_[type];
        std::list<std::shared_ptr<Device>>::iterator iter;
        for (iter = list.begin(); iter != list.end(); iter++) {
            std::shared_ptr<Device> device = *iter;
            if (device->GetDeviceInfo()->GetDeviceId() == deviceId) {
                iter = list.erase(iter);
                EDM_LOGI(
                    MODULE_DEV_MGR, "successfully unregistered device, deviceId is %{public}016" PRIx64 "", deviceId);
                return;
            }
        }
    }
    EDM_LOGD(MODULE_DEV_MGR, "device has been unregistered, deviceId is %{public}016" PRIx64 "", deviceId);
}

int32_t DevChangeCallback::OnDeviceAdd(std::shared_ptr<DeviceInfo> device)
{
    EDM_LOGD(MODULE_DEV_MGR, "OnDeviceAdd start");
    device->devInfo_.devBusInfo.busType = this->busType_;
    return this->extDevMgr_.RegisterDevice(device);
}

int32_t DevChangeCallback::OnDeviceRemove(std::shared_ptr<DeviceInfo> device)
{
    EDM_LOGD(MODULE_DEV_MGR, "OnDeviceRemove start");
    device->devInfo_.devBusInfo.busType = this->busType_;
    this->extDevMgr_.UnRegisterDevice(device);
    return EDM_OK;
}
} // namespace ExternalDeviceManager
} // namespace OHOS