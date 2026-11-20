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

#include <string>

#include <gtest/gtest.h>

#include "recovery_mode_checker.h"

using namespace OHOS::ExternalDeviceManager;
using namespace testing::ext;

namespace {
const char *g_validBopdValues[] = {BOPD_BEFORE_DL, BOPD_STG_RO_BEFORE_DL, BOPD_HW_DEG_BEFORE_DL,
    BOPD_STG_RO_HW_DEG_BEFORE_DL, BOPD_AFTER_DL, BOPD_HW_DEG_AFTER_DL, BOPD_HW_DEG_VIA_ERECOVERY};
const char *g_invalidBopdValues[] = {
    "",
    "NA",
    "2",
    "10",
    "0X2",
    " 0x2",
    "0x2 ",
    "+0x2",
    "02",
    "0x02",
    "0x0",
    "0x1",
    "0x4",
    "0x5",
    "0xb",
    "0x10",
    "0x12",
    "2abc",
    "0x2abc",
    "0x2x",
    "-1",
    "invalid",
};
} // namespace

HWTEST(RecoveryModeCheckerTest, ValidBopdValues, TestSize.Level1)
{
    for (const auto value : g_validBopdValues) {
        EXPECT_TRUE(RecoveryModeChecker::IsBopdRunMode(value)) << "value=" << value;
    }
}

HWTEST(RecoveryModeCheckerTest, BopdModeConstantsMatchSpec, TestSize.Level1)
{
    EXPECT_STREQ(BOPD_BEFORE_DL, "0x2");
    EXPECT_STREQ(BOPD_STG_RO_BEFORE_DL, "0x3");
    EXPECT_STREQ(BOPD_HW_DEG_BEFORE_DL, "0x6");
    EXPECT_STREQ(BOPD_STG_RO_HW_DEG_BEFORE_DL, "0x7");
    EXPECT_STREQ(BOPD_AFTER_DL, "0xa");
    EXPECT_STREQ(BOPD_HW_DEG_AFTER_DL, "0xe");
    EXPECT_STREQ(BOPD_HW_DEG_VIA_ERECOVERY, "0xf");
}

HWTEST(RecoveryModeCheckerTest, InvalidBopdValues, TestSize.Level1)
{
    for (const auto value : g_invalidBopdValues) {
        EXPECT_FALSE(RecoveryModeChecker::IsBopdRunMode(value)) << "value=" << value;
    }
}

HWTEST(RecoveryModeCheckerTest, ReaderBopdValue, TestSize.Level1)
{
    EXPECT_TRUE(RecoveryModeChecker::IsRecoveryModeWithReader([]() {
        return std::string("0x2");
    }));
}

HWTEST(RecoveryModeCheckerTest, ReaderInvalidValue, TestSize.Level1)
{
    EXPECT_FALSE(RecoveryModeChecker::IsRecoveryModeWithReader([]() {
        return std::string("0xb");
    }));
    EXPECT_FALSE(RecoveryModeChecker::IsRecoveryModeWithReader([]() {
        return std::string("0x2abc");
    }));
}

HWTEST(RecoveryModeCheckerTest, ReaderDefaultValue, TestSize.Level1)
{
    EXPECT_FALSE(RecoveryModeChecker::IsRecoveryModeWithReader(nullptr));
}

HWTEST(RecoveryModeCheckerTest, TestReaderAffectsIsRecoveryMode, TestSize.Level1)
{
    RecoveryModeChecker::SetParameterReaderForTest([]() {
        return std::string("0x6");
    });
    EXPECT_TRUE(RecoveryModeChecker::IsRecoveryMode());

    RecoveryModeChecker::SetParameterReaderForTest([]() {
        return std::string("0x0");
    });
    EXPECT_FALSE(RecoveryModeChecker::IsRecoveryMode());
    RecoveryModeChecker::ClearParameterReaderForTest();
}
