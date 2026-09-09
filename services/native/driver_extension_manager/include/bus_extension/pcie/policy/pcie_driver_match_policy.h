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

#ifndef OHOS_EXTERNAL_DEVICE_MANAGER_PCIE_DRIVER_MATCH_POLICY_H
#define OHOS_EXTERNAL_DEVICE_MANAGER_PCIE_DRIVER_MATCH_POLICY_H

#include "bus_extension/pcie/common/pcie_driver_types.h"

#include <string>
#include <vector>

namespace OHOS {
namespace ExternalDeviceManager {

/*
 * PcieDriverMatchPolicy — pure-function policy engine for PCIe driver matching.
 *
 * Evaluates each board x driver pair through a fixed-order 6-gate chain,
 * producing deterministic PcieDriverPolicyDecisions.  Post-processing
 * ranks multiple eligible drivers per board and selects the best match.
 *
 * This module does NOT perform I/O — all inputs arrive through
 * PcieDriverPolicyInput.
 */
class PcieDriverMatchPolicy {
public:
    /* ── Public entry point ────────────────────────────────────────── */

    /* Run the full 6-gate evaluation for every board x driver pair,
     * then rank and resolve conflicts. */
    std::vector<PcieDriverPolicyDecision> Evaluate(const PcieDriverPolicyInput& policyInput);

    /* ── Pure function utilities (public, stateless, testable) ──────── */

    static int32_t ScoreAlias(const std::string& alias);
    static bool KernelReleaseMatches(const std::string& moduleVersion,
                                     const std::string& hostRelease);

    /* ── Per-gate helpers ──────────────────────────────────────────── */

    /* Gate 1: match board modalias against driver aliases.
     * Bidirectional PcieWildcardMatch over all aliases, select highest ScoreAlias. */
    static bool AliasMatches(const PcieDeviceInfo& board,
                             const PcieDriverMetadata& metadata,
                             std::string& matchedAlias,
                             int32_t& aliasScore);

    /* Gate 2: KABI check — kabi or vermagic (fallback) vs host version. */
    static bool KabiMatches(const PcieDriverMetadata& metadata,
                            const KernelSecurityContext& securityContext);

    /* Gate 3: DSL safety-level whitelist check. */
    static bool DslAllowed(const KernelSecurityContext& securityContext);

    /* Gate 4: all depends entries must exist in dependency snapshot. */
    static bool DependencyMet(const PcieDriverMetadata& metadata,
                             const std::vector<std::string>& dependencySnapshot);

    /* Gate 5: skip if module is already loaded or built into the kernel. */
    static bool IsDuplicate(const PcieDriverMetadata& metadata,
                            const std::vector<std::string>& loadedModules);

    /* Gate 6: driverPath must have sandbox root prefix. */
    static bool PathWithinSandbox(const std::string& driverPath,
                                  const std::string& sandboxRoot);

    /* ── Post-processing ───────────────────────────────────────────── */

    /* Sort decisions by priority; per board, best keeps checkPassed=true, rest → LOWER_PRECISION. */
    static void RankDecisions(std::vector<PcieDriverPolicyDecision>& decisions);

    /* Tie-breaker for RankDecisions. */
    static bool IsBetterDecision(const PcieDriverPolicyDecision& firstDecision,
                                 const PcieDriverPolicyDecision& secondDecision);

private:
    static int  KabiPrecision(bool kabiMatched, bool vermagicMatched, bool kabiAvailable);

    static bool EvaluateMatchAndSecurityGates(const PcieDeviceInfo& board,
        const PcieDriverRecord& driverFileInfo, const PcieDriverMetadata& driverMetadata,
        const PcieDriverPolicyInput& policyInput, PcieDriverPolicyDecision& decision);
    static bool EvaluateDependencyAndLoadGates(const PcieDeviceInfo& board,
        const PcieDriverRecord& driverFileInfo, const PcieDriverMetadata& driverMetadata,
        const PcieDriverPolicyInput& policyInput, PcieDriverPolicyDecision& decision);
    static void EvaluateBoard(const PcieDeviceInfo& board, const PcieDriverPolicyInput& policyInput,
        std::vector<PcieDriverPolicyDecision>& decisions);
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif
