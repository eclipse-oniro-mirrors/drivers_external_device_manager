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

#ifndef DRIVER_PKG_MANAGER_H
#define DRIVER_PKG_MANAGER_H

#include <stdint.h>
#include <iostream>

#include "bundle_monitor.h"

#include "ibus_extension.h"
#include "driver_bundle_status_callback.h"

namespace DriverExtension {

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace DriverExtension;

enum {
    ERR_DRV_PKG_MGR_ERROR = 1,
};

struct BundleInfoNames {
    string bundleName;
    string abilityName;
};

class DriverPkgManager {
public:
    DriverPkgManager();
    ~DriverPkgManager();

    void PrintTest();

    /**
     * @brief Called at first before Monitor DriverExtension Package.
     * @param bundleName Indicates the bundle name of the application.
     * @param launcherShortcutInfo List of LauncherShortcutInfo objects if obtained.
     * @return Returns true if the function is successfully called; returns false otherwise.
     */
    bool Init();

    BundleInfoNames* QueryMatchDriver(struct DeviceInfo &devInfo);

    void OnBundleUpdate(PCALLBACKFUN pFun);
private:
    shared_ptr<BundleMonitor> bundleMonitor_ = nullptr;
    sptr<DrvBundleStateCallback> bundleStateCallback_ = nullptr;
    BundleInfoNames bundleInfoName_;

    bool RegisterCallback(const sptr<IBundleStatusCallback> &callback);
    bool UnRegisterCallback();
};

} // namespace

#endif // DRIVER_PKG_MANAGER_H