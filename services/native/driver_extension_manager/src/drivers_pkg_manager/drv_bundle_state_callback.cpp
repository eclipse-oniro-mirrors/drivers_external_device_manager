/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "accesstoken_kit.h"
#include <unordered_map>
#include <pthread.h>
#include <thread>
#include "driver_report_sys_event.h"
#include "usb_driver_info.h"
#include <memory.h>
namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;
using namespace OHOS::Security::AccessToken;

constexpr uint64_t LABEL = HITRACE_TAG_OHOS;
const string DRV_INFO_BUS = "bus";
const string DRV_INFO_VENDOR = "vendor";
const string DRV_INFO_DESC = "description";
const string DRV_INFO_LAUNCHONBIND = "launchonbind";
const string DRV_INFO_ALLOW_ACCESSED = "ohos.permission.ACCESS_DDK_ALLOWED";

static constexpr const char *BUNDLE_RESET_TASK_NAME = "DRIVER_INFO_RESET";
static constexpr const char *BUNDLE_UPDATE_TASK_NAME = "DRIVER_INFO_UPDATE";
static constexpr const char *GET_DRIVERINFO_TASK_NAME = "GET_DRIVERINFO_ASYNC";

std::string DrvBundleStateCallback::GetBundleSize(const std::string &bundleName)
{
    std::string bundleSize = "";
    int32_t userId = GetCurrentActiveUserId();
    std::vector<int64_t> bundleStats;
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (!GetBundleMgrProxy()) {
        EDM_LOGE(MODULE_PKG_MGR, "%{public}s: failed to GetBundleMgrProxy", __func__);
        return bundleSize;
    }
    if (!bundleMgr_->GetBundleStats(bundleName, userId, bundleStats)) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleStats failed");
        return bundleSize;
    }
    if (!bundleStats.empty()) {
        bundleSize.append(std::to_string(bundleStats[0]));
    }
    return bundleSize;
}

void DrvBundleStateCallback::ChangeValue(DriverInfo &tmpDrvInfo, const map<string, string> &metadata)
{
    for (auto data : metadata) {
        if (data.first == DRV_INFO_BUS) {
            tmpDrvInfo.bus_ = data.second;
            tmpDrvInfo.busType_ = BusExtensionCore::GetBusTypeByName(tmpDrvInfo.bus_);
        }
        if (data.first == DRV_INFO_VENDOR) {
            tmpDrvInfo.vendor_ = data.second;
        }
        if (data.first == DRV_INFO_DESC) {
            tmpDrvInfo.description_ = data.second;
        }
        if (LowerStr(data.first) == DRV_INFO_LAUNCHONBIND) {
            tmpDrvInfo.launchOnBind_ = (data.second == "true");
        }
        if (data.first == DRV_INFO_ALLOW_ACCESSED) {
            tmpDrvInfo.accessAllowed_ = (data.second == "true");
        }
    }
}

