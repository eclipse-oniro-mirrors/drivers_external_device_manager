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

#include "recovery_mode_checker.h"

#ifdef RECOVERY_MODE_TEST_HOOKS
#include <mutex>
#endif

#include "hilog_wrapper.h"
#include "parameters.h"

namespace OHOS {
namespace ExternalDeviceManager {
namespace {
constexpr const char *BOPD_MODE_DEFAULT = "NA";

#ifdef RECOVERY_MODE_TEST_HOOKS
std::mutex g_testReaderMutex;
#endif
} // namespace

bool RecoveryModeChecker::IsBopdRunMode(const std::string &value)
{
    return value == BOPD_BEFORE_DL || value == BOPD_STG_RO_BEFORE_DL || value == BOPD_HW_DEG_BEFORE_DL ||
        value == BOPD_STG_RO_HW_DEG_BEFORE_DL || value == BOPD_AFTER_DL || value == BOPD_HW_DEG_AFTER_DL ||
        value == BOPD_HW_DEG_VIA_ERECOVERY;
}

bool RecoveryModeChecker::IsRecoveryMode()
{
#ifdef RECOVERY_MODE_TEST_HOOKS
    ParameterReader reader;
    {
        std::lock_guard<std::mutex> lock(g_testReaderMutex);
        reader = testReader_;
    }
    if (reader != nullptr) {
        return IsBopdRunMode(reader());
    }
#endif
    std::string value = system::GetParameter(BOOT_BOPD_MODE, BOPD_MODE_DEFAULT);
    bool isBopd = IsBopdRunMode(value);
    if (isBopd) {
        EDM_LOGI(MODULE_EA_MGR, "%{public}s:%{public}s is BOPD mode", BOOT_BOPD_MODE, value.c_str());
    } else if (value != BOPD_MODE_DEFAULT) {
        EDM_LOGW(MODULE_EA_MGR, "%{public}s:%{public}s is invalid, treat as non-BOPD", BOOT_BOPD_MODE, value.c_str());
    }
    return isBopd;
}

#ifdef RECOVERY_MODE_TEST_HOOKS
bool RecoveryModeChecker::IsRecoveryModeWithReader(const ParameterReader &reader)
{
    std::string value = reader ? reader() : system::GetParameter(BOOT_BOPD_MODE, BOPD_MODE_DEFAULT);
    return IsBopdRunMode(value);
}

RecoveryModeChecker::ParameterReader RecoveryModeChecker::testReader_;

void RecoveryModeChecker::SetParameterReaderForTest(const ParameterReader &reader)
{
    std::lock_guard<std::mutex> lock(g_testReaderMutex);
    testReader_ = reader;
}

void RecoveryModeChecker::ClearParameterReaderForTest()
{
    std::lock_guard<std::mutex> lock(g_testReaderMutex);
    testReader_ = nullptr;
}
#endif
} // namespace ExternalDeviceManager
} // namespace OHOS
