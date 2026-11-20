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

#include "bus_extension/pcie/policy/pcie_driver_match_policy.h"
#include "bus_extension/pcie/common/pcie_wildcard_match.h"
#include "hilog_wrapper.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <set>
#include <string>

namespace OHOS {
namespace ExternalDeviceManager {

namespace {
constexpr char DSL_KEXT_SIGNING_ALLOWED[] = "1";

bool ResolveRealPath(const std::string &path, std::string &resolvedPath)
{
    char realPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), realPath) == nullptr) {
        return false;
    }
    resolvedPath.assign(realPath);
    return true;
}

bool ValidateAliasFieldValue(const std::string &alias, size_t pos, size_t valueLen, bool &hasWildcard)
{
    hasWildcard = false;
    for (size_t index = 0; index < valueLen; ++index) {
        char value = alias[pos + index];
        if (value == '?') {
            hasWildcard = true;
        }
        if (!std::isxdigit(static_cast<unsigned char>(value)) && value != '?') {
            return false;
        }
    }
    return true;
}
} // namespace

/* ── ScoreAlias ───────────────────────────────────────────────────────────
 *
 * PCI alias format:  pci:v<vendor>d<device>sv<subvendor>sd<subdevice>
 *                        bc<baseclass>sc<subclass>i<interface>
 *
 * The seven fields appear in fixed order.  Score = count of fields where
 * the value is NOT a wildcard ('*' or '?').
 *
 * Fields are parsed at their expected offsets so hexadecimal values cannot be
 * mistaken for field tags.
 */
int32_t PcieDriverMatchPolicy::ScoreAlias(const std::string &alias)
{
    constexpr char PCI_ALIAS_PREFIX[] = "pci:";
    constexpr size_t PCI_ALIAS_PREFIX_LENGTH = sizeof(PCI_ALIAS_PREFIX) - 1;
    static const struct {
        const char *tag;
        size_t tagLen;
        size_t valueLen;
    } fields[] = {
        {"v",  1, 8},
        {"d",  1, 8},
        {"sv", 2, 8},
        {"sd", 2, 8},
        {"bc", 2, 2},
        {"sc", 2, 2},
        {"i",  1, 2}
    };

    int32_t score = 0;
    size_t pos = alias.find(PCI_ALIAS_PREFIX);
    if (pos == std::string::npos) {
        return 0; /* not a PCI alias */
    }
    pos += PCI_ALIAS_PREFIX_LENGTH;

    for (const auto &field : fields) {
        if (alias.compare(pos, field.tagLen, field.tag) != 0) {
            break; /* field missing entirely — stop */
        }
        pos += field.tagLen;

        if (pos >= alias.size()) {
            break;
        }
        if (alias[pos] == '*') {
            ++pos;
            continue;
        }
        if (pos + field.valueLen > alias.size()) {
            break;
        }
        bool hasWildcard = false;
        if (!ValidateAliasFieldValue(alias, pos, field.valueLen, hasWildcard)) {
            break;
        }
        if (!hasWildcard) {
            ++score;
        }
        pos += field.valueLen;
    }
    return score;
}

/* ── KernelReleaseMatches ─────────────────────────────────────────────────
 *
 * Requires the first whitespace-delimited field of the module version
 * to equal the complete host kernel release, including any release suffix.
 * Subsequent vermagic flags are not part of the release.
 *
 * Empty inputs always return false.
 */
namespace {

std::string FirstToken(const std::string &value)
{
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    size_t end = start;
    while (end < value.size() && !std::isspace(static_cast<unsigned char>(value[end]))) {
        ++end;
    }
    return value.substr(start, end - start);
}

} // namespace

bool PcieDriverMatchPolicy::KernelReleaseMatches(const std::string &moduleVersion, const std::string &hostRelease)
{
    if (moduleVersion.empty() || hostRelease.empty()) {
        return false;
    }
    return FirstToken(moduleVersion) == hostRelease;
}

/* ── Gate 6: PathWithinSandbox ───────────────────────────────────────────
 *
 * Resolves both the driver path and the sandbox root to their canonical
 * absolute paths (realpath) before checking containment.  This closes
 * symlink-based and ../ traversal escapes that a naive prefix-match
 * would miss.
 */
