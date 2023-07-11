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
#include <vector>

#include "driver_ext_mgr_client.h"
#include "hilog_wrapper.h"


namespace OHOS {
namespace ExternalDeviceManager {
using namespace testing::ext;
class EmitEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override {};
    void TearDown() override {};

    static DriverExtMgrClient &edmClient;
};

DriverExtMgrClient &EmitEventTest::edmClient = DriverExtMgrClient::GetInstance();

void EmitEventTest::SetUpTestCase()
{
    auto ret = edmClient.CreateDevice(100, 100, 100);
    ASSERT_GE(ret, 0);
}

void EmitEventTest::TearDownTestCase()
{
    int32_t ret = edmClient.DestroyDevice();
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent001, TestSize.Level1)
{
    std::vector<EmitItem> items({
        {0, 1, 0x14a, 108},
        {0, 3, 0,     50 },
        {0, 3, 1,     50 },
        {0, 0, 0,     0  }
    });
    auto ret = edmClient.EmitEvent(0, items);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent002, TestSize.Level1)
{
    std::vector<EmitItem> items(21, {0, 1, 0x14a, 108});
    auto ret = edmClient.EmitEvent(0, items);
    ASSERT_NE(ret, 0);
}

HWTEST_F(EmitEventTest, EmitEvent003, TestSize.Level1)
{
    std::vector<EmitItem> items(20, {0, 1, 0x14a, 108});
    auto ret = edmClient.EmitEvent(0, items);
    ASSERT_EQ(ret, 0);
}
} // namespace ExternalDeviceManager
} // namespace OHOS