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

#ifndef DRIVERS_EXTERNAL_DEVICE_MANAGER_RECOVERY_MODE_CHECKER_H
#define DRIVERS_EXTERNAL_DEVICE_MANAGER_RECOVERY_MODE_CHECKER_H

#include <string>
#ifdef RECOVERY_MODE_TEST_HOOKS
#include <functional>
#endif

#define BOPD_BEFORE_DL "0x2"
#define BOPD_STG_RO_BEFORE_DL "0x3"
#define BOPD_HW_DEG_BEFORE_DL "0x6"
#define BOPD_STG_RO_HW_DEG_BEFORE_DL "0x7"
#define BOPD_AFTER_DL "0xa"
#define BOPD_HW_DEG_AFTER_DL "0xe"
#define BOPD_HW_DEG_VIA_ERECOVERY "0xf"

namespace OHOS {
namespace ExternalDeviceManager {
class RecoveryModeChecker final {
public:
    static constexpr const char *BOOT_BOPD_MODE = "ohos.boot.bopd.mode";

    // True when ohos.boot.bopd.mode is one of the seven BOPD values.
    static bool IsRecoveryMode();
    // True when value exactly matches one of the BOPD mode constants.
    static bool IsBopdRunMode(const std::string &value);

#ifdef RECOVERY_MODE_TEST_HOOKS
    using ParameterReader = std::function<std::string()>;
    static bool IsRecoveryModeWithReader(const ParameterReader &reader);
    static void SetParameterReaderForTest(const ParameterReader &reader);
    static void ClearParameterReaderForTest();
#endif

private:
    RecoveryModeChecker() = delete;
#ifdef RECOVERY_MODE_TEST_HOOKS
    static ParameterReader testReader_;
#endif
};
} // namespace ExternalDeviceManager
} // namespace OHOS

#endif // DRIVERS_EXTERNAL_DEVICE_MANAGER_RECOVERY_MODE_CHECKER_H
