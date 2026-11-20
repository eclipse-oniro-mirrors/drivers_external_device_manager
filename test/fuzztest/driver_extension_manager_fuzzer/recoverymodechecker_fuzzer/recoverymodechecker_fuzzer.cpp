/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "recoverymodechecker_fuzzer.h"

#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "recovery_mode_checker.h"

namespace OHOS {
namespace ExternalDeviceManager {
namespace {
constexpr uint8_t METHOD_COUNT = 4;
constexpr uint8_t METHOD_READ_SYSTEM_PARAMETER = 2;
constexpr uint8_t RECOVERY_VALUE_COUNT = 12;
constexpr size_t MAX_PARAMETER_VALUE_LEN = 256;

// Boot parameter values treated as recovery mode, plus values that must stay non-recovery.
const char *g_recoveryValues[RECOVERY_VALUE_COUNT] = {BOPD_BEFORE_DL, BOPD_STG_RO_BEFORE_DL,
    BOPD_HW_DEG_BEFORE_DL, BOPD_STG_RO_HW_DEG_BEFORE_DL, BOPD_AFTER_DL, BOPD_HW_DEG_AFTER_DL,
    BOPD_HW_DEG_VIA_ERECOVERY, "", "2", "0X2", "0x2abc", "0x2 "};

std::string ConsumeParameterValue(FuzzedDataProvider &fdp)
{
    uint8_t valueIndex = fdp.ConsumeIntegral<uint8_t>() % (RECOVERY_VALUE_COUNT + 1);
    if (valueIndex < RECOVERY_VALUE_COUNT) {
        return g_recoveryValues[valueIndex];
    }
    return fdp.ConsumeRandomLengthString(MAX_PARAMETER_VALUE_LEN);
}

void FuzzValidRecoveryValue(FuzzedDataProvider &fdp)
{
    std::string value = ConsumeParameterValue(fdp);
    RecoveryModeChecker::IsBopdRunMode(value);
}

void FuzzRecoveryModeWithReader(FuzzedDataProvider &fdp)
{
    std::string value = ConsumeParameterValue(fdp);
    RecoveryModeChecker::IsRecoveryModeWithReader([value]() {
        return value;
    });
}

void FuzzReadSystemParameter()
{
    RecoveryModeChecker::IsRecoveryModeWithReader(nullptr);
}

void FuzzRecoveryModeForTest(FuzzedDataProvider &fdp)
{
    std::string value = ConsumeParameterValue(fdp);
    RecoveryModeChecker::SetParameterReaderForTest([value]() {
        return value;
    });
    RecoveryModeChecker::IsRecoveryMode();
    RecoveryModeChecker::ClearParameterReaderForTest();
}
} // namespace

static void FuzzRecoveryModeChecker(FuzzedDataProvider &fdp)
{
    uint8_t method = fdp.ConsumeIntegral<uint8_t>() % METHOD_COUNT;
    switch (method) {
        case 0:
            FuzzValidRecoveryValue(fdp);
            break;
        case 1:
            FuzzRecoveryModeWithReader(fdp);
            break;
        case METHOD_READ_SYSTEM_PARAMETER:
            FuzzReadSystemParameter();
            break;
        default:
            FuzzRecoveryModeForTest(fdp);
            break;
    }
}
} // namespace ExternalDeviceManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    OHOS::ExternalDeviceManager::FuzzRecoveryModeChecker(fdp);
    return 0;
}
