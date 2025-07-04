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
#include "notification_locale.h"
#include "notification_peripheral.h"
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {
class DeviceNotificationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DeviceNotificationTest::SetUpTestCase(void) {}
void DeviceNotificationTest::TearDownTestCase(void) {}
void DeviceNotificationTest::SetUp(void) {}
void DeviceNotificationTest::TearDown(void) {}

/**
 * @tc.name: HandleNotification001
 * @tc.desc: Test HandleNotification
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNotificationTest, HandleNotification001, TestSize.Level1)
{
    DeviceNotification &perNotification = DeviceNotification::GetInstance();
    FaultInfo faultInfo;
    EXPECT_FALSE(perNotification.HandleNotification(faultInfo));
}

/**
 * @tc.name: PeripheralDeviceNotification001
 * @tc.desc: Test PeripheralDeviceNotification
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNotificationTest, PeripheralDeviceNotification001, TestSize.Level1)
{
    DeviceNotification &perNotification = DeviceNotification::GetInstance();
    FaultInfo faultInfo;
    EXPECT_FALSE(perNotification.PeripheralDeviceNotification(faultInfo));
}

/**
 * @tc.name: GetPixelMap001
 * @tc.desc: Test GetPixelMap
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNotificationTest, GetPixelMap001, TestSize.Level1)
{
    DeviceNotification &perNotification = DeviceNotification::GetInstance();
    std::string path = "";
    perNotification.GetPixelMap(path);
    EXPECT_FALSE(std::filesystem::exists(path)) << "Test file does not exist: " << path;
}

/**
 * @tc.name: GetPixelMap002
 * @tc.desc: Test GetPixelMap
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNotificationTest, GetPixelMap002, TestSize.Level1)
{
    DeviceNotification &perNotification = DeviceNotification::GetInstance();
    perNotification.iconPixelMap_ = std::make_unique<Media::PixelMap>();
    std::string path = "/system/etc/peripheral/resources/peripheral_fault_icon.png";
    perNotification.GetPixelMap(path);
    EXPECT_TRUE(std::filesystem::exists(path)) << "Test file exist: " << path;
}

/**
 * @tc.name: FillNotificationCfg001
 * @tc.desc: Test FillNotificationCfg
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNotificationTest, FillNotificationCfg001, TestSize.Level1)
{
    DeviceNotification &perNotification = DeviceNotification::GetInstance();
    FaultInfo faultInfo;
    faultInfo.title = "";
    faultInfo.msg = "";
    faultInfo.uri = "";
    FaultInfo result = perNotification.FillNotificationCfg(faultInfo);
    EXPECT_TRUE(result.title.empty());
    EXPECT_TRUE(result.msg.empty());
    EXPECT_TRUE(result.uri.empty());
}

/**
 * @tc.name: FillNotificationCfg002
 * @tc.desc: Test FillNotificationCfg
 * @tc.type: FUNC
 */
HWTEST_F(DeviceNotificationTest, FillNotificationCfg002, TestSize.Level1)
{
    DeviceNotification &perNotification = DeviceNotification::GetInstance();
    FaultInfo faultInfo;
    faultInfo.title = "usb_transmission_error_title";
    faultInfo.msg = "usb_troubleshoot_message";
    faultInfo.uri = "www.gitee.com";
    NotificationLocale &perLocale = NotificationLocale::GetInstance();
    perLocale.stringMap_["usb_transmission_error_title"] = faultInfo.title;
    perLocale.stringMap_["usb_troubleshoot_message"] = faultInfo.msg;
    perLocale.stringMap_["www.gitee.com"] = faultInfo.uri;
    perLocale.stringMap_["usb_transmission_error_title"] = faultInfo.title;

    FaultInfo result = perNotification.FillNotificationCfg(faultInfo);
    EXPECT_EQ(result.title, "usb_transmission_error_title");
    EXPECT_EQ(result.msg, "usb_troubleshoot_message");
    EXPECT_EQ(result.uri, "www.gitee.com");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
