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

#include "drv_bundle_state_callback.h"
#include "usb_bus_extension.h"

#include <want.h>
#include <element_name.h>

#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "bundle_constants.h"
#include "os_account_manager.h"

#include "hdf_log.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "bus_extension_core.h"
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;

constexpr uint64_t LABEL = HITRACE_TAG_OHOS;
const string DRV_INFO_BUS = "bus";
const string DRV_INFO_VENDOR = "vendor";
const string DRV_INFO_VERSION = "version";

DrvBundleStateCallback::DrvBundleStateCallback()
{
    allDrvInfos_.clear();
    stiching.clear();
    stiching += "########";

    std::map<string, DriverInfo> drvInfos_;
    if (GetAllDriverInfos(drvInfos_)) {
        EDM_LOGE(MODULE_PKG_MGR, "GetAllDriverInfos in DrvBundleStateCallback OK");
    } else {
        EDM_LOGE(MODULE_PKG_MGR, "GetAllDriverInfos in DrvBundleStateCallback ERR");
    }
};

DrvBundleStateCallback::~DrvBundleStateCallback()
{
    return;
};

void DrvBundleStateCallback::PrintTest()
{
    cout << "allDrvInfos_ size = " << allDrvInfos_.size() << endl;
}

void DrvBundleStateCallback::OnBundleStateChanged(const uint8_t installType, const int32_t resultCode,
    const std::string &resultMsg, const std::string &bundleName)
{
    EDM_LOGE(MODULE_PKG_MGR, "OnBundleStateChanged");
    return;
};

/**
    * @brief Called when a new application package has been installed on the device.
    * @param bundleName Indicates the name of the bundle whose state has been installed.
    * @param userId Indicates the id of the bundle whose state has been installed.
    */
void DrvBundleStateCallback::OnBundleAdded(const std::string &bundleName, const int userId)
{
    EDM_LOGE(MODULE_PKG_MGR, "OnBundleAdded");
    StartTrace(LABEL, "OnBundleAdded");
    if (QueryExtensionAbilityInfos(bundleName, userId) != ERR_OK) {
        return;
    }

    if (ParseBaseDriverInfo(BUNDLE_ADDED)) {
        OnBundleDrvAdded();
    }
    FinishTrace(LABEL);
}
/**
    * @brief Called when a new application package has been Updated on the device.
    * @param bundleName Indicates the name of the bundle whose state has been Updated.
    * @param userId Indicates the id of the bundle whose state has been Updated.
    */
void DrvBundleStateCallback::OnBundleUpdated(const std::string &bundleName, const int userId)
{
    EDM_LOGE(MODULE_PKG_MGR, "OnBundleUpdated");
    StartTrace(LABEL, "OnBundleUpdated");
    if (QueryExtensionAbilityInfos(bundleName, userId) != ERR_OK) {
        return;
    }

    if (ParseBaseDriverInfo(BUNDLE_UPDATED)) {
        OnBundleDrvUpdated();
    }
    FinishTrace(LABEL);
}

/**
    * @brief Called when a new application package has been Removed on the device.
    * @param bundleName Indicates the name of the bundle whose state has been Removed.
    * @param userId Indicates the id of the bundle whose state has been Removed.
    */
void DrvBundleStateCallback::OnBundleRemoved(const std::string &bundleName, const int userId)
{
    EDM_LOGE(MODULE_PKG_MGR, "OnBundleRemoved");
    StartTrace(LABEL, "OnBundleRemoved");

    OnBundleDrvRemoved(bundleName);

    FinishTrace(LABEL);
}

sptr<IRemoteObject> DrvBundleStateCallback::AsObject()
{
    return nullptr;
}

bool DrvBundleStateCallback::GetAllDriverInfos(std::map<string, DriverInfo> &driverInfos)
{
    if (initOnce) {
        driverInfos = allDrvInfos_;
        return true;
    }

    // query history bundle
    auto iBundleMgr = GetBundleMgrProxy();
    if (iBundleMgr == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Can not get iBundleMgr");
        driverInfos = allDrvInfos_;
        return false;
    }
    std::vector<BundleInfo> bundleInfos;
  
    int32_t userId = GetCurrentActiveUserId();
    int32_t flags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) + \
                    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA);
    if (!(iBundleMgr->GetBundleInfos(flags, bundleInfos, userId))) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleInfos err");
        driverInfos = allDrvInfos_;
        return false;
    }

    StorageHistoryDrvInfo(bundleInfos);
    initOnce = true;
    return true;
}

string DrvBundleStateCallback::GetStiching()
{
    return stiching;
}

bool DrvBundleStateCallback::CheckBundleMgrProxyPermission()
{
    // check permission
    auto iBundleMgr = GetBundleMgrProxy();
    if (iBundleMgr == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Can not get iBundleMgr");
        return false;
    }
    if (!iBundleMgr->VerifySystemApi(Constants::INVALID_API_VERSION)) {
        EDM_LOGE(MODULE_PKG_MGR, "non-system app calling system api");
        return false;
    }
    if (!iBundleMgr->VerifyCallingPermission(Constants::LISTEN_BUNDLE_CHANGE)) {
        EDM_LOGE(MODULE_PKG_MGR, "register bundle status callback failed due to lack of permission");
        return false;
    }
    return true;
}

