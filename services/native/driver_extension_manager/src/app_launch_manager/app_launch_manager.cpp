/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "app_launch_manager.h"

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace ExternalDeviceManager {

IMPLEMENT_SINGLE_INSTANCE(AppLaunchManager);

AppLaunchManager::~AppLaunchManager()
{
    UnInit();
}

void AppLaunchManager::LoadConfig()
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    if (!config_.LoadConfig()) {
        EDM_LOGW(MODULE_BUS_USB, "LoadConfig failed, app launch feature may not work");
    }
}

void AppLaunchManager::SubscribeCommonEvent()
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    paramSubscriber_.SubscribeEvent([this]() { config_.LoadConfig(); });
}

void AppLaunchManager::UnInit()
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    paramSubscriber_.UnsubscribeEvent();
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (bundleMgr_ != nullptr && bundleMgr_->AsObject()) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(bmsDeathRecipient_);
    }
    bundleMgr_ = nullptr;
    bmsDeathRecipient_ = nullptr;
}

void AppLaunchManager::OnDeviceConnected(uint16_t vid, uint16_t pid)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    std::optional<AppLaunchEntry> entry = config_.FindEntry(vid, pid);
    if (!entry.has_value()) {
        EDM_LOGI(MODULE_BUS_USB, "no config entry found for device");
        return;
    }
    AppInstallStatus status = CheckAppInstallStatus(entry->bundleName);
    if (status == AppInstallStatus::INSTALLED) {
        EDM_LOGI(MODULE_BUS_USB, "app is installed, launching");
        int32_t launchRet = launcher_.LaunchApp(entry->bundleName, entry->abilityName);
        if (launchRet != EDM_OK) {
            EDM_LOGE(MODULE_BUS_USB, "LaunchApp failed, ret=%{public}d", launchRet);
        }
    } else if (status == AppInstallStatus::NOT_INSTALLED) {
        EDM_LOGI(MODULE_BUS_USB, "app is not installed, sending notification");
        int32_t notifyRet = notifier_.SendNotification(*entry);
        if (notifyRet != EDM_OK) {
            EDM_LOGE(MODULE_BUS_USB, "SendNotification failed, ret=%{public}d", notifyRet);
        }
    } else {
        EDM_LOGE(MODULE_BUS_USB, "account not ready, skip app launch for now");
    }
}

AppInstallStatus AppLaunchManager::CheckAppInstallStatus(const std::string &bundleName)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    sptr<AppExecFwk::IBundleMgr> bundleMgr = GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "failed to GetBundleMgrProxy");
        return AppInstallStatus::ACCOUNT_NOT_READY;
    }

    int32_t userId = 0;
    ErrCode getUserIdRet = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (getUserIdRet != 0) {
        EDM_LOGE(MODULE_BUS_USB, "GetForegroundOsAccountLocalId failed, ret=%{public}d", getUserIdRet);
        return AppInstallStatus::ACCOUNT_NOT_READY;
    }

    AppExecFwk::BundleInfo bundleInfo;
    int32_t flags = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_DEFAULT);
    bool ret = bundleMgr->GetBundleInfo(bundleName, flags, bundleInfo, userId);
    if (!ret) {
        EDM_LOGI(MODULE_BUS_USB, "app not installed");
        return AppInstallStatus::NOT_INSTALLED;
    }

    EDM_LOGI(MODULE_BUS_USB, "app is installed");
    return AppInstallStatus::INSTALLED;
}

sptr<AppExecFwk::IBundleMgr> AppLaunchManager::GetBundleMgrProxy()
{
    {
        std::lock_guard<std::mutex> lock(bundleMgrMutex_);
        if (bundleMgr_ != nullptr) {
            return bundleMgr_;
        }
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "GetSystemAbilityManager is null");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObj =
        systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "GetSystemAbility BUNDLE_MGR is null");
        return nullptr;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
    if (bundleMgr == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "iface_cast IBundleMgr get null");
        return nullptr;
    }

    auto deathRecipient_ = new (std::nothrow) AppLaunchBmsDeathRecipient();
    if (deathRecipient_ == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "failed to create BmsDeathRecipient");
        return nullptr;
    }
    remoteObj->AddDeathRecipient(deathRecipient_);

    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (bundleMgr_ != nullptr) {
        remoteObj->RemoveDeathRecipient(deathRecipient);
        return bundleMgr_;
    }
    bundleMgr_ = bundleMgr;
    bmsDeathRecipient_ = deathRecipient;
    return bundleMgr_;
}

void AppLaunchManager::ResetBundleMgr()
{
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (bundleMgr_ != nullptr && bundleMgr_->AsObject()) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(bmsDeathRecipient_);
    }
    bundleMgr_ = nullptr;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
