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

#include "app_launch_config.h"

#include <fstream>
#include <gtest/gtest.h>
#include <string>

#include "edm_errors.h"
#include "hilog_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {

static const std::string TEST_CONFIG_FILE = "./app_launch_config.json";

static const std::string VALID_JSON = R"(
[
    {
        "vid": "12D1",
        "pid": "4321",
        "bundleName": "com.test.usb.launcher",
        "abilityName": "MainAbility",
        "appStoreUri": "market://detail?id=com.test.usb.launcher"
    },
    {
        "vid": "04E8",
        "pid": "6860",
        "bundleName": "com.test.usb.viewer",
        "abilityName": "EntryAbility",
        "appStoreUri": "market://detail?id=com.test.usb.viewer"
    }
]
)";

// Truncated JSON: just an opening brace
static const std::string TRUNCATED_JSON = "{";

// Not an array
static const std::string NOT_ARRAY_JSON = R"({"vid": "12D1", "pid": "4321"})";

// Empty array
static const std::string EMPTY_ARRAY_JSON = "[]";

class AppLaunchConfigTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static void WriteFile(const std::string &filePath, const std::string &content);
    static void DeleteFile(const std::string &filePath);
};

void AppLaunchConfigTest::WriteFile(const std::string &filePath, const std::string &content)
{
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}

void AppLaunchConfigTest::DeleteFile(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) == 0) {
        if (remove(filePath.c_str()) != 0) {
            EDM_LOGE(MODULE_BUS_USB, "Failed to remove file: %{public}s", filePath.c_str());
        }
    }
}

void AppLaunchConfigTest::SetUpTestCase(void) {}

void AppLaunchConfigTest::TearDownTestCase(void)
{
    DeleteFile(TEST_CONFIG_FILE);
}

void AppLaunchConfigTest::SetUp(void) {}
void AppLaunchConfigTest::TearDown(void) {}

