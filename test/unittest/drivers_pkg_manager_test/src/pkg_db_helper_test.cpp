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

#include <gtest/gtest.h>
#include <iostream>
#define private public
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "pkg_db_helper.h"
#include "ibus_extension.h"
#include "usb_device_info.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;
using namespace OHOS::ExternalDeviceManager;

class PkgDbHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    void SetUp() override {}
    void TearDown() override {}
};

static std::string g_bundleName = "testBundleName";
static std::string g_Ability = "testAbility";
static size_t g_expect_size = 1;

void PkgDbHelperTest::SetUpTestCase()
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    bool isUpdate = false;
    int32_t ret = helper->CheckIfNeedUpdateEx(isUpdate, g_Ability);
    if (!isUpdate) {
        return;
    }
    ret = helper->DeleteRightRecord(g_bundleName);
    if (ret != PKG_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s delete pkg from db fail", __func__);
    }
}

void PkgDbHelperTest::TearDownTestCase()
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    bool isUpdate = false;
    int32_t ret = helper->CheckIfNeedUpdateEx(isUpdate, g_Ability);
    if (!isUpdate) {
        return;
    }
    ret = helper->DeleteRightRecord(g_bundleName);
    if (ret != PKG_OK) {
        EDM_LOGE(EDM_MODULE_TEST, "%{public}s delete pkg from db fail", __func__);
    }
}

HWTEST_F(PkgDbHelperTest, PkgDb_CheckIfNeedUpdateEx_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    bool isUpdate = false;
    int32_t ret = helper->CheckIfNeedUpdateEx(isUpdate, g_Ability);
    EXPECT_EQ(false, isUpdate);
    EXPECT_EQ(0, ret);
    cout << "PkgDb_CheckIfNeedUpdateEx_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_AddOrUpdateRightRecord_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    string driverInfo = "{}";
    int32_t ret = helper->AddOrUpdateRightRecord(g_bundleName, g_Ability, driverInfo);
    EXPECT_EQ(0, ret);
    cout << "PkgDb_AddOrUpdateRightRecord_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryAllSize_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    std::vector<std::string> allBundleAbilityNames;
    int32_t ret = helper->QueryAllSize(allBundleAbilityNames);
    EXPECT_LE(1, ret);
    EXPECT_LE(g_expect_size, allBundleAbilityNames.size());
    cout << "PkgDb_QueryAllSize_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryAllBundleAbilityNames_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    std::vector<std::string> bundleAbilityNames;
    int32_t ret = helper->QueryAllBundleAbilityNames(g_bundleName, bundleAbilityNames);
    EXPECT_EQ(1, ret);
    EXPECT_EQ(g_expect_size, bundleAbilityNames.size());
    cout << "PkgDb_QueryAllBundleAbilityNames_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryAllDriverInfos_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    std::vector<std::string> driverInfos;
    int32_t ret = helper->QueryAllDriverInfos(driverInfos);
    EXPECT_LE(1, ret);
    EXPECT_LE(g_expect_size, driverInfos.size());
    cout << "PkgDb_QueryAllDriverInfos_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryBundleInfoNames_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    string driverInfo = "{}";
    string bundleName = helper->QueryBundleInfoNames(driverInfo);
    EXPECT_EQ("testAbility", bundleName);
    cout << "PkgDb_QueryBundleInfoNames_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_QueryBundleInfoNames1_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    string driverInfo = "{test}";
    string bundleName = helper->QueryBundleInfoNames(driverInfo);
    EXPECT_EQ("", bundleName);
    cout << "PkgDb_QueryBundleInfoNames1_Test" << endl;
}

HWTEST_F(PkgDbHelperTest, PkgDb_DeleteRightRecord_Test, TestSize.Level1)
{
    std::shared_ptr<PkgDbHelper> helper= PkgDbHelper::GetInstance();
    int32_t ret = helper->DeleteRightRecord(g_bundleName);
    EXPECT_EQ(0, ret);
    cout << "PkgDb_DeleteRightRecord_Test" << endl;
}
}
}