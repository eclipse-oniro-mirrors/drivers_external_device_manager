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

#include <element_name.h>
#include <want.h>

#include "hdf_log.h"
#include "hilog_wrapper.h"
#include "matching_skills.h"
#include "common_event_support.h"
#include "common_event_subscribe_info.h"
#include "bus_extension_core.h"
#include "pkg_db_helper.h"
#include "driver_pkg_manager.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;

IMPLEMENT_SINGLE_INSTANCE(DriverPkgManager);

DriverPkgManager::DriverPkgManager()
{
};

DriverPkgManager::~DriverPkgManager()
{
    if (UnRegisterCallback() == 0) {
        EDM_LOGE(MODULE_PKG_MGR, "~DriverPkgManager UnRegisterCallback Fail");
    }
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
        EDM_LOGE(MODULE_PKG_MGR, "bundleMonitor_ new Err");
        return EDM_ERR_INVALID_OBJECT;
    }

    bundleStateCallback_ = new DrvBundleStateCallback();

    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "bundleStateCallback_ new Err");
        return EDM_ERR_INVALID_OBJECT;
    }

    if (!bundleStateCallback_->GetAllDriverInfos()) {
        EDM_LOGE(MODULE_PKG_MGR, "bundleStateCallback_ GetAllDriverInfos Err");
        return EDM_ERR_NOT_SUPPORT;
    }
    // register calback to BMS
    return RegisterCallback(bundleStateCallback_);
}

shared_ptr<BundleInfoNames> DriverPkgManager::QueryMatchDriver(shared_ptr<DeviceInfo> devInfo)
{
    EDM_LOGI(MODULE_PKG_MGR, "Enter QueryMatchDriver");
    shared_ptr<IBusExtension> extInstance = nullptr;
    auto ret = make_shared<BundleInfoNames>();
    ret->bundleName.clear();
    ret->abilityName.clear();
    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryMatchDriver bundleStateCallback_ null");
        return nullptr;
    }

    if (!bundleStateCallback_->GetAllDriverInfos()) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryMatchDriver GetAllDriverInfos Err");
        return nullptr;
    }

    std::vector<PkgInfoTable> pkgInfos;
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    int32_t retRdb = helper->QueryPkgInfos(pkgInfos);
    if (retRdb <= 0) {
        /* error or empty record */
        return nullptr;
    }
    EDM_LOGD(MODULE_PKG_MGR, "total driverInfos number: %{public}zu", pkgInfos.size());
    for (const auto &pkgInfo : pkgInfos) {
        DriverInfo driverInfo;
        driverInfo.UnSerialize(pkgInfo.driverInfo);
        extInstance = BusExtensionCore::GetInstance().GetBusExtensionByName(driverInfo.GetBusName());
        if (extInstance != nullptr && extInstance->MatchDriver(driverInfo, *devInfo)) {
            ret->bundleName = pkgInfo.bundleName;
            ret->abilityName = pkgInfo.driverName;
            ret->driverUid = pkgInfo.driverUid;
            return ret;
        }
    }
    EDM_LOGI(MODULE_PKG_MGR, "QueryMatchDriver return null");
    return nullptr;
}

int32_t DriverPkgManager::QueryDriverInfo(vector<shared_ptr<DriverInfo>> &driverInfos,
    bool isByDriverUid, const std::string &driverUid)
{
    EDM_LOGD(MODULE_PKG_MGR, "DriverPkgManager::QueryDriverInfo enter");
    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryDriverInfo bundleStateCallback_ null");
        return EDM_NOK;
    }

    if (!bundleStateCallback_->GetAllDriverInfos()) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryDriverInfo GetAllDriverInfos Err");
        return EDM_NOK;
    }

    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    std::vector<PkgInfoTable> pkgInfos;
    EDM_LOGD(MODULE_PKG_MGR, "pkg QueryDriverInfo driverUid: %{public}s", driverUid.c_str());
    int32_t pkgSize = helper->QueryPkgInfos(pkgInfos, isByDriverUid, driverUid);
    if (pkgSize < 0) {
        return EDM_NOK;
    }
    for (const auto &pkgInfo : pkgInfos) {
        std::shared_ptr<DriverInfo> driverInfo
            = std::make_shared<DriverInfo>(pkgInfo.bundleName, pkgInfo.driverName, pkgInfo.driverUid);
        if (driverInfo->UnSerialize(pkgInfo.driverInfo) != EDM_OK) {
            return EDM_NOK;
        }
        driverInfos.push_back(driverInfo);
    }
    EDM_LOGD(MODULE_PKG_MGR, "DriverPkgManager::QueryDriverInfo driverInfos size:%{public}zu", driverInfos.size());
    return EDM_OK;
}

int32_t DriverPkgManager::RegisterCallback(const sptr<IBundleStatusCallback> &callback)
{
    EDM_LOGI(MODULE_PKG_MGR, "RegisterCallback called");
    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to register callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }

    if (!bundleStateCallback_->CheckBundleMgrProxyPermission()) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to register callback, Permission check is false");
        return EDM_ERR_NOT_SUPPORT;
    }

    if (bundleMonitor_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to register callback, bundleMonitor_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }

    if ((int32_t)(bundleMonitor_->Subscribe(callback)) == 1) {
        return EDM_OK;
    }

    return EDM_NOK;
}

int32_t DriverPkgManager::UnRegisterCallback()
{
    EDM_LOGI(MODULE_PKG_MGR, "UnRegisterCallback called");
    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to unregister callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }

    if (!bundleStateCallback_->CheckBundleMgrProxyPermission()) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to unregister callback, Permission check is false");
        return EDM_ERR_NOT_SUPPORT;
    }

    if (bundleMonitor_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to unregister callback, bundleMonitor is null");
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
        EDM_LOGE(MODULE_PKG_MGR, "failed to register callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }
    bundleStateCallback_->m_pFun = pFun;
    return EDM_OK;
}

int32_t DriverPkgManager::RegisterOnBundleUpdate(ONBUNDLESUPDATE pFun)
{
    if (pFun == nullptr) {
        return EDM_ERR_INVALID_OBJECT;
    }

    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to register onBundlesUpdate, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }
    bundleStateCallback_->onBundlesUpdate = pFun;
    return EDM_OK;
}

int32_t DriverPkgManager::UnRegisterOnBundleUpdate()
{
    if (bundleStateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "failed to unregister callback, bundleStateCallback_ is null");
        return EDM_ERR_INVALID_OBJECT;
    }
    bundleStateCallback_->m_pFun = nullptr;
    bundleStateCallback_->onBundlesUpdate = nullptr;
    return EDM_OK;
}
} // namespace ExternalDeviceManager
} // namespace OHOS