ErrCode DrvBundleStateCallback::QueryExtensionAbilityInfos(const std::string &bundleName, const int userId)
{
    // clear ExtensionAbilityInfos vector
    extensionInfos_.clear();

    if (bundleName.empty()) {
        EDM_LOGE(MODULE_PKG_MGR, "BundleName empty");
        return ERR_DRV_STATUS_CALLBACK_ERROR;
    }

    if (bundleMgr_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "BundleMgr_ nullptr");
        return ERR_DRV_STATUS_CALLBACK_ERROR;
    }
 
    BundleInfo tmpBundleInfo;
    int32_t flags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) + \
                    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA);
    if (!(bundleMgr_->GetBundleInfo(bundleName, flags, tmpBundleInfo, userId))) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleInfo err");
        return ERR_DRV_STATUS_CALLBACK_ERROR;
    }

    extensionInfos_ = tmpBundleInfo.extensionInfos;
    if (extensionInfos_.empty()) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleInfo extensionInfos_ empty");
        return ERR_DRV_STATUS_CALLBACK_ERROR;
    }

    return ERR_OK;
}

void DrvBundleStateCallback::ChangeValue(DriverInfo &tmpDrvInfo, std::vector<Metadata> &metadata)
{
    for (auto data : metadata) {
        if (data.name == DRV_INFO_BUS) {
            tmpDrvInfo.bus_ = data.value;
        }
        if (data.name == DRV_INFO_VENDOR) {
            tmpDrvInfo.vendor_ = data.value;
        }
        if (data.name == DRV_INFO_VERSION) {
            tmpDrvInfo.version_ = data.value;
        }
    }
}

bool DrvBundleStateCallback::ParseBaseDriverInfo(int bundleStatus)
{
    shared_ptr<IBusExtension> extInstance = nullptr;
    DriverInfo tmpDrvInfo;
    ExtensionAbilityType type;
    std::vector<Metadata> metadata;
    string bundleName;
    string abilityName;
    bool ret = false;

    // clear DriverInfos vector
    innerDrvInfos_.clear();

    // parase infos to innerDrvInfos_
    while (!extensionInfos_.empty()) {
        tmpDrvInfo.bus_.clear();
        tmpDrvInfo.vendor_.clear();
        tmpDrvInfo.version_.clear();
        tmpDrvInfo.driverInfoExt_ = nullptr;

        type = extensionInfos_.back().type;
        metadata = extensionInfos_.back().metadata;
        bundleName = extensionInfos_.back().bundleName;
        abilityName = extensionInfos_.back().name;
        extensionInfos_.pop_back();

        if ((type != ExtensionAbilityType::DRIVER) || metadata.empty()) {
            continue;
        }

        ChangeValue(tmpDrvInfo, metadata);

        if (tmpDrvInfo.GetBusName() == "USB") {
            extInstance = BusExtensionCore::GetInstance().GetBusExtensionByName("USB");
        }

        if (extInstance == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "QueryMatchDriver GetInstance at bus:%{public}s", tmpDrvInfo.bus_.c_str());
            continue;
        }

        tmpDrvInfo.driverInfoExt_ = extInstance->ParseDriverInfo(metadata);
        if (tmpDrvInfo.driverInfoExt_ == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "ParseDriverInfo null");
            continue;
        }

        if (m_pFun != nullptr) {
            m_pFun(bundleStatus, BusType::BUS_TYPE_USB, bundleName, abilityName);
        }

        bundleName += stiching + abilityName;
        innerDrvInfos_[bundleName] = tmpDrvInfo;
        ret = true;
    }
    return ret;
}

int32_t DrvBundleStateCallback::GetCurrentActiveUserId()
{
    std::vector<int32_t> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryActiveOsAccountIds failed ret:%{public}d", ret);
        return Constants::INVALID_USERID;
    }
    if (activeIds.empty()) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryActiveOsAccountIds activeIds empty");
        return Constants::ALL_USERID;
    }
    return activeIds[0];
}

sptr<OHOS::AppExecFwk::IBundleMgr> DrvBundleStateCallback::GetBundleMgrProxy()
{
    if (bundleMgr_ == nullptr) {
        std::lock_guard<std::mutex> lock(bundleMgrMutex_);
        if (bundleMgr_ == nullptr) {
            auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityManager == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "GetBundleMgr GetSystemAbilityManager is null");
                return nullptr;
            }
            auto bundleMgrSa = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
            if (bundleMgrSa == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "GetBundleMgr GetSystemAbility is null");
                return nullptr;
            }
            auto bundleMgr = OHOS::iface_cast<IBundleMgr>(bundleMgrSa);
            if (bundleMgr == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "GetBundleMgr iface_cast get null");
            }
            bundleMgr_ = bundleMgr;
        }
    }
    return bundleMgr_;
}

void DrvBundleStateCallback::StorageHistoryDrvInfo(std::vector<BundleInfo> &bundleInfos)
{
    allDrvInfos_.clear();
    while (!bundleInfos.empty()) {
        extensionInfos_.clear();
        extensionInfos_ = bundleInfos.back().extensionInfos;
        bundleInfos.pop_back();
        if (extensionInfos_.empty()) {
            continue;
        }

        if (ParseBaseDriverInfo(BUNDLE_NULL)) {
            OnBundleDrvAdded();
        }
    }
}

void DrvBundleStateCallback::OnBundleDrvAdded()
{
    for (auto ele : innerDrvInfos_) {
        allDrvInfos_[ele.first] = innerDrvInfos_[ele.first];
    }
}

void DrvBundleStateCallback::OnBundleDrvUpdated()
{
    OnBundleDrvAdded();
}

void DrvBundleStateCallback::OnBundleDrvRemoved(const std::string &bundleName)
{
    for (auto iter = allDrvInfos_.begin(); iter != allDrvInfos_.end();) {
        if (iter->first.find(bundleName) != std::string::npos) {
            iter = allDrvInfos_.erase(iter);
        } else {
            ++iter;
        }
    }
}
}
}