/**
 * @tc.name: ParseConfigFileTo001
 * @tc.desc: Test ParseConfigFileTo with valid JSON
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, ParseConfigFileTo001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo001 begin");
    WriteFile(TEST_CONFIG_FILE, VALID_JSON);
    AppLaunchConfig config;
    std::unordered_map<std::string, AppLaunchEntry> outMap;
    bool ret = config.ParseConfigFileTo(TEST_CONFIG_FILE, outMap);
    EXPECT_TRUE(ret);
    EXPECT_EQ(outMap.size(), 2u);

    std::string key1 = "4817_17185"; // 0x12D1=4817, 0x4321=17185
    ASSERT_TRUE(outMap.find(key1) != outMap.end());
    EXPECT_EQ(outMap[key1].vid, 0x12D1);
    EXPECT_EQ(outMap[key1].pid, 0x4321);
    EXPECT_EQ(outMap[key1].bundleName, "com.test.usb.launcher");
    EXPECT_EQ(outMap[key1].abilityName, "MainAbility");
    EXPECT_EQ(outMap[key1].appStoreUri, "market://detail?id=com.test.usb.launcher");

    std::string key2 = "1256_26720"; // 0x04E8=1256, 0x6860=26720
    ASSERT_TRUE(outMap.find(key2) != outMap.end());
    EXPECT_EQ(outMap[key2].vid, 0x04E8);
    EXPECT_EQ(outMap[key2].pid, 0x6860);
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo001 end");
}

/**
 * @tc.name: ParseConfigFileTo002
 * @tc.desc: Test ParseConfigFileTo with truncated JSON
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, ParseConfigFileTo002, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo002 begin");
    WriteFile(TEST_CONFIG_FILE, TRUNCATED_JSON);
    AppLaunchConfig config;
    std::unordered_map<std::string, AppLaunchEntry> outMap;
    bool ret = config.ParseConfigFileTo(TEST_CONFIG_FILE, outMap);
    EXPECT_FALSE(ret);
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo002 end");
}

/**
 * @tc.name: ParseConfigFileTo003
 * @tc.desc: Test ParseConfigFileTo with non-array JSON
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, ParseConfigFileTo003, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo003 begin");
    WriteFile(TEST_CONFIG_FILE, NOT_ARRAY_JSON);
    AppLaunchConfig config;
    std::unordered_map<std::string, AppLaunchEntry> outMap;
    bool ret = config.ParseConfigFileTo(TEST_CONFIG_FILE, outMap);
    EXPECT_FALSE(ret);
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo003 end");
}

/**
 * @tc.name: ParseConfigFileTo004
 * @tc.desc: Test ParseConfigFileTo with empty array
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, ParseConfigFileTo004, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo004 begin");
    WriteFile(TEST_CONFIG_FILE, EMPTY_ARRAY_JSON);
    AppLaunchConfig config;
    std::unordered_map<std::string, AppLaunchEntry> outMap;
    bool ret = config.ParseConfigFileTo(TEST_CONFIG_FILE, outMap);
    EXPECT_TRUE(ret);
    EXPECT_EQ(outMap.size(), 0u);
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo004 end");
}

/**
 * @tc.name: ParseConfigFileTo005
 * @tc.desc: Test ParseConfigFileTo with non-existent file
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, ParseConfigFileTo005, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo005 begin");
    AppLaunchConfig config;
    std::unordered_map<std::string, AppLaunchEntry> outMap;
    bool ret = config.ParseConfigFileTo("./non_existent.json", outMap);
    EXPECT_FALSE(ret);
    EDM_LOGI(MODULE_BUS_USB, "ParseConfigFileTo005 end");
}

/**
 * @tc.name: FindEntry001
 * @tc.desc: Test FindEntry with existing entry
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, FindEntry001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "FindEntry001 begin");
    WriteFile(TEST_CONFIG_FILE, VALID_JSON);
    AppLaunchConfig config;
    std::unordered_map<std::string, AppLaunchEntry> outMap;
    bool ret = config.ParseConfigFileTo(TEST_CONFIG_FILE, outMap);
    ASSERT_TRUE(ret);
    config.entryMap_ = std::move(outMap);

    auto entry = config.FindEntry(0x12D1, 0x4321);
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->vid, 0x12D1);
    EXPECT_EQ(entry->pid, 0x4321);
    EXPECT_EQ(entry->bundleName, "com.test.usb.launcher");
    EXPECT_EQ(entry->abilityName, "MainAbility");
    EXPECT_EQ(entry->appStoreUri, "market://detail?id=com.test.usb.launcher");
    EDM_LOGI(MODULE_BUS_USB, "FindEntry001 end");
}

/**
 * @tc.name: FindEntry002
 * @tc.desc: Test FindEntry with non-existing entry
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, FindEntry002, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "FindEntry002 begin");
    WriteFile(TEST_CONFIG_FILE, VALID_JSON);
    AppLaunchConfig config;
    std::unordered_map<std::string, AppLaunchEntry> outMap;
    bool ret = config.ParseConfigFileTo(TEST_CONFIG_FILE, outMap);
    ASSERT_TRUE(ret);
    config.entryMap_ = std::move(outMap);

    auto entry = config.FindEntry(0xFFFF, 0xFFFF);
    EXPECT_FALSE(entry.has_value());
    EDM_LOGI(MODULE_BUS_USB, "FindEntry002 end");
}

/**
 * @tc.name: MakeKey001
 * @tc.desc: Test MakeKey generates correct key
 * @tc.type: FUNC
 */
HWTEST_F(AppLaunchConfigTest, MakeKey001, TestSize.Level1)
{
    EDM_LOGI(MODULE_BUS_USB, "MakeKey001 begin");
    AppLaunchConfig config;
    std::string key = config.MakeKey(0x12D1, 0x4321);
    EXPECT_EQ(key, "4817_17185");
    EDM_LOGI(MODULE_BUS_USB, "MakeKey001 end");
}

} // namespace ExternalDeviceManager
} // namespace OHOS