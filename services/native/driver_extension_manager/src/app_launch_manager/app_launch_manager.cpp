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
}

void AppLaunchManager::OnDeviceConnected(uint16_t vid, uint16_t pid)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    std::optional<AppLaunchEntry> entry = config_.FindEntry(vid, pid);
    if (!entry.has_value()) {
        EDM_LOGI(MODULE_BUS_USB, "no config entry found for device");
        return;
    }
    if (IsAppInstalled(entry->bundleName)) {
        EDM_LOGI(MODULE_BUS_USB, "app is installed, launching");
        int32_t launchRet = launcher_.LaunchApp(entry->bundleName, entry->abilityName);
        if (launchRet != EDM_OK) {
            EDM_LOGE(MODULE_BUS_USB, "LaunchApp failed, ret=%{public}d", launchRet);
        }
    } else {
        EDM_LOGI(MODULE_BUS_USB, "app is not installed, sending notification");
        int32_t notifyRet = notifier_.SendNotification(*entry);
        if (notifyRet != EDM_OK) {
            EDM_LOGE(MODULE_BUS_USB, "SendNotification failed, ret=%{public}d", notifyRet);
        }
    }
}

bool AppLaunchManager::IsAppInstalled(const std::string &bundleName)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    std::lock_guard<std::mutex> lock(bundleMgrMutex_);
    if (!GetBundleMgrProxy()) {
        EDM_LOGE(MODULE_BUS_USB, "failed to GetBundleMgrProxy");
        return false;
    }

    AppExecFwk::BundleInfo bundleInfo;
    int32_t flags = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_DEFAULT);
    int32_t userId = 0;
    ErrCode getUserIdRet = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (getUserIdRet != 0) {
        EDM_LOGE(MODULE_BUS_USB, "GetForegroundOsAccountLocalId failed, ret=%{public}d", getUserIdRet);
        return false;
    }
    bool ret = bundleMgr_->GetBundleInfo(bundleName, flags, bundleInfo, userId);
    if (!ret) {
        EDM_LOGI(MODULE_BUS_USB, "app not installed");
        return false;
    }

    EDM_LOGI(MODULE_BUS_USB, "app is installed");
    return true;
}

bool AppLaunchManager::GetBundleMgrProxy()
{
    if (bundleMgr_ != nullptr) {
        return true;
    }

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "GetSystemAbilityManager is null");
        return false;
    }

    sptr<IRemoteObject> remoteObj =
        systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "GetSystemAbility BUNDLE_MGR is null");
        return false;
    }

    bundleMgr_ = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
    if (bundleMgr_ == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "iface_cast IBundleMgr get null");
        return false;
    }

    return true;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
