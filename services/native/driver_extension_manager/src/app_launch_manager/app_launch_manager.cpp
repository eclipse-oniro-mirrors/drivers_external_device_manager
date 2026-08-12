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

#include <algorithm>
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
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (!running_) {
        running_ = true;
        workerThread_ = std::thread(&AppLaunchManager::NotifyWorker, this);
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
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        running_ = false;
    }
    queueCv_.notify_one();
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void AppLaunchManager::OnDeviceConnected(uint16_t vid, uint16_t pid)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter, vid=%{public}04x, pid=%{public}04x", __func__, vid, pid);
    std::optional<AppLaunchEntry> entry = config_.FindEntry(vid, pid);
    if (!entry.has_value()) {
        EDM_LOGI(MODULE_BUS_USB, "no config entry found for vid=%{public}04x pid=%{public}04x", vid, pid);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        auto it = std::find_if(pendingEntries_.begin(), pendingEntries_.end(),
            [&entry](const AppLaunchEntry &e) { return e.bundleName == entry->bundleName; });
        if (it == pendingEntries_.end()) {
            pendingEntries_.push_back(*entry);
            EDM_LOGI(MODULE_BUS_USB, "queued entry for vid=%{public}04x pid=%{public}04x bundle=%{public}s",
                vid, pid, entry->bundleName.c_str());
        }
    }
    queueCv_.notify_one();
}

void AppLaunchManager::NotifyWorker()
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s started", __func__);
    while (running_) {
        AppLaunchEntry entry;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this] { return !pendingEntries_.empty() || !running_; });
            if (!running_ && pendingEntries_.empty()) {
                break;
            }
            entry = pendingEntries_.front();
            pendingEntries_.erase(pendingEntries_.begin());
        }

        EDM_LOGI(MODULE_BUS_USB, "processing entry for bundle=%{public}s", entry.bundleName.c_str());
        if (IsAppInstalled(entry.bundleName)) {
            EDM_LOGI(MODULE_BUS_USB, "app %{public}s is installed, launching", entry.bundleName.c_str());
            int32_t launchRet = launcher_.LaunchApp(entry.bundleName, entry.abilityName);
            EDM_LOGI(MODULE_BUS_USB, "LaunchApp for %{public}s ret=%{public}d",
                entry.bundleName.c_str(), launchRet);
            if (launchRet != EDM_OK) {
                EDM_LOGE(MODULE_BUS_USB, "LaunchApp failed for %{public}s, ret=%{public}d",
                    entry.bundleName.c_str(), launchRet);
            }
        } else {
            EDM_LOGI(MODULE_BUS_USB, "app %{public}s is not installed, sending notification",
                entry.bundleName.c_str());
            int32_t notifyRet = notifier_.SendNotification(entry);
            EDM_LOGI(MODULE_BUS_USB, "SendNotification for %{public}s ret=%{public}d",
                entry.bundleName.c_str(), notifyRet);
            if (notifyRet != EDM_OK) {
                EDM_LOGE(MODULE_BUS_USB, "SendNotification failed for %{public}s, ret=%{public}d",
                    entry.bundleName.c_str(), notifyRet);
            }
        }
    }
    EDM_LOGI(MODULE_BUS_USB, "%{public}s exited", __func__);
}

bool AppLaunchManager::IsAppInstalled(const std::string &bundleName)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter, bundleName=%{public}s", __func__, bundleName.c_str());
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
        EDM_LOGI(MODULE_BUS_USB, "GetBundleInfo failed for %{public}s, app not installed", bundleName.c_str());
        return false;
    }

    EDM_LOGI(MODULE_BUS_USB, "bundle %{public}s is installed", bundleName.c_str());
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

    EDM_LOGI(MODULE_BUS_USB, "GetBundleMgrProxy success");
    return true;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
