/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "edm_errors.h"
#include "event_config.h"
#include "hilog_wrapper.h"
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include "file_ex.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {
static std::string g_localJsonStr = R"(
[
    {
        "domain": "USB",
        "fault": [
            {
                "faultName": "TRANSFOR_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            }
        ]
    }
]
)";

static std::string g_ccmJsonStr = R"(
[
    {
        "domain": "USB",
        "fault": [
            {
                "faultName": "TRANSFOR_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            },
            {
                "faultName": "READ_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            }
        ]
    },
    {
        "domain": "MOUSE",
        "fault": [
            {
                "faultName": "TRANSFOR_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            }
        ]
    }
]
)";

const std::string LOCAL_J_SON_FILE_PATH = "./local_event_config.json";
const std::string CCM_J_SON_FILE_PATH = "./ccm_event_config.json";

class EventConfigTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void DeleteFile(const std::string &filePath);
};

void EventConfigTest::DeleteFile(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) == 0) {
        if (remove(filePath.c_str()) != 0) {
            EDM_LOGE(MODULE_SERVICE, "Failed to remove file: %{public}s", filePath.c_str());
        }
    }
}

void EventConfigTest::SetUpTestCase(void)
{
    SaveStringToFile(LOCAL_J_SON_FILE_PATH, g_localJsonStr);
    SaveStringToFile(CCM_J_SON_FILE_PATH, g_ccmJsonStr);
}

void EventConfigTest::TearDownTestCase(void)
{
    DeleteFile(LOCAL_J_SON_FILE_PATH);
    DeleteFile(CCM_J_SON_FILE_PATH);
}
void EventConfigTest::SetUp(void) {}
void EventConfigTest::TearDown(void) {}

/**
 * @tc.name: ParseJsonFile001
 * @tc.desc: Test ParseJsonFile
 * @tc.type: FUNC
 */
HWTEST_F(EventConfigTest, ParseJsonFile001, TestSize.Level1)
{
    EDM_LOGI(MODULE_SERVICE, "ParseJsonFile001 begin");
    auto &eventConfig = EventConfig::GetInstance();
    EXPECT_NE(&eventConfig, nullptr);
    eventConfig.ParseJsonFile();
    size_t size = eventConfig.peripheralFaultsMap_.size();
    ASSERT_GT(size, 0);
    if (size > 0) {
        std::vector<FaultInfo> faults = eventConfig.GetFaultsInfoByDomain("USB");
        EXPECT_GT(faults.size(), 0);
        FaultInfo faultInfo = eventConfig.GetFaultInfo("USB", "TRANSFOR_FAULT");
        EXPECT_EQ(faultInfo.faultName, "TRANSFOR_FAULT");
        EXPECT_EQ(faultInfo.type, "FAULT");
        EXPECT_EQ(faultInfo.title, "usb_transmission_error_title");
        EXPECT_EQ(faultInfo.msg, "usb_troubleshoot_message");
        EDM_LOGI(MODULE_SERVICE, "ParseJsonFile001 end");
    }
}

/**
 * @tc.name: ParseJsonFile002
 * @tc.desc: Test ParseJsonFile
 * @tc.type: FUNC
 */
HWTEST_F(EventConfigTest, ParseJsonFile002, TestSize.Level1)
{
    EDM_LOGI(MODULE_SERVICE, "ParseJsonFile002 begin");
    EventConfig &eventConfig = EventConfig::GetInstance();
    EXPECT_NE(&eventConfig, nullptr);
    DomainFaultsMap peripheralFaultsMap;
    bool bRet = eventConfig.ParseJsonFile(LOCAL_J_SON_FILE_PATH, peripheralFaultsMap);
    EXPECT_TRUE(bRet);
    EXPECT_GT(peripheralFaultsMap.size(), 0);
    EDM_LOGI(MODULE_SERVICE, "ParseJsonFile002 end");
}

/**
 * @tc.name: FillFaultsMap001
 * @tc.desc: Test FillFaultsMap
 * @tc.type: FUNC
 */
HWTEST_F(EventConfigTest, FillFaultsMap001, TestSize.Level1)
{
    EDM_LOGI(MODULE_SERVICE, "FillFaultsMap001 begin");
    EventConfig &eventConfig = EventConfig::GetInstance();
    EXPECT_NE(&eventConfig, nullptr);
    DomainFaultsMap ccmMap;
    DomainFaultsMap localMap;
    DomainFaultsMap comMap;
    bool bRet = eventConfig.ParseJsonFile(LOCAL_J_SON_FILE_PATH, ccmMap);
    EXPECT_TRUE(bRet);
    EXPECT_EQ(ccmMap.size(), 1);
    bRet = eventConfig.ParseJsonFile(CCM_J_SON_FILE_PATH, localMap);
    EXPECT_TRUE(bRet);
    EXPECT_EQ(localMap.size(), 2);
    comMap = eventConfig.FillFaultsMap(ccmMap, localMap);
    EXPECT_EQ(comMap.size(), 1);
    EDM_LOGI(MODULE_SERVICE, "FillFaultsMap001 end");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
