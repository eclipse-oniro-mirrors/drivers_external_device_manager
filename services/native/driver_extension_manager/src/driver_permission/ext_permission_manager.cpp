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
#include "ext_permission_manager.h"

#include "os_account_manager.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "accesstoken_kit.h"
#include "privacy_kit.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;
using namespace OHOS::Security::AccessToken;

const uint32_t MIN_TARGET_VERSION = 10;

IMPLEMENT_SINGLE_INSTANCE(ExtPermissionManager);

ExtPermissionManager::ExtPermissionManager()
{
};

ExtPermissionManager::~ExtPermissionManager()
{
};

DDK_PERMISSION ExtPermissionManager::NeedCheckPermission()
{
    auto iBundleMgr = GetBundleMgrProxy();
    if (iBundleMgr == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "Can not get iBundleMgr");
        return DDK_PERMISSION::ERROR;
    }
    int uid = IPCSkeleton::GetCallingUid();
    std::string bundleName;
    if (!iBundleMgr->GetBundleNameForUid(uid, bundleName)) {
        EDM_LOGE(MODULE_DEV_MGR, "GetBundleNameForUid err");
        return DDK_PERMISSION::ERROR;
    }
    EDM_LOGD(MODULE_DEV_MGR, "GetBundleNameForUid bundleName:%{public}s", bundleName.c_str());
    int32_t userId = GetCurrentActiveUserId();
    BundleInfo bundleInfo;
    if (!iBundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId)) {
        EDM_LOGE(MODULE_DEV_MGR, "GetBundleNameForUid err");
        return DDK_PERMISSION::ERROR;
    }
    EDM_LOGD(MODULE_DEV_MGR, "GetBundleInfo bundleInfo targetVersion:%{public}d", bundleInfo.targetVersion);
    if (bundleInfo.targetVersion > MIN_TARGET_VERSION) {
        return DDK_PERMISSION::CHECK;
    }
    return DDK_PERMISSION::NOT_CHECK;
}

bool ExtPermissionManager::HasPermission(std::string permissionName)
{
    DDK_PERMISSION needCheck = NeedCheckPermission();
    if (needCheck != DDK_PERMISSION::CHECK) {
        return needCheck == DDK_PERMISSION::ERROR ? false : true;
    }
    
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int result = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (PrivacyKit::AddPermissionUsedRecord(callerToken, permissionName, static_cast<int32_t>(1 + result),
        static_cast<int32_t>(-result)) != 0) {
        EDM_LOGE(MODULE_DEV_MGR, "AddPermissionUsedRecord failed.");
    }
    if (result != PERMISSION_GRANTED) {
        EDM_LOGE(MODULE_DEV_MGR, "usb_ddk_api:  No permission.");
        return false;
    }
    EDM_LOGI(MODULE_DEV_MGR, "usb_ddk_api: Check permission succeeded.");
    return true;
}

sptr<OHOS::AppExecFwk::IBundleMgr> ExtPermissionManager::GetBundleMgrProxy()
{
    if (bundleMgr_ != nullptr) {
        return bundleMgr_;
    }
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
    return bundleMgr_;
}

int32_t ExtPermissionManager::GetCurrentActiveUserId()
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
} // namespace ExternalDeviceManager
} // namespace OHOS