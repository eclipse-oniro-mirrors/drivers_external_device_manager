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

#ifndef APP_LAUNCH_MANAGER_H
#define APP_LAUNCH_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include "app_launch_config.h"
#include "bundle_mgr_proxy.h"
#include "app_launch_launcher.h"
#include "app_launch_notifier.h"
#include "app_launch_param_subscriber.h"
#include "single_instance.h"

namespace OHOS {
namespace ExternalDeviceManager {

class AppLaunchManager {
    DECLARE_SINGLE_INSTANCE_BASE(AppLaunchManager);
public:
    ~AppLaunchManager();
    void LoadConfig();
    void SubscribeCommonEvent();
    void UnInit();
    void OnDeviceConnected(uint16_t vid, uint16_t pid);

private:
    AppLaunchManager() = default;
    bool IsAppInstalled(const std::string &bundleName);
    void NotifyWorker();

    AppLaunchConfig config_;
    AppLaunchParamSubscriber paramSubscriber_;
    AppLaunchLauncher launcher_;
    AppLaunchNotifier notifier_;
    std::mutex bundleMgrMutex_;
    sptr<AppExecFwk::IBundleMgr> bundleMgr_ = nullptr;
    bool GetBundleMgrProxy();

    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::vector<AppLaunchEntry> pendingEntries_;
    std::thread workerThread_;
    std::atomic<bool> running_{false};
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // APP_LAUNCH_MANAGER_H