std::vector<DriverInfo> DrvBundleStateCallback::ParseToPkgInfoTables(
    const std::vector<ExtensionAbilityInfo> &driverInfos, std::vector<PkgInfoTable> &pkgInfoTables)
{
    std::unordered_map<std::string, std::string> bundlesSize;
    shared_ptr<IBusExtension> extInstance = nullptr;
    std::vector<DriverInfo> ret;
    for (const auto &driverInfo : driverInfos) {
        if (driverInfo.type != ExtensionAbilityType::DRIVER || driverInfo.metadata.empty()) {
            continue;
        }
        DriverInfo tmpDrvInfo(driverInfo.bundleName, driverInfo.name);
        map<string, string> metaMap;
        for (auto meta : driverInfo.metadata) {
            metaMap.emplace(meta.name, meta.value);
        }
        ChangeValue(tmpDrvInfo, metaMap);
        extInstance = BusExtensionCore::GetInstance().GetBusExtensionByName(tmpDrvInfo.GetBusName());
        if (extInstance == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "GetBusExtensionByName failed, bus:%{public}s", tmpDrvInfo.bus_.c_str());
            continue;
        }
        tmpDrvInfo.driverInfoExt_ = extInstance->ParseDriverInfo(metaMap);
        if (tmpDrvInfo.driverInfoExt_ == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "ParseDriverInfo null");
            continue;
        }

        if (bundlesSize.find(driverInfo.bundleName) == bundlesSize.end()) {
            std::string bundleSize = GetBundleSize(driverInfo.bundleName);
            bundlesSize.emplace(driverInfo.bundleName, bundleSize);
        }

        tmpDrvInfo.driverSize_ = bundlesSize[driverInfo.bundleName];
        tmpDrvInfo.version_ = driverInfo.applicationInfo.versionName;
        string driverInfoStr;
        if (tmpDrvInfo.Serialize(driverInfoStr) != EDM_OK) {
            EDM_LOGE(MODULE_PKG_MGR, "Serialize driverInfo faild");
            continue;
        }

        PkgInfoTable pkgInfo = CreatePkgInfoTable(driverInfo, driverInfoStr);
        tmpDrvInfo.driverUid_ = pkgInfo.driverUid;
        tmpDrvInfo.userId_ = pkgInfo.userId;
        pkgInfoTables.emplace_back(pkgInfo);
        ret.emplace_back(tmpDrvInfo);
    }
    return ret;
}

PkgInfoTable DrvBundleStateCallback::CreatePkgInfoTable(const ExtensionAbilityInfo &driverInfo, string driverInfoStr)
{
    PkgInfoTable pkgInfo = {
        .driverUid = driverInfo.name + "-" + std::to_string(driverInfo.applicationInfo.accessTokenId),
        .bundleAbility = driverInfo.bundleName + "-" + driverInfo.name,
        .bundleName = driverInfo.bundleName,
        .driverName = driverInfo.name,
        .driverInfo = driverInfoStr
    };
    HapTokenInfo hapTokenInfo;
    if (AccessTokenKit::GetHapTokenInfo(driverInfo.applicationInfo.accessTokenId, hapTokenInfo)
        == AccessTokenKitRet::RET_SUCCESS) {
        pkgInfo.userId = hapTokenInfo.userID;
        pkgInfo.appIndex = hapTokenInfo.instIndex;
    }
    return pkgInfo;
}

DrvBundleStateCallback::DrvBundleStateCallback()
{
    stiching.clear();
    stiching += "-";
};

