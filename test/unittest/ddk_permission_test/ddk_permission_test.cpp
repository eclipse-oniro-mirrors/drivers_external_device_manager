/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSEDDK_ERR_NOPERM.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "usb_config_desc_parser.h"
#include "usb_ddk_api.h"
#include "usb_ddk_types.h"

using namespace testing::ext;

/** No permission */
constexpr int32_t DDK_ERR_NOPERM = -19;

namespace {
class DdkNoPermissionTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void DdkNoPermissionTest::SetUp()
{
}

void DdkNoPermissionTest::TearDown()
{
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_Init_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_Init();
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_GetDeviceDescriptor_001, TestSize.Level1)
{
    struct UsbDeviceDescriptor desc;
    int32_t ret = OH_Usb_GetDeviceDescriptor(0, &desc);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_GetConfigDescriptor_001, TestSize.Level1)
{
    struct UsbDdkConfigDescriptor *config = nullptr;
    int32_t ret = OH_Usb_GetConfigDescriptor(0, 1, &config);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_ClaimInterface_001, TestSize.Level1)
{
    uint64_t g_interfaceHandle = 0;
    int32_t ret = OH_Usb_ClaimInterface(0, 0, &g_interfaceHandle);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_ReleaseInterface_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_ReleaseInterface(0);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_SelectInterfaceSetting_001, TestSize.Level1)
{
    int32_t ret = OH_Usb_SelectInterfaceSetting(0, 0);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_GetCurrentInterfaceSetting_001, TestSize.Level1)
{
    uint8_t settingIndex = 0;
    int32_t ret = OH_Usb_GetCurrentInterfaceSetting(0, &settingIndex);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_SendControlReadRequest_001, TestSize.Level1)
{
    struct UsbControlRequestSetup setup;
    uint8_t strDesc[2] = {0};
    uint32_t len = 100;
    int32_t ret = OH_Usb_SendControlReadRequest(0, &setup, 0, strDesc, &len);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_SendControlWriteRequest_001, TestSize.Level1)
{
    struct UsbControlRequestSetup strDescSetup;
    uint8_t data[2] = {0x02, 0x02};
    int32_t ret = OH_Usb_SendControlWriteRequest(0, &strDescSetup, 0, data, 2);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_SendPipeRequest_001, TestSize.Level1)
{
    struct UsbRequestPipe pipe;
    uint8_t address = 0;
    struct UsbDeviceMemMap devMmap = {.address = &address};
    int32_t ret = OH_Usb_SendPipeRequest(&pipe, &devMmap);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkNoPermissionTest, OH_Usb_CreateDeviceMemMap_001, TestSize.Level1)
{
    UsbDeviceMemMap *devMmap = nullptr;
    int32_t ret = OH_Usb_CreateDeviceMemMap(0, 100, &devMmap);
    EXPECT_EQ(ret, DDK_ERR_NOPERM);
}
} // namespace

namespace {
class DdkPermissionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.ACCESS_DDK_USB";

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "TestCase";
    tokenId = GetAccessTokenId(&infoInstance);
    EXPECT_EQ(0, SetSelfTokenID(tokenId));
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

void DdkPermissionTest::SetUpTestCase(void)
{
    GetNativeToken();
}

void DdkPermissionTest::TearDownTestCase(void)
{
    OH_Usb_Release();
}

void DdkPermissionTest::SetUp(void)
{
}

void DdkPermissionTest::TearDown(void)
{
}

HWTEST_F(DdkPermissionTest, OH_Usb_Init_002, TestSize.Level1)
{
    int32_t ret = OH_Usb_Init();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetDeviceDescriptor_002, TestSize.Level1)
{
    struct UsbDeviceDescriptor desc;
    int32_t ret = OH_Usb_GetDeviceDescriptor(0, &desc);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetConfigDescriptor_002, TestSize.Level1)
{
    struct UsbDdkConfigDescriptor *config = nullptr;
    int32_t ret = OH_Usb_GetConfigDescriptor(0, 1, &config);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_ClaimInterface_002, TestSize.Level1)
{
    uint64_t g_interfaceHandle = 0;
    int32_t ret = OH_Usb_ClaimInterface(0, 0, &g_interfaceHandle);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SelectInterfaceSetting_002, TestSize.Level1)
{
    int32_t ret = OH_Usb_SelectInterfaceSetting(0, 0);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_GetCurrentInterfaceSetting_002, TestSize.Level1)
{
    uint8_t settingIndex = 0;
    int32_t ret = OH_Usb_GetCurrentInterfaceSetting(0, &settingIndex);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendControlReadRequest_002, TestSize.Level1)
{
    struct UsbControlRequestSetup setup;
    uint8_t strDesc[2] = {0};
    uint32_t len = 100;
    int32_t ret = OH_Usb_SendControlReadRequest(0, &setup, 0, strDesc, &len);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendControlWriteRequest_002, TestSize.Level1)
{
    struct UsbControlRequestSetup strDescSetup;
    uint8_t data[2] = {0x02, 0x02};
    int32_t ret = OH_Usb_SendControlWriteRequest(0, &strDescSetup, 0, data, 2);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_SendPipeRequest_002, TestSize.Level1)
{
    struct UsbRequestPipe pipe;
    uint8_t address = 0;
    struct UsbDeviceMemMap devMmap = {.address = &address};
    int32_t ret = OH_Usb_SendPipeRequest(&pipe, &devMmap);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}

HWTEST_F(DdkPermissionTest, OH_Usb_CreateDeviceMemMap_002, TestSize.Level1)
{
    UsbDeviceMemMap *devMmap = nullptr;
    int32_t ret = OH_Usb_CreateDeviceMemMap(0, 100, &devMmap);
    EXPECT_NE(ret, DDK_ERR_NOPERM);
}
} // namespace