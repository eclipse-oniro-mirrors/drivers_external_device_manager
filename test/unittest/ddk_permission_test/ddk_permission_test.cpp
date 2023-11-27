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
#include "usb_config_desc_parser.h"
#include "usb_ddk_api.h"
#include "usb_ddk_types.h"

using namespace testing::ext;

namespace {
bool g_state = false;
void GrantPermission(void)
{
    g_state = true;
}
void RevokePermission(void)
{
    g_state = false;
}

class DdkPermissionTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
};
} // namespace

namespace OHOS {
namespace ExternalDeviceManager {
class ExtPermissionManager {
    static ExtPermissionManager& GetInstance();
    bool HasPermission(std::string permissionName);
};
ExtPermissionManager& ExtPermissionManager::GetInstance()
{
    static auto instance = new ExtPermissionManager();
    return *instance;
}
// required permissions are not yet implemented, mock this method so that the tests may pass.
bool ExtPermissionManager::HasPermission(std::string permissionName)
{
    return g_state;
}
} // namespace Security
} // namespace OHOS

void DdkPermissionTest::SetUp()
{
    RevokePermission();
    int32_t ret = OH_Usb_Init();
    EXPECT_NE(ret, 0);
    GrantPermission();
    ret = OH_Usb_Init();
    EXPECT_EQ(ret, 0);
}

void DdkPermissionTest::TearDown()
{
    RevokePermission();
    OH_Usb_Release();
    GrantPermission();
    int32_t ret = OH_Usb_GetDeviceDescriptor(0, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
    OH_Usb_Release();
    ret = OH_Usb_GetDeviceDescriptor(0, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_OPERATION);
}

namespace {
HWTEST_F(DdkPermissionTest, OH_Usb_GetDeviceDescriptor_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_GetDeviceDescriptor(0, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetDeviceDescriptor_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_GetDeviceDescriptor(0, nullptr);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetConfigDescriptor_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_GetConfigDescriptor(0, 1, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetConfigDescriptor_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_GetConfigDescriptor(0, 1, nullptr);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_ClaimInterface_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_ClaimInterface(0, 0, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_ClaimInterface_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_ClaimInterface(0, 0, nullptr);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_ReleaseInterface_001, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_ReleaseInterface(1);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SelectInterfaceSetting_001, TestSize.Level1)
{
    OH_Usb_Release();
    int32_t ret = OH_Usb_SelectInterfaceSetting(0, 0);
    EXPECT_EQ(ret, USB_DDK_INVALID_OPERATION);
    OH_Usb_Init();
}

HWTEST_F(DdkPermissionTest, OH_Usb_SelectInterfaceSetting_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_SelectInterfaceSetting(0, 0);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetCurrentInterfaceSetting_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_GetCurrentInterfaceSetting(0, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetCurrentInterfaceSetting_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_GetCurrentInterfaceSetting(0, nullptr);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendControlReadRequest_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_SendControlReadRequest(0, nullptr, 0, nullptr, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendControlReadRequest_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_SendControlReadRequest(0, nullptr, 0, nullptr, nullptr);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendControlWriteRequest_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_SendControlWriteRequest(0, nullptr, 0, nullptr, 0);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendControlWriteRequest_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_SendControlWriteRequest(0, nullptr, 0, nullptr, 0);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendPipeRequest_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_SendPipeRequest(nullptr, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendPipeRequest_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_SendPipeRequest(nullptr, nullptr);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}

HWTEST_F(DdkPermissionTest, OH_Usb_CreateDeviceMemMap_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_CreateDeviceMemMap(0, 100, nullptr);
    EXPECT_EQ(ret, USB_DDK_INVALID_PARAMETER);
}

HWTEST_F(DdkPermissionTest, OH_Usb_CreateDeviceMemMap_002, TestSize.Level1)
{
    RevokePermission();
    int32_t ret = OH_Usb_CreateDeviceMemMap(0, 100, nullptr);
    EXPECT_EQ(ret, USB_DDK_FAILED);
}
} // namespace