bool PcieDriverMatchPolicy::PathWithinSandbox(const std::string &driverPath, const std::string &sandboxRoot)
{
    if (sandboxRoot.empty() || driverPath.empty()) {
        return false;
    }

    /* resolve sandbox root to canonical form */
    std::string root;
    if (!ResolveRealPath(sandboxRoot, root)) {
        return false;
    }
    if (root.back() != '/') {
        root += '/';
    }

    /* resolve driver path to canonical form */
    std::string resolved;
    if (!ResolveRealPath(driverPath, resolved)) {
        return false;
    }

    /* the resolved driver path must lie inside the resolved sandbox root */
    return resolved.size() >= root.size() && resolved.compare(0, root.size(), root) == 0;
}

/* ── Gate 5: IsDuplicate (non-blocking skip) ──────────────────────────── */
// Check whether the module is already loaded or built into the kernel to avoid loading it again.
bool PcieDriverMatchPolicy::IsDuplicate(
    const PcieDriverMetadata &metadata, const std::vector<std::string> &loadedModules)
{
    if (metadata.moduleName.empty()) {
        return false;
    }
    return std::any_of(loadedModules.begin(), loadedModules.end(), [&metadata](const std::string &moduleName) {
        return NormalizePcieModuleName(moduleName) == NormalizePcieModuleName(metadata.moduleName);
    });
}

/* ── Gate 4: DependencyMet ─────────────────────────────────────────────── */
bool PcieDriverMatchPolicy::DependencyMet(
    const PcieDriverMetadata &metadata, const std::vector<std::string> &dependencySnapshot)
{
    for (const auto &dep : metadata.depends) {
        if (std::none_of(dependencySnapshot.begin(), dependencySnapshot.end(), [&dep](const std::string &moduleName) {
                return NormalizePcieModuleName(moduleName) == NormalizePcieModuleName(dep);
            })) {
            return false;
        }
    }
    return true;
}

/* ── Gate 1: AliasMatches (bidirectional WildcardMatch, best score) ──── */
bool PcieDriverMatchPolicy::AliasMatches(
    const PcieDeviceInfo &board, const PcieDriverMetadata &metadata, std::string &matchedAlias, int32_t &aliasScore)
{
    if (!board.isModaliasValid || !metadata.isParseable || board.modalias.empty()) {
        aliasScore = 0;
        return false;
    }

    int32_t bestScore = -1;
    std::string bestAlias;
    for (const auto &alias : metadata.aliases) {
        if (alias.empty()) {
            continue;
        }
        if (PcieWildcardMatch(alias, board.modalias)) {
            int32_t score = ScoreAlias(alias);
            if (score > bestScore) {
                bestScore = score;
                bestAlias = alias;
            }
        }
    }

    if (bestScore >= 0) {
        matchedAlias = bestAlias;
        aliasScore = bestScore;
        return true;
    }
    aliasScore = 0;
    return false;
}

/* ── Gate 2: KabiMatches (kabi preferred, vermagic fallback) ────────── */
bool PcieDriverMatchPolicy::KabiMatches(
    const PcieDriverMetadata &metadata, const KernelSecurityContext &securityContext)
{
    const bool kabiMatched = securityContext.kernelVersionAvailable && !metadata.kabi.empty() &&
        metadata.kabi == securityContext.kernelVersion;
    if (kabiMatched) {
        return true;
    }

    return securityContext.kernelReleaseAvailable && !metadata.vermagic.empty() &&
        KernelReleaseMatches(metadata.vermagic, securityContext.kernelRelease);
}

/* ── Gate 3: DslAllowed ────────────────────────────────────────────────── */
bool PcieDriverMatchPolicy::DslAllowed(const KernelSecurityContext &securityContext)
{
    if (!securityContext.dslAvailable) {
        return false;
    }
    return securityContext.dslSafetyLevel == DSL_KEXT_SIGNING_ALLOWED;
}