DrvBundleStateCallback::DrvBundleStateCallback(shared_future<int32_t> bmsFuture, shared_future<int32_t> accountFuture,
    shared_future<int32_t> commEventFuture): DrvBundleStateCallback()
{
    bmsFuture_ = bmsFuture;
    accountFuture_ = accountFuture;
    commEventFuture_ = commEventFuture;
}

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
    if (!IsCurrentUserId(userId)) {
        return;
    }
    std::vector<ExtensionAbilityInfo> driverInfos;
    if (!QueryDriverInfos(bundleName, userId, driverInfos)) {
        auto extDevEvent = std::make_shared<ExtDevEvent>(__func__, DRIVER_PACKAGE_DATA_REFRESH);
        if (extDevEvent == nullptr) {
            return;
        }
        extDevEvent->bundleName = bundleName;
        extDevEvent->userId = userId;
        ExtDevReportSysEvent::ReportExternalDeviceEvent(extDevEvent,
            ExtDevReportSysEvent::EventErrCode::QUERY_DRIVER_EXTENSION_FAILED);
        return;
    }

    if (!UpdateToRdb(driverInfos, bundleName, __func__)) {
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
    if (!IsCurrentUserId(userId)) {
        return;
    }
    std::vector<ExtensionAbilityInfo> driverInfos;
    if (!QueryDriverInfos(bundleName, userId, driverInfos)) {
        auto extDevEvent = std::make_shared<ExtDevEvent>(__func__, DRIVER_PACKAGE_DATA_REFRESH);
        if (extDevEvent == nullptr) {
            return;
        }
        extDevEvent->bundleName = bundleName;
        extDevEvent->userId = userId;
        ExtDevReportSysEvent::ReportExternalDeviceEvent(extDevEvent,
            ExtDevReportSysEvent::EventErrCode::QUERY_DRIVER_EXTENSION_FAILED);
        return;
    }

    if (driverInfos.empty()) {
        OnBundleDrvRemoved(bundleName, __func__);
        return;
    }

    if (!UpdateToRdb(driverInfos, bundleName, __func__)) {
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
    if (!IsCurrentUserId(userId)) {
        return;
    }
    OnBundleDrvRemoved(bundleName, __func__);
    FinishTrace(LABEL);
}

sptr<IRemoteObject> DrvBundleStateCallback::AsObject()
{
    return nullptr;
}

void DrvBundleStateCallback::ResetInitOnce()
{
    std::lock_guard<std::mutex> lock(initOnceMutex_);
    initOnce = false;
}

void DrvBundleStateCallback::ResetMatchedBundles(const int32_t userId)
{
    if (bundleUpdateCallback_ == nullptr) {
        EDM_LOGE(MODULE_PKG_MGR, "DrvBundleStateCallback::ResetMatchedBundles bundleUpdateCallback_ is null");
        return;
    }
    std::thread taskThread([userId, this]() {
        bundleUpdateCallback_->OnBundlesReseted(userId);
    });
    pthread_setname_np(taskThread.native_handle(), BUNDLE_RESET_TASK_NAME);
    taskThread.detach();
}

bool DrvBundleStateCallback::GetAllDriverInfos()
{
    std::lock_guard<std::mutex> lock(initOnceMutex_);
    if (initOnce) {
        EDM_LOGI(MODULE_PKG_MGR, "GetAllDriverInfos has inited");
        return true;
    }
    
    std::vector<ExtensionAbilityInfo> driverInfos;
    int32_t userId = GetCurrentActiveUserId();
    if (userId == Constants::INVALID_USERID) {
        EDM_LOGI(MODULE_PKG_MGR, "GetCurrentActiveUserId userId is invalid");
        return false;
    }
    EDM_LOGI(MODULE_PKG_MGR, "QueryExtensionAbilityInfos userId:%{public}d", userId);
    bundleMgrMutex_.lock();
    if (!GetBundleMgrProxy()) {
        EDM_LOGE(MODULE_PKG_MGR, "%{public}s: failed to GetBundleMgrProxy", __func__);
        bundleMgrMutex_.unlock();
        return false;
    }
    bundleMgr_->QueryExtensionAbilityInfos(ExtensionAbilityType::DRIVER, userId, driverInfos);
    bundleMgrMutex_.unlock();
    if (!UpdateToRdb(driverInfos, "", __func__)) {
        EDM_LOGE(MODULE_PKG_MGR, "UpdateToRdb failed");
        return false;
    }
    initOnce = true;
    return true;
}

void DrvBundleStateCallback::GetAllDriverInfosAsync()
{
    EDM_LOGI(MODULE_PKG_MGR, "GetAllDriverInfosAsync enter");
    std::thread taskThread([this]() {
        bmsFuture_.wait();
        accountFuture_.wait();
        if (!GetAllDriverInfos()) {
            EDM_LOGE(MODULE_PKG_MGR, "GetAllDriverInfos failed");
        }
    });
    pthread_setname_np(taskThread.native_handle(), GET_DRIVERINFO_TASK_NAME);
    taskThread.detach();
}

string DrvBundleStateCallback::GetStiching()
{
    return stiching;
}

bool DrvBundleStateCallback::CheckBundleMgrProxyPermission()
{
    // check permission
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (!GetBundleMgrProxy()) {
        EDM_LOGE(MODULE_PKG_MGR, "%{public}s: failed to GetBundleMgrProxy", __func__);
        return false;
    }
    if (!bundleMgr_->VerifySystemApi(Constants::INVALID_API_VERSION)) {
        EDM_LOGE(MODULE_PKG_MGR, "non-system app calling system api");
        return false;
    }
    if (!bundleMgr_->VerifyCallingPermission(Constants::LISTEN_BUNDLE_CHANGE)) {
        EDM_LOGE(MODULE_PKG_MGR, "register bundle status callback failed due to lack of permission");
        return false;
    }
    return true;
}

bool DrvBundleStateCallback::QueryDriverInfos(const std::string &bundleName, const int userId,
    std::vector<ExtensionAbilityInfo> &driverInfos)
{
    if (bundleName.empty()) {
        EDM_LOGE(MODULE_PKG_MGR, "%{public}s: BundleName is empty", __func__);
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (!GetBundleMgrProxy()) {
        EDM_LOGE(MODULE_PKG_MGR, "%{public}s: failed to GetBundleMgrProxy", __func__);
        return false;
    }
 
    BundleInfo tmpBundleInfo;
    int32_t flags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) + \
                    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) + \
                    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA);
    if (!(bundleMgr_->GetBundleInfo(bundleName, flags, tmpBundleInfo, userId))) {
        EDM_LOGE(MODULE_PKG_MGR, "GetBundleInfo err");
        return false;
    }

    for (auto &extensionInfo : tmpBundleInfo.extensionInfos) {
        if (extensionInfo.type == ExtensionAbilityType::DRIVER) {
            extensionInfo.applicationInfo = tmpBundleInfo.applicationInfo;
            driverInfos.emplace_back(extensionInfo);
        }
    }
    return true;
}

