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

#include "ddk_api.h"
#include "ddk_types.h"
#include "gtest/gtest.h"

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {

constexpr int32_t EMULATOR_NOT_SUPPORT_CODE = 804;

class DdkBaseEmulatorTest : public testing::Test {};

HWTEST_F(DdkBaseEmulatorTest, OH_DDK_CreateAshmem_Emulator_001, TestSize.Level1)
{
    DDK_Ashmem *ashmem = nullptr;
    const uint8_t name[100] = "TestAshmem";
    auto ret = OH_DDK_CreateAshmem(name, 10, &ashmem);
    EXPECT_EQ(ret, static_cast<DDK_RetCode>(EMULATOR_NOT_SUPPORT_CODE));
}

HWTEST_F(DdkBaseEmulatorTest, OH_DDK_MapAshmem_Emulator_001, TestSize.Level1)
{
    const uint8_t ashmemMapType = 0x01;
    auto ret = OH_DDK_MapAshmem(nullptr, ashmemMapType);
    EXPECT_EQ(ret, static_cast<DDK_RetCode>(EMULATOR_NOT_SUPPORT_CODE));
}

HWTEST_F(DdkBaseEmulatorTest, OH_DDK_UnmapAshmem_Emulator_001, TestSize.Level1)
{
    auto ret = OH_DDK_UnmapAshmem(nullptr);
    EXPECT_EQ(ret, static_cast<DDK_RetCode>(EMULATOR_NOT_SUPPORT_CODE));
}

HWTEST_F(DdkBaseEmulatorTest, OH_DDK_DestroyAshmem_Emulator_001, TestSize.Level1)
{
    auto ret = OH_DDK_DestroyAshmem(nullptr);
    EXPECT_EQ(ret, static_cast<DDK_RetCode>(EMULATOR_NOT_SUPPORT_CODE));
}
} // ExternalDeviceManager
} // OHOS