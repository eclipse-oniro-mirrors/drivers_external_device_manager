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

#ifndef DRIVER_BUNDLE_STATUS_CALLBACK_H
#define DRIVER_BUNDLE_STATUS_CALLBACK_H

#include <stdint.h>
#include <vector>
#include <map>
#include <iostream>

#include "bundle_info.h"
#include "bundle_mgr_proxy.h"
#include "extension_ability_info.h"

#include "ibus_extension.h"

namespace DriverExtension {
class DrvBundleStateCallback;

using namespace std;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace DriverExtension;

enum {
    ERR_DRV_STATUS_CALLBACK_ERROR = 1,
};

enum ON_BUNDLE_STATUS{
    BUNDLE_NULL,
    BUNDLE_ADDED = 1,
    BUNDLE_UPDATED,
    BUNDLE_REMOVED,
};

typedef void(*PCALLBACKFUN)(int, string, string);
/* class AACallback
{
    virtual void OnBundleUpdate(DrvBundleStateCallback *obj, PCALLBACKFUN pFun);
};

class AACallbackBundleInfo :public AACallback
{
     void OnBundleUpdate(DrvBundleStateCallback *obj,PCALLBACKFUN pFun) override;
}; */

class DrvBundleStateCallback : public IBundleStatusCallback {
public:
    DrvBundleStateCallback();
    ~DrvBundleStateCallback();

    void PrintTest();

    virtual void OnBundleStateChanged(const uint8_t installType, const int32_t resultCode,
            const std::string &resultMsg, const std::string &bundleName) override;

    /**
     * @brief Called when a new application package has been installed on the device.
     * @param bundleName Indicates the name of the bundle whose state has been installed.
     * @param userId Indicates the id of the bundle whose state has been installed.
     */
    virtual void OnBundleAdded(const std::string &bundleName, const int userId) override;
    /**
     * @brief Called when a new application package has been Updated on the device.
     * @param bundleName Indicates the name of the bundle whose state has been Updated.
     * @param userId Indicates the id of the bundle whose state has been Updated.
     */
    virtual void OnBundleUpdated(const std::string &bundleName, const int userId) override;
    /**
     * @brief Called when a new application package has been Removed on the device.
     * @param bundleName Indicates the name of the bundle whose state has been Removed.
     * @param userId Indicates the id of the bundle whose state has been Removed.
     */
    virtual void OnBundleRemoved(const std::string &bundleName, const int userId) override;
    
    virtual sptr<IRemoteObject> AsObject() override;

    bool GetAllDriverInfos(std::map<string, DriverInfo> &driverInfos);

    bool CheckBundleMgrProxyPermission();

    string GetStiching();

    PCALLBACKFUN m_pFun = nullptr;
private:
    std::vector<ExtensionAbilityInfo> extensionInfos_;
    std::map<string, DriverInfo> innerDrvInfos_;
    std::map<string, DriverInfo> allDrvInfos_;
    std::mutex bundleMgrMutex_;
    sptr<IBundleMgr> bundleMgr_ = nullptr;
    string stiching = "This is used for Name Stiching";

    ErrCode QueryExtensionAbilityInfos(const std::string &bundleName, const int userId);
    bool ParseBaseDriverInfo(int onBundleStatus);

    sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgrProxy();
    int32_t GetCurrentActiveUserId();
    void StorageHistoryDrvInfo(std::vector<BundleInfo> &bundleInfos);

    void OnBundleDrvAdded();
    void OnBundleDrvUpdated();
    void OnBundleDrvRemoved();
};

} // namespace

#endif // DRIVER_BUNDLE_STATUS_CALLBACK_H