void DrvBundleStateCallback::ClearDriverInfo(DriverInfo &tmpDrvInfo)
{
    tmpDrvInfo.bus_.clear();
    tmpDrvInfo.vendor_.clear();
    tmpDrvInfo.version_.clear();
    tmpDrvInfo.driverInfoExt_ = nullptr;
}

static void ReportPkgsEvent(const std::vector<DriverInfo> &driverObjs, const std::string &interfaceName,
    const ExtDevReportSysEvent::EventErrCode errCode)
{
    EDM_LOGI(MODULE_PKG_MGR, "ReportPkgsEvent enter, interfaceName:%{public}s, %{public}zu", interfaceName.c_str(),
        driverObjs.size());
    auto extDevEvent = std::make_shared<ExtDevEvent>(interfaceName, DRIVER_PACKAGE_DATA_REFRESH);
    for (const auto &driverObj : driverObjs) {
        ExtDevReportSysEvent::ParseToExtDevEvent(std::make_shared<DriverInfo>(driverObj), extDevEvent);
        ExtDevReportSysEvent::ReportExternalDeviceEvent(extDevEvent, errCode);
    }
}

static void ReportPkgsDelEvent(const std::vector<PkgInfoTable> &pkgInfos,
    const std::string &interfaceName, const ExtDevReportSysEvent::EventErrCode errCode)
{
    EDM_LOGI(MODULE_PKG_MGR, "ReportPkgsDelEvent enter, interfaceName:%{public}s, %{public}zu", interfaceName.c_str(),
        pkgInfos.size());
    auto extDevEvent = std::make_shared<ExtDevEvent>(interfaceName, DRIVER_PACKAGE_DATA_REFRESH);
    for (const auto &pkgInfo : pkgInfos) {
        std::shared_ptr<DriverInfo> driverInfo
            = std::make_shared<DriverInfo>(pkgInfo.bundleName, pkgInfo.driverName, pkgInfo.driverUid, pkgInfo.userId);
        if (driverInfo == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "ReportPkgsDelEvent driverInfo is null");
            continue;
        }
        driverInfo->UnSerialize(pkgInfo.driverInfo);
        ExtDevReportSysEvent::ParseToExtDevEvent(driverInfo, extDevEvent);
        ExtDevReportSysEvent::ReportExternalDeviceEvent(extDevEvent, errCode);
    }
}

