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

#include "pcie_driver_report_sys_event.h"

#include "hisysevent.h"
#include "hilog_wrapper.h"
#include "bus_extension/pcie/common/pcie_driver_types.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace ExternalDeviceManager {

namespace {
constexpr const char* EVENT_NAME = "PCIE_MATCH_EVENT";

bool ReportDecisionLoadResults(const PcieDriverPolicyDecision &decision,
    const std::vector<PcieDriverLoadResult> &loadResults, std::vector<bool> &reportedResults)
{
    if (!decision.checkPassed) {
        return false;
    }
    bool loadResultReported = false;
    for (size_t resultIndex = 0; resultIndex < loadResults.size(); ++resultIndex) {
        const auto &loadResult = loadResults[resultIndex];
        if (loadResult.driverIdentity.boardId != decision.boardId) {
            continue;
        }
        PcieDriverReportSysEvent::ReportDecision(
            decision, loadResult.driverIdentity.moduleName, loadResult.statusMessage);
        loadResultReported = true;
        reportedResults[resultIndex] = true;
    }
    return loadResultReported;
}
}

void PcieDriverReportSysEvent::ReportDecision(const PcieDriverPolicyDecision& decision,
    const std::string& moduleName, const std::string& statusMessage)
{
    int32_t reportStatus = HiSysEventWrite(
        HiSysEvent::Domain::EXTERNAL_DEVICE, EVENT_NAME,
        HiSysEvent::EventType::STATISTIC,
        "BOARD_ID", decision.boardId,
        "ALIAS_MATCHED", decision.aliasMatched,
        "ALIAS_SCORE", decision.aliasScore,
        "BOARD_HAS_BOUND_DRIVER", decision.boardHasBoundDriver ? 1 : 0,
        "BLOCK_REASON", PcieDriverFailureReasonToString(decision.blockReason),
        "MODULE_NAME", moduleName,
        "STATUS_MSG", statusMessage);
    if (reportStatus != 0) {
        EDM_LOGW(MODULE_SERVICE, "PCIE_MATCH_EVENT ReportDecision failed ret=%{public}d", reportStatus);
    }
}

void PcieDriverReportSysEvent::ReportReconcileRound(const std::vector<PcieDriverPolicyDecision>& decisions,
    const std::vector<PcieDriverLoadResult>& loadResults)
{
    std::vector<bool> reportedResults(loadResults.size(), false);
    for (const auto& decision : decisions) {
        if (ReportDecisionLoadResults(decision, loadResults, reportedResults)) {
            continue;
        }
        ReportDecision(decision, decision.moduleName, decision.checkPassed ? "" : "blocked_by_policy");
    }
    for (size_t resultIndex = 0; resultIndex < loadResults.size(); ++resultIndex) {
        const auto &loadResult = loadResults[resultIndex];
        if (reportedResults[resultIndex] || loadResult.statusCode == 0) {
            continue;
        }
        PcieDriverPolicyDecision failureDecision;
        failureDecision.boardId = loadResult.driverIdentity.boardId;
        failureDecision.driverPath = loadResult.driverIdentity.driverPath;
        failureDecision.blockReason = PcieDriverFailureReason::LOAD_FAILED;
        std::string statusMessage = loadResult.statusMessage + " [status=" +
            std::to_string(loadResult.statusCode) + "]";
        ReportDecision(failureDecision, loadResult.driverIdentity.moduleName, statusMessage);
    }
}

} // namespace ExternalDeviceManager
} // namespace OHOS
