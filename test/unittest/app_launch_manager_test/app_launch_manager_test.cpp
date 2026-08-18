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

#include "app_launch_launcher.h"
#include "app_launch_manager.h"
#include "app_launch_notifier.h"

#include <fstream>
#include <gtest/gtest.h>
#include <string>

#include "edm_errors.h"
#include "hilog_wrapper.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace ExternalDeviceManager {

static const std::string TEST_CONFIG_FILE = "./app_launch_mgr_config.json";

static const std::string VALID_JSON = R"(
[
    {
        "vid": "12D1",
        "pid": "4321",
        "bundleName": "com.test.usb.launcher",
        "abilityName": "MainAbility",
        "appStoreUri": "market://detail?id=com.test.usb.launcher"
    }
]
)";

class AppLaunchManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppLaunchManagerTest::SetUpTestCase(void)
{
    std::ofstream file(TEST_CONFIG_FILE);
    if (file.is_open()) {
        file << VALID_JSON;
        file.close();
    }
}

void AppLaunchManagerTest::TearDownTestCase(void)
{
    if (access(TEST_CONFIG_FILE.c_str(), F_OK) == 0) {
        if (remove(TEST_CONFIG_FILE.c_str()) != 0) {
            EDM_LOGE(MODULE_BUS_USB, "Failed to remove file: %{public}s", TEST_CONFIG_FILE.c_str());
        }
    }
}

void AppLaunchManagerTest::SetUp(void)
{
    auto &mgr = AppLaunchManager::GetInstance();
    {
        std::lock_guard<std::mutex> lock(mgr.config_.configMutex_);
        mgr.config_.entryMap_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(mgr.queueMutex_);
        mgr.pendingEntries_.clear();
    }
    mgr.bundleMgr_ = nullptr;
    mgr.running_ = false;
}

void AppLaunchManagerTest::TearDown(void) {}