/* ── KABI precision for tie-breaking ───────────────────────────────────── */
int PcieDriverMatchPolicy::KabiPrecision(bool kabiMatched, bool vermagicMatched, bool kabiAvailable)
{
    constexpr int KABI_AND_VERMAGIC_MATCHED = 4;
    constexpr int VERMAGIC_MATCHED = 3;
    constexpr int KABI_MATCHED = 2;
    constexpr int KABI_AVAILABLE = 1;
    constexpr int NO_KABI_MATCH = 0;

    if (kabiMatched && vermagicMatched) {
        return KABI_AND_VERMAGIC_MATCHED;
    }
    if (vermagicMatched) {
        return VERMAGIC_MATCHED;
    }
    if (kabiMatched) {
        return KABI_MATCHED;
    }
    if (kabiAvailable) {
        return KABI_AVAILABLE;
    }
    return NO_KABI_MATCH;
}

/* ── Tie-breaker ───────────────────────────────────────────────────────── */
bool PcieDriverMatchPolicy::IsBetterDecision(
    const PcieDriverPolicyDecision &firstDecision, const PcieDriverPolicyDecision &secondDecision)
{
    if (firstDecision.aliasScore != secondDecision.aliasScore) {
        return firstDecision.aliasScore > secondDecision.aliasScore;
    }

    int firstKabiScore =
        KabiPrecision(firstDecision.kabiMatched, firstDecision.vermagicMatched, firstDecision.kabiAvailable);
    int secondKabiScore =
        KabiPrecision(secondDecision.kabiMatched, secondDecision.vermagicMatched, secondDecision.kabiAvailable);
    if (firstKabiScore != secondKabiScore) {
        return firstKabiScore > secondKabiScore;
    }

    if (firstDecision.driverMtime != secondDecision.driverMtime) {
        return firstDecision.driverMtime > secondDecision.driverMtime;
    }
    return firstDecision.driverPath < secondDecision.driverPath;
}

/* ── RankDecisions: one winner per board, rest → LOWER_PRECISION ──────── */
void PcieDriverMatchPolicy::RankDecisions(std::vector<PcieDriverPolicyDecision> &decisions)
{
    std::stable_sort(decisions.begin(), decisions.end(), IsBetterDecision);
    std::set<std::string> boardsWithPreferredDriver;
    for (auto &decision : decisions) {
        if (!decision.checkPassed) {
            continue;
        }
        if (!boardsWithPreferredDriver.insert(decision.boardId).second) {
            decision.checkPassed = false;
            decision.blockReason = PcieDriverFailureReason::LOWER_PRECISION;
        }
    }
}

bool PcieDriverMatchPolicy::EvaluateMatchAndSecurityGates(const PcieDeviceInfo &board,
    const PcieDriverRecord &, const PcieDriverMetadata &driverMetadata,
    const PcieDriverPolicyInput &policyInput, PcieDriverPolicyDecision &decision)
{
    if (!DslAllowed(policyInput.securityContext)) {
        EDM_LOGI(MODULE_SERVICE, "  gate1: board=%{public}s result=BLOCK reason=DSL_DENIED",
            board.boardId.c_str());
        decision.blockReason = PcieDriverFailureReason::DSL_DENIED;
        return false;
    }
    decision.dslAllowed = true;

    std::string matchedAlias;
    int32_t aliasScore;
    if (!AliasMatches(board, driverMetadata, matchedAlias, aliasScore)) {
        EDM_LOGI(MODULE_SERVICE,
            "  gate2: board=%{public}s "
            "result=BLOCK reason=ALIAS_NOT_MATCHED",
            board.boardId.c_str());
        decision.blockReason = PcieDriverFailureReason::ALIAS_NOT_MATCHED;
        return false;
    }
    decision.aliasMatched = matchedAlias;
    decision.aliasScore = aliasScore;

    decision.kabiAvailable = policyInput.securityContext.kernelVersionAvailable;
    decision.kabiMatched = !driverMetadata.kabi.empty() &&
        driverMetadata.kabi == policyInput.securityContext.kernelVersion;
    decision.vermagicMatched = policyInput.securityContext.kernelReleaseAvailable &&
        KernelReleaseMatches(driverMetadata.vermagic, policyInput.securityContext.kernelRelease);
    if (!decision.kabiMatched && !decision.vermagicMatched) {
        EDM_LOGI(MODULE_SERVICE,
            "  gate3: board=%{public}s "
            "result=BLOCK reason=KABI_MISMATCH",
            board.boardId.c_str());
        decision.blockReason = PcieDriverFailureReason::KABI_MISMATCH;
        return false;
    }

    return true;
}

