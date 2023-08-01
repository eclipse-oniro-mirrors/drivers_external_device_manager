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
#include  "pkg_db_helper.h"

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
    stiching.clear();
    stiching += "########";

    if (GetAllDriverInfos()) {
        EDM_LOGI(MODULE_PKG_MGR, "GetAllDriverInfos in DrvBundleStateCallback OK");
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
    std::vector<std::string> allBundleAbilityNames;
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    int32_t ret = helper->QueryAllSize(allBundleAbilityNames);
    if (ret <= 0) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryAllSize failed");
        return;
    }
    cout << "allBundleAbilityNames_ size = " << allBundleAbilityNames.size() << endl;
}

void DrvBundleStateCallback::OnBundleStateChanged(const uint8_t installType, const int32_t resultCode,
    const std::string &resultMsg, const std::string &bundleName)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleStateChanged");
    return;
};

/**
    * @brief Called when a new application package has been installed on the device.
    * @param bundleName Indicates the name of the bundle whose state has been installed.
    * @param userId Indicates the id of the bundle whose state has been installed.
    */
void DrvBundleStateCallback::OnBundleAdded(const std::string &bundleName, const int userId)
{
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleAdded");
    StartTrace(LABEL, "OnBundleAdded");
    if (QueryExtensionAbilityInfos(bundleName, userId) != ERR_OK) {
        return;
    }

    if (ParseBaseDriverInfo()) {
        EDM_LOGE(MODULE_PKG_MGR, "OnBundleAdded error");
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
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleUpdated");
    StartTrace(LABEL, "OnBundleUpdated");
    if (QueryExtensionAbilityInfos(bundleName, userId) != ERR_OK) {
        return;
    }

    if (ParseBaseDriverInfo()) {
        EDM_LOGE(MODULE_PKG_MGR, "OnBundleUpdated error");
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
    EDM_LOGI(MODULE_PKG_MGR, "OnBundleRemoved");
    StartTrace(LABEL, "OnBundleRemoved");
    OnBundleDrvRemoved(bundleName);
    FinishTrace(LABEL);
}

sptr<IRemoteObject> DrvBundleStateCallback::AsObject()
{
    return nullptr;
}

bool DrvBundleStateCallback::GetAllDriverInfos()
{
    if (initOnce) {
        return true;
    }

    // query history bundle
    auto iBundleMgr = GetBundleMgrProxy();
    if (iBundleMgr == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "Can not get iBundleMgr");
        return false;
    }
    std::vector<BundleInfo> bundleInfos;
  
    int32_t userId = GetCurrentActiveUserId();
    int32_t flags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) + \
                    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA);
    if (!(iBundleMgr->GetBundleInfos(flags, bundleInfos, userId))) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleInfos err");
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

bool DrvBundleStateCallback::ParseBaseDriverInfo()
{
    shared_ptr<IBusExtension> extInstance = nullptr;
    DriverInfo tmpDrvInfo;
    ExtensionAbilityType type;
    std::vector<Metadata> metadata;
    string bundleName;
    string abilityName;
    bool ret = false;

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

        extInstance = BusExtensionCore::GetInstance().GetBusExtensionByName(tmpDrvInfo.GetBusName());
        if (extInstance == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "QueryMatchDriver GetInstance at bus:%{public}s", tmpDrvInfo.bus_.c_str());
            continue;
        }

        tmpDrvInfo.driverInfoExt_ = extInstance->ParseDriverInfo(metadata);
        if (tmpDrvInfo.driverInfoExt_ == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "ParseDriverInfo null");
            continue;
        }
        string driverInfo;
        tmpDrvInfo.Serialize(driverInfo);

        string tempbundleName = bundleName;
        bundleName += stiching + abilityName;

        std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
        bool flag = true;
        helper->CheckIfNeedUpdateEx(flag, bundleName);
        int32_t addOrUpdate = helper->AddOrUpdateRightRecord(tempbundleName, bundleName, driverInfo);

        if (addOrUpdate < 0) {
            EDM_LOGE(MODULE_PKG_MGR, "add or update failed: %{public}s, addOrUpdate=%{public}d", bundleName.c_str(), addOrUpdate);
            continue;
        }

        if(m_pFun != nullptr) {
            if(flag) {
                m_pFun(BUNDLE_UPDATED, BusType::BUS_TYPE_USB, tempbundleName, abilityName);
            } else {
                m_pFun(BUNDLE_ADDED, BusType::BUS_TYPE_USB, tempbundleName, abilityName);
            }
        }
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
    while (!bundleInfos.empty()) {
        extensionInfos_.clear();
        extensionInfos_ = bundleInfos.back().extensionInfos;
        bundleInfos.pop_back();
        if (extensionInfos_.empty()) {
            continue;
        }

        if (ParseBaseDriverInfo()) {
            EDM_LOGE(MODULE_PKG_MGR, "OnBundleAdded error");
        }
    }
}

void DrvBundleStateCallback::OnBundleDrvRemoved(const std::string &bundleName)
{
    std::vector<std::string> bundleAbilityNames;
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    int32_t retRdb = helper->QueryAllBundleAbilityNames(bundleName, bundleAbilityNames);
    if (retRdb <= 0) {
        EDM_LOGE(MODULE_PKG_MGR, "QueryAllBundleAbilityNames failed: %{public}s", bundleName.c_str());
        return;
    }
    int32_t totalNames = static_cast<int32_t>(bundleAbilityNames.size());
    EDM_LOGE(MODULE_PKG_MGR, "totalbundleAbilityNames: %{public}d", totalNames);
    for (int32_t i = 0; i < totalNames; i++) {
        if (m_pFun != nullptr) {
            std::string bundleAbilityName = bundleAbilityNames.at(i);
            std::string bundleName = bundleAbilityName.substr(0, bundleAbilityName.find_first_of(GetStiching()));
            std::string abilityName = bundleAbilityName.substr(bundleAbilityName.find_last_of(GetStiching()) + 1);
            EDM_LOGI(MODULE_PKG_MGR, "bundleName: %{public}s abilityName: %{public}s", bundleName.c_str(), abilityName.c_str());      
            m_pFun(BUNDLE_REMOVED, BusType::BUS_TYPE_USB, bundleName, abilityName);
        }
    }

    int32_t ret = helper->DeleteRightRecord(bundleName);
    if (ret < 0) {
        EDM_LOGE(MODULE_PKG_MGR, "delete failed: %{public}s", bundleName.c_str());
        return;
    }
}
}
}