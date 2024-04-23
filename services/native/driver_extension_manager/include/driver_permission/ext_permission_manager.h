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

#include <iostream>
#include <stdint.h>

#include "bundle_info.h"
#include "bundle_mgr_proxy.h"
#include "single_instance.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::ExternalDeviceManager;

enum DDK_PERMISSION {
    ERROR,
    CHECK,
    NOT_CHECK
};

class ExtPermissionManager {
    DECLARE_SINGLE_INSTANCE_BASE(ExtPermissionManager);

public:
    bool HasPermission(std::string permissionName);

    static bool IsSystemApp();

    ~ExtPermissionManager();
private:
    sptr<IBundleMgr> bundleMgr_ = nullptr;
    std::mutex bundleMgrMutex_;
    DDK_PERMISSION NeedCheckPermission();
    sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgrProxy();
    int32_t GetCurrentActiveUserId();
    ExtPermissionManager();
    std::map<std::string, bool> rightsMap_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_PKG_MANAGER_H