/**
 * @tc.name: OnDeviceConnected001
 * @tc.desc: Test OnDeviceConnected001 with no matching config entry
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchManagerTest, OnDeviceConnected001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "OnDeviceConnected001 begin");
    auto &mgr = AppLaunchManager::GetInstance();
    mgr.OnDeviceConnected(0xFFFF, 0xFFFF);

    {
        std::lock_guard<std::mutex> lock(mgr.queueMutex_);
        EXPECT_EQ(mgr.pendingEntries_.size(), 0u);
    }
    EDM_LOGI(MODULE_BUS_USB, "OnDeviceConnected001 end");
}

/**
 * @tc.name: OnDeviceConnected002
 * @tc.desc: Test OnDeviceConnected002 with matching config entry should enqueue
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchManagerTest, OnDeviceConnected002, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "OnDeviceConnected002 begin");
    auto &mgr = AppLaunchManager::GetInstance();
    {
        std::lock_guard<std::mutex> lock(mgr.config_.configMutex_);
        AppLaunchEntry entry;
        entry.vid = 0x12D1;
        entry.pid = 0x4321;
        entry.bundleName = "com.test.usb.launcher";
        entry.abilityName = "MainAbility";
        entry.appStoreUri = "market://detail?id=com.test.usb.launcher";
        mgr.config_.entryMap_[mgr.config_.MakeKey(0x12D1, 0x4321)] = entry;
    }

    mgr.OnDeviceConnected(0x12D1, 0x4321);

    {
        std::lock_guard<std::mutex> lock(mgr.queueMutex_);
        EXPECT_EQ(mgr.pendingEntries_.size(), 1u);
        EXPECT_EQ(mgr.pendingEntries_[0].vid, 0x12D1);
        EXPECT_EQ(mgr.pendingEntries_[0].pid, 0x4321);
        EXPECT_EQ(mgr.pendingEntries_[0].bundleName, "com.test.usb.launcher");
    }
    EDM_LOGI(MODULE_BUS_USB, "OnDeviceConnected002 end");
}

/**
 * @tc.name: OnDeviceConnected003
 * @tc.desc: Test OnDeviceConnected003 with same bundleName should not duplicate
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchManagerTest, OnDeviceConnected003, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "OnDeviceConnected003 begin");
    auto &mgr = AppLaunchManager::GetInstance();
    {
        std::lock_guard<std::mutex> lock(mgr.config_.configMutex_);
        AppLaunchEntry entry1;
        entry1.vid = 0x12D1;
        entry1.pid = 0x4321;
        entry1.bundleName = "com.test.duplicate";
        entry1.abilityName = "MainAbility";
        entry1.appStoreUri = "market://detail?id=test";
        mgr.config_.entryMap_[mgr.config_.MakeKey(0x12D1, 0x4321)] = entry1;

        AppLaunchEntry entry2;
        entry2.vid = 0x04E8;
        entry2.pid = 0x6860;
        entry2.bundleName = "com.test.duplicate";
        entry2.abilityName = "MainAbility";
        entry2.appStoreUri = "market://detail?id=test";
        mgr.config_.entryMap_[mgr.config_.MakeKey(0x04E8, 0x6860)] = entry2;
    }

    mgr.OnDeviceConnected(0x12D1, 0x4321);
    mgr.OnDeviceConnected(0x04E8, 0x6860);

    {
        std::lock_guard<std::mutex> lock(mgr.queueMutex_);
        EXPECT_EQ(mgr.pendingEntries_.size(), 1u);
    }
    EDM_LOGI(MODULE_BUS_USB, "OnDeviceConnected003 end");
}

/**
 * @tc.name: UnInit001
 * @tc.desc: Test UnInit001 does not crash when worker not started
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchManagerTest, UnInit001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "UnInit001 begin");
    auto &mgr = AppLaunchManager::GetInstance();
    mgr.UnInit();
    EXPECT_FALSE(mgr.running_);
    EDM_LOGI(MODULE_BUS_USB, "UnInit001 end");
}

/**
 * @tc.name: IsAppInstalled001
 * @tc.desc: Test IsAppInstalled001 with mock BundleMgr returning true
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchManagerTest, IsAppInstalled001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "IsAppInstalled001 begin");
    auto &mgr = AppLaunchManager::GetInstance();
    auto mockBundleMgr = AppExecFwk::MockBundleMgr::GetInstance();
    EXPECT_CALL(*mockBundleMgr, GetBundleInfo(_, _, _, _))
        .WillOnce(Return(true));
    mgr.bundleMgr_ = mockBundleMgr;

    bool installed = mgr.IsAppInstalled("com.test.installed");
    EXPECT_TRUE(installed);
    EDM_LOGI(MODULE_BUS_USB, "IsAppInstalled001 end");
}

/**
 * @tc.name: IsAppInstalled002
 * @tc.desc: Test IsAppInstalled002 with mock BundleMgr returning false
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchManagerTest, IsAppInstalled002, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "IsAppInstalled002 begin");
    auto &mgr = AppLaunchManager::GetInstance();
    auto mockBundleMgr = AppExecFwk::MockBundleMgr::GetInstance();
    EXPECT_CALL(*mockBundleMgr, GetBundleInfo(_, _, _, _))
        .WillOnce(Return(false));
    mgr.bundleMgr_ = mockBundleMgr;

    bool installed = mgr.IsAppInstalled("com.test.notinstalled");
    EXPECT_FALSE(installed);
    EDM_LOGI(MODULE_BUS_USB, "IsAppInstalled002 end");
}

class AppLaunchLauncherTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: LaunchApp001
 * @tc.desc: Test LaunchApp001 with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchLauncherTest, LaunchApp001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "LaunchApp001 begin");
    AppLaunchLauncher launcher;
    int32_t ret = launcher.LaunchApp("", "MainAbility");
    EXPECT_EQ(ret, EDM_ERR_INVALID_PARAM);
    EDM_LOGI(MODULE_BUS_USB, "LaunchApp001 end");
}

/**
 * @tc.name: LaunchApp002
 * @tc.desc: Test LaunchApp002 with empty abilityName
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchLauncherTest, LaunchApp002, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "LaunchApp002 begin");
    AppLaunchLauncher launcher;
    int32_t ret = launcher.LaunchApp("com.test.bundle", "");
    EXPECT_EQ(ret, EDM_ERR_INVALID_PARAM);
    EDM_LOGI(MODULE_BUS_USB, "LaunchApp002 end");
}

/**
 * @tc.name: LaunchApp003
 * @tc.desc: Test LaunchApp003 with both empty
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchLauncherTest, LaunchApp003, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "LaunchApp003 begin");
    AppLaunchLauncher launcher;
    int32_t ret = launcher.LaunchApp("", "");
    EXPECT_EQ(ret, EDM_ERR_INVALID_PARAM);
    EDM_LOGI(MODULE_BUS_USB, "LaunchApp003 end");
}

class AppLaunchNotifierTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SendNotification001
 * @tc.desc: Test SendNotification001 with vaild entry (expect failure in test env)
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchNotifierTest, SendNotification001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "SendNotification001 begin");
    AppLaunchNotifier notifier;
    AppLaunchEntry entry;
    entry.vid = 0x12D1;
    entry.pid = 0x4321;
    entry.bundleName = "com.test.usb.launcher";
    entry.abilityName = "MainAbility";
    entry.appStoreUri = "market://detail?id=com.test.usb.launcher";
    int32_t ret = notifier.SendNotification(entry);
    EXPECT_NE(ret, EDM_OK);
    EDM_LOGI(MODULE_BUS_USB, "SendNotification001 end");
}

/**
 * @tc.name: GetPixelMap001
 * @tc.desc: Test GetPixelMap001 with non-existent icon path
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchNotifierTest, GetPixelMap001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "GetPixelMap001 begin");
    AppLaunchNotifier notifier;
    std::string path = "";
    bool ret = notifier.GetPixelMap(path);
    EXPECT_FALSE(ret);
    EXPECT_EQ(notifier.iconPixelMap_, nullptr);
    EXPECT_FALSE(std::filesystem::exists(path)) << "Test file does not exist: " + path;
    EDM_LOGI(MODULE_BUS_USB, "GetPixelMap001 end");
}

/**
 * @tc.name: GetPixelMap002
 * @tc.desc: Test GetPixelMap002 with system icon path
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchNotifierTest, GetPixelMap002, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "GetPixelMap002 begin");
    AppLaunchNotifier notifier;
    std::string path = "/system/etc/peripheral/resources/peripheral_fault_icon.png";
    bool ret = notifier.GetPixelMap(path);
    if (std::filesystem::exists(path)) {
        EXPECT_TRUE(ret);
        EXPECT_NE(notifier.iconPixelMap_, nullptr);
    } else {
        EXPECT_FALSE(ret);
        EXPECT_EQ(notifier.iconPixelMap_, nullptr);
    }
    EDM_LOGI(MODULE_BUS_USB, "GetPixelMap002 end");
}

} // namespace ExternalDeviceManager
} // namespace OHOS