bool DrvBundleStateCallback::UpdateToRdb(const std::vector<ExtensionAbilityInfo> &driverInfos,
    const std::string &bundleName, const std::string &interfaceName)
{
    std::vector<PkgInfoTable> pkgInfoTables;
    auto driverObjs = ParseToPkgInfoTables(driverInfos, pkgInfoTables);
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    if (helper->AddOrUpdatePkgInfo(pkgInfoTables, bundleName) < PKG_OK) {
        EDM_LOGE(MODULE_PKG_MGR, "add or update failed,bundleName:%{public}s", bundleName.c_str());
        ReportPkgsEvent(driverObjs, interfaceName, ExtDevReportSysEvent::EventErrCode::UPDATE_DATABASE_FAILED);
        return false;
    }
    ReportPkgsEvent(driverObjs, interfaceName, ExtDevReportSysEvent::EventErrCode::SUCCESS);

    if (bundleUpdateCallback_ != nullptr) {
        std::thread taskThread([bundleName, this]() {
            if (bundleUpdateCallback_ == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "UpdateToRdb bundleUpdateCallback_ is nullptr");
                return;
            }
            bundleUpdateCallback_->OnBundlesUpdated(bundleName);
        });
        pthread_setname_np(taskThread.native_handle(), BUNDLE_UPDATE_TASK_NAME);
        taskThread.detach();
    }
    return true;
}

int32_t DrvBundleStateCallback::GetCurrentActiveUserId()
{
    int32_t localId;
    int32_t ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(localId);
    if (ret != 0) {
        EDM_LOGE(MODULE_PKG_MGR, "GetForegroundOsAccountLocalId failed ret:%{public}d", ret);
        return Constants::INVALID_USERID;
    }
    return localId;
}

bool DrvBundleStateCallback::IsCurrentUserId(const int userId)
{
    return GetCurrentActiveUserId() == userId;
}

bool DrvBundleStateCallback::GetBundleMgrProxy()
{
    if (bundleMgr_ == nullptr) {
        auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "%{public}s: GetSystemAbilityManager is null", __func__);
            return false;
        }
        auto remoteObj = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObj == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "%{public}s: GetSystemAbility is null", __func__);
            return false;
        }
        bundleMgr_ = OHOS::iface_cast<IBundleMgr>(remoteObj);
        if (bundleMgr_ == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "%{public}s: iface_cast get null", __func__);
            return false;
        }
        bmsDeathRecipient_ = new (std::nothrow) BundleMgrDeathRecipient([this]() {
            ResetBundleMgr();
        });
        if (bmsDeathRecipient_ == nullptr) {
            EDM_LOGE(MODULE_PKG_MGR, "%{public}s: failed to create BundleMgrDeathRecipient", __func__);
            bundleMgr_ = nullptr;
            return false;
        }
        remoteObj->AddDeathRecipient(bmsDeathRecipient_);
    }
    return true;
}

void DrvBundleStateCallback::OnBundleDrvRemoved(const std::string &bundleName, const std::string &interfaceName)
{
    std::shared_ptr<PkgDbHelper> helper = PkgDbHelper::GetInstance();
    std::vector<PkgInfoTable> pkgInfos;
    (void)helper->QueryPkgInfos(bundleName, pkgInfos);
    int32_t ret = helper->DeleteRightRecord(bundleName);
    if (ret < 0) {
        EDM_LOGE(MODULE_PKG_MGR, "delete failed: %{public}s", bundleName.c_str());
        ReportPkgsDelEvent(pkgInfos, interfaceName, ExtDevReportSysEvent::EventErrCode::UPDATE_DATABASE_FAILED);
        return;
    }
    ReportPkgsDelEvent(pkgInfos, interfaceName, ExtDevReportSysEvent::EventErrCode::SUCCESS);
    if (bundleUpdateCallback_ != nullptr) {
        std::thread taskThread([bundleName, this]() {
            if (bundleUpdateCallback_ == nullptr) {
                EDM_LOGE(MODULE_PKG_MGR, "OnBundleDrvRemoved bundleUpdateCallback_ is nullptr");
                return;
            }
            bundleUpdateCallback_->OnBundlesUpdated(bundleName);
        });
        pthread_setname_np(taskThread.native_handle(), BUNDLE_UPDATE_TASK_NAME);
        taskThread.detach();
    }
}

void DrvBundleStateCallback::ResetBundleMgr()
{
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (bundleMgr_ != nullptr && bundleMgr_->AsObject()) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(bmsDeathRecipient_);
    }
    bundleMgr_ = nullptr;
}
}
}