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

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace DriverExtension;

DriverPkgManager::DriverPkgManager() 
{
    cout << "DriverPkgManager" << endl;
};

DriverPkgManager::~DriverPkgManager()
{
    if (!UnRegisterCallback()) {
        HDF_LOGE("~DriverPkgManager UnRegisterCallback Fail");
        cout << "UnRegisterCallback Fail" << endl;
    }
    delete bundleStateCallback_;
    bundleStateCallback_ = nullptr;
}

void DriverPkgManager::PrintTest()
{
    bundleStateCallback_->PrintTest();
}

bool DriverPkgManager::Init()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    bundleMonitor_ = std::make_shared<BundleMonitor>(subscribeInfo);

    if (bundleMonitor_ == nullptr) {
        HDF_LOGE("bundleMonitor_ new Err");
        return false;
    }

    bundleStateCallback_ = new DrvBundleStateCallback();

    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("bundleStateCallback_ new Err");
        return false;
    }

    std::map<string, DriverInfo> drvInfos_;
    if (!bundleStateCallback_->GetAllDriverInfos(drvInfos_)) {
        HDF_LOGE("bundleStateCallback_ GetAllDriverInfos Err");
        return false;
    }

    // register calback to BMS
    return RegisterCallback(bundleStateCallback_);
}

BundleInfoNames* DriverPkgManager::QueryMatchDriver(struct DeviceInfo &devInfo)
{
    HDF_LOGD("Enter QueryMatchDriver");
    shared_ptr<IBusExtension> extInstance = nullptr;
    bundleInfoName_.bundleName.clear();
    bundleInfoName_.abilityName.clear();

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
        cout << "auto [key, val] : drvInfos_" << endl;
        extInstance = IBusExtension::GetInstance(val.bus);
        if (extInstance == nullptr) {
            HDF_LOGD("QueryMatchDriver GetInstance at bus:%{public}s", val.bus.c_str());
            continue;
        }

        if (extInstance->MatchDriver(devInfo, val)) {
            string bundleName = key;
            bundleInfoName_.bundleName = 
            bundleName.substr(0, bundleName.find_first_of(bundleStateCallback_->GetStiching()));
            bundleInfoName_.abilityName = 
            bundleName.substr(bundleName.find_last_of(bundleStateCallback_->GetStiching()) + 1);
            return &bundleInfoName_;
        }
    }

    HDF_LOGD("QueryMatchDriver return null");
    return nullptr;
}

bool DriverPkgManager::RegisterCallback(const sptr<IBundleStatusCallback> &callback)
{
    HDF_LOGD("RegisterCallback called");
    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("failed to register callback, bundleStateCallback_ is null");
        return false;
    }

    if (!bundleStateCallback_->CheckBundleMgrProxyPermission()) {
        HDF_LOGE("failed to register callback, Permission check is false");
        return false;
    }

    if (bundleMonitor_ == nullptr) {
        HDF_LOGE("failed to register callback, bundleMonitor_ is null");
        return false;
    }

    return bundleMonitor_->Subscribe(callback);
}

bool DriverPkgManager::UnRegisterCallback()
{
    HDF_LOGD("UnRegisterCallback called");
    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("failed to unregister callback, bundleStateCallback_ is null");
        return false;
    }

    if (!bundleStateCallback_->CheckBundleMgrProxyPermission()) {
        HDF_LOGE("failed to unregister callback, Permission check is false");
        return false;
    }

    if (bundleMonitor_ == nullptr) {
        HDF_LOGE("failed to unregister callback, bundleMonitor is null");
        return false;
    }

    return bundleMonitor_->UnSubscribe();
}

void DriverPkgManager::RegisterOnBundleUpdate(PCALLBACKFUN pFun)
{
    if (pFun == nullptr) {
        return;
    }

    if (bundleStateCallback_ == nullptr) {
        HDF_LOGE("failed to unregister callback, bundleStateCallback_ is null");
        return;
    }
    bundleStateCallback_->m_pFun = pFun;
}