bool PcieDriverMatchPolicy::EvaluateDependencyAndLoadGates(const PcieDeviceInfo &board,
    const PcieDriverRecord &driverFileInfo, const PcieDriverMetadata &driverMetadata,
    const PcieDriverPolicyInput &policyInput, PcieDriverPolicyDecision &decision)
{
    if (!DependencyMet(driverMetadata, policyInput.dependencySnapshot)) {
        EDM_LOGI(MODULE_SERVICE,
            "  gate4: board=%{public}s "
            "result=BLOCK reason=DEPENDENCY_FAILED",
            board.boardId.c_str());
        decision.blockReason = PcieDriverFailureReason::DEPENDENCY_FAILED;
        return false;
    }
    decision.dependencyMet = true;

    if (IsDuplicate(driverMetadata, policyInput.loadedModules)) {
        EDM_LOGI(MODULE_SERVICE,
            " gate5: board=%{public}s "
            "result=BLOCK reason=DUPLICATE_LOADED",
            board.boardId.c_str());
        decision.skipAlreadyLoaded = true;
        decision.blockReason = PcieDriverFailureReason::DUPLICATE_LOADED;
        return false;
    }

    if (!PathWithinSandbox(driverFileInfo.driverPath, policyInput.sandboxRoot)) {
        EDM_LOGI(MODULE_SERVICE, "  gate6: board=%{public}s result=BLOCK reason=PATH_ESCAPE",
            board.boardId.c_str());
        decision.blockReason = PcieDriverFailureReason::PATH_ESCAPE;
        return false;
    }
    return true;
}

void PcieDriverMatchPolicy::EvaluateBoard(const PcieDeviceInfo &board, const PcieDriverPolicyInput &policyInput,
    std::vector<PcieDriverPolicyDecision> &decisions)
{
    if (!board.boundDriver.empty()) {
        EDM_LOGI(MODULE_SERVICE,
            "  gate0: board=%{public}s boundDriver=%{public}s "
            "result=BLOCK reason=ALREADY_BOUND",
            board.boardId.c_str(), board.boundDriver.c_str());
        PcieDriverPolicyDecision decision;
        decision.boardId = board.boardId;
        decision.boardHasBoundDriver = true;
        decision.blockReason = PcieDriverFailureReason::ALREADY_BOUND;
        decisions.push_back(decision);
        return;
    }

    for (const auto &driverCandidate : policyInput.driverCandidates) {
        const auto &driverFileInfo = driverCandidate.driver;
        if (!driverFileInfo.isValid) {
            continue;
        }
        const auto &driverMetadata = driverCandidate.metadata;
        PcieDriverPolicyDecision decision;
        decision.boardId = board.boardId;
        decision.driverPath = driverFileInfo.driverPath;
        decision.moduleName = driverMetadata.moduleName;
        decision.driverMtime = static_cast<int64_t>(driverFileInfo.mtime);
        if (EvaluateMatchAndSecurityGates(board, driverFileInfo, driverMetadata, policyInput, decision) &&
            EvaluateDependencyAndLoadGates(board, driverFileInfo, driverMetadata, policyInput, decision)) {
            decision.checkPassed = true;
        }
        decisions.push_back(decision);
    }
}

/* ── Evaluate: 6-gate chain, double loop board×driver ─────────────────── */
std::vector<PcieDriverPolicyDecision> PcieDriverMatchPolicy::Evaluate(const PcieDriverPolicyInput &policyInput)
{
    std::vector<PcieDriverPolicyDecision> decisions;

    for (const auto &board : policyInput.boardRecords) {
        if (!board.isModaliasValid) {
            continue;
        }
        EvaluateBoard(board, policyInput, decisions);
    }

    RankDecisions(decisions);
    const size_t passedCount =
        std::count_if(decisions.begin(), decisions.end(), [](const PcieDriverPolicyDecision &decision) {
            return decision.checkPassed;
        });
    EDM_LOGI(MODULE_SERVICE,
        "PCIe policy evaluation completed: boards=%{public}zu drivers=%{public}zu "
        "decisions=%{public}zu passed=%{public}zu blocked=%{public}zu",
        policyInput.boardRecords.size(), policyInput.driverCandidates.size(), decisions.size(), passedCount,
        decisions.size() - passedCount);
    return decisions;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
