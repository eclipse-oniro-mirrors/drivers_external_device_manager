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
 *
 * Mock bundle_mgr_proxy.h for unit testing.
 * This header shadows the SDK version by being included first via include_dirs.
 */

#ifndef BUNDLE_MGR_PROXY_H
#define BUNDLE_MGR_PROXY_H

#include <gmock/gmock.h>
#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {

enum class GetBundleInfoFlag {
    GET_BUNDLE_INFO_DEFAULT = 0x00000000,
};

struct BundleInfo {
    bool isNewVersion = false;
    std::string name;
};

class IBundleMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.BundleMgr");

    virtual bool GetBundleInfo(const std::string &bundleName, int32_t flags,
        BundleInfo &bundleInfo, int32_t userId) = 0;

    virtual sptr<IRemoteObject> AsObject() = 0;
};

class MockBundleMgr : public IBundleMgr {
public:
    MOCK_METHOD4(GetBundleInfo, bool(const std::string &bundleName, int32_t flags,
        BundleInfo &bundleInfo, int32_t userId));

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    static sptr<MockBundleMgr> GetInstance()
    {
        static sptr<MockBundleMgr> instance = sptr<MockBundleMgr>::MakeSptr();
        return instance;
    }
};

} // namespace AppExecFwk

template<>
inline sptr<AppExecFwk::IBundleMgr> iface_cast<AppExecFwk::IBundleMgr>(const sptr<IRemoteObject> &object)
{
    return AppExecFwk::MockBundleMgr::GetInstance();
}

} // namespace OHOS

#endif // BUNDLE_MGR_PROXY_H