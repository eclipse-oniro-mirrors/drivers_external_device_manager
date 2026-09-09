/**
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

#ifndef PCIE_DRIVER_REPORT_SYS_EVENT_H
#define PCIE_DRIVER_REPORT_SYS_EVENT_H

#include <string>
#include <vector>

#include "v1_0/pcie_types.h"

namespace OHOS {
namespace ExternalDeviceManager {

struct PcieDriverPolicyDecision;
using OHOS::HDI::Pcie::V1_0::PcieDriverLoadResult;

class PcieDriverReportSysEvent {
public:
    /* Report a merged decision+load event: board-level match info + module-level result */
    static void ReportDecision(const PcieDriverPolicyDecision& decision,
                               const std::string& moduleName,
                               const std::string& statusMessage);

    /* Convenience: pair decisions with load results and report merged events */
    static void ReportReconcileRound(
        const std::vector<PcieDriverPolicyDecision>& decisions,
        const std::vector<PcieDriverLoadResult>& loadResults);
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // PCIE_DRIVER_REPORT_SYS_EVENT_H
