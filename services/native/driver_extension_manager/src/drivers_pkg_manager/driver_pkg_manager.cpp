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

#include "driver_pkg_manager.h"
#include "usb_bus_extension.h"

#include <want.h>
#include <element_name.h>

#include "common_event_subscribe_info.h"
#include "common_event_support.h"
#include "matching_skills.h"

#include "hdf_log.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;

DriverPkgManager::DriverPkgManager()
{
};

DriverPkgManager::~DriverPkgManager()
{
    if (UnRegisterCallback() == 0) {
        HDF_LOGE("~DriverPkgManager UnRegisterCallback Fail");
    }
    delete bundleStateCallback_;
    bundleStateCallback_ = nullptr;
}

void DriverPkgManager::PrintTest()
{
    bundleStateCallback_->PrintTest();
}

int32_t DriverPkgManager::Init()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    bundleMonitor_ = std::make_shared<BundleMonitor>(subscribeInfo);

    if (bundleMonitor_ == nullptr) {
        HDF_LOGE("bundleMonitor_ new Err");
        return EDM_ERR_INVALID_OBJECT;
    }

    bundleStateCallback_ = new DrvBundleStateCallback();

    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("bundleStateCallback_ new Err");
        return EDM_ERR_INVALID_OBJECT;
    }

    std::map<string, DriverInfo> drvInfos_;
    if (!bundleStateCallback_->GetAllDriverInfos(drvInfos_)) {
        HDF_LOGE("bundleStateCallback_ GetAllDriverInfos Err");
        return EDM_ERR_NOT_SUPPORT;
    }
    // register calback to BMS
    return RegisterCallback(bundleStateCallback_);
}

shared_ptr<BundleInfoNames> DriverPkgManager::QueryMatchDriver(shared_ptr<DeviceInfo> devInfo)
{
    HDF_LOGD("Enter QueryMatchDriver");
    shared_ptr<IBusExtension> extInstance = nullptr;
    auto ret = make_shared<BundleInfoNames>();
    ret->bundleName.clear();
    ret->abilityName.clear();

    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("QueryMatchDriver bundleStateCallback_ null");
        return nullptr;
    }

    std::map<string, DriverInfo> drvInfos_;
    if (!bundleStateCallback_->GetAllDriverInfos(drvInfos_)) {
        HDF_LOGE("QueryMatchDriver GetAllDriverInfos Err");
        return nullptr;
    }

    if (drvInfos_.empty()) {
        HDF_LOGD("QueryMatchDriver drvInfos_ Empty");
        return nullptr;
    }

    for (auto [key, val] : drvInfos_) {
        extInstance = IBusExtension::GetInstance(val.GetBusName());
        if (extInstance == nullptr) {
            HDF_LOGD("QueryMatchDriver GetInstance at bus:%{public}s", val.GetBusName().c_str());
            continue;
        }

        if (extInstance->MatchDriver(val, *devInfo)) {
            string bundleName = key;
            ret->bundleName =
            bundleName.substr(0, bundleName.find_first_of(bundleStateCallback_->GetStiching()));
            ret->abilityName =
            bundleName.substr(bundleName.find_last_of(bundleStateCallback_->GetStiching()) + 1);
            return ret;
        }
    }

    HDF_LOGD("QueryMatchDriver return null");
    return nullptr;
}

int32_t DriverPkgManager::RegisterCallback(const sptr<IBundleStatusCallback> &callback)
{
    HDF_LOGD("RegisterCallback called");
    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("failed to register callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }

    if (!bundleStateCallback_->CheckBundleMgrProxyPermission()) {
        HDF_LOGE("failed to register callback, Permission check is false");
        return EDM_ERR_NOT_SUPPORT;
    }

    if (bundleMonitor_ == nullptr) {
        HDF_LOGE("failed to register callback, bundleMonitor_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }

    if ((int32_t)(bundleMonitor_->Subscribe(callback)) == 1) {
        return EDM_OK;
    }

    return EDM_NOK;
}

int32_t DriverPkgManager::UnRegisterCallback()
{
    HDF_LOGD("UnRegisterCallback called");
    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("failed to unregister callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }

    if (!bundleStateCallback_->CheckBundleMgrProxyPermission()) {
        HDF_LOGE("failed to unregister callback, Permission check is false");
        return EDM_ERR_NOT_SUPPORT;
    }

    if (bundleMonitor_ == nullptr) {
        HDF_LOGE("failed to unregister callback, bundleMonitor is null");
        return EDM_ERR_INVALID_OBJECT;
    }

    if ((int32_t)(bundleMonitor_->UnSubscribe()) == 1) {
        return EDM_OK;
    }

    return EDM_NOK;
}

int32_t DriverPkgManager::RegisterOnBundleUpdate(PCALLBACKFUN pFun)
{
    if (pFun == nullptr) {
        return EDM_ERR_INVALID_OBJECT;
    }

    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("failed to register callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }
    bundleStateCallback_->m_pFun = pFun;
    return EDM_OK;
}

int32_t DriverPkgManager::UnRegisterOnBundleUpdate()
{
    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("failed to unregister callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }
    bundleStateCallback_->m_pFun = nullptr;
    return EDM_OK;
}
}
}