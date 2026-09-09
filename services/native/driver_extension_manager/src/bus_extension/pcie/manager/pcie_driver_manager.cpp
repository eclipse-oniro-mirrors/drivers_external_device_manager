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

#include "bus_extension/pcie/manager/pcie_driver_manager.h"
#include "bus_extension/pcie/policy/pcie_driver_match_policy.h"
#include "bus_extension/pcie/system/security_query.h"
#include "hilog_wrapper.h"
#include <chrono>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <dirent.h>
#include <fstream>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace OHOS {
namespace ExternalDeviceManager {

constexpr char DRIVER_FILE_EXTENSION[] = ".ko";
constexpr size_t DRIVER_FILE_EXTENSION_LENGTH = sizeof(DRIVER_FILE_EXTENSION) - 1;

namespace {

bool ResolveRealPath(const std::string &path, std::string &resolvedPath)
{
    char realPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), realPath) == nullptr) {
        return false;
    }
    resolvedPath.assign(realPath);
    return true;
}

// Remove the directory and .ko suffix, then replace hyphens with underscores to normalize the module name.
std::string NormalizeModuleName(const std::string &moduleName)
{
    std::string normalized = moduleName;
    size_t slashPosition = normalized.find_last_of('/');
    if (slashPosition != std::string::npos) {
        normalized.erase(0, slashPosition + 1);
    }
    if (normalized.size() > DRIVER_FILE_EXTENSION_LENGTH &&
        normalized.compare(normalized.size() - DRIVER_FILE_EXTENSION_LENGTH, DRIVER_FILE_EXTENSION_LENGTH,
            DRIVER_FILE_EXTENSION) == 0) {
        normalized.erase(normalized.size() - DRIVER_FILE_EXTENSION_LENGTH);
    }
    return NormalizePcieModuleName(normalized);
}

bool BuildPcieDriverRecord(
    const std::string &normalizedSandboxRoot, const std::string &entryName, PcieDriverRecord &driverRecord)
{
    if (entryName.size() < DRIVER_FILE_EXTENSION_LENGTH ||
        entryName.compare(entryName.size() - DRIVER_FILE_EXTENSION_LENGTH, DRIVER_FILE_EXTENSION_LENGTH,
            DRIVER_FILE_EXTENSION) != 0) {
        return false;
    }

    std::string driverPath = normalizedSandboxRoot + entryName;
    struct stat fileStatus;
    // Read file attributes; only regular files are accepted later, excluding symbolic links and directories.
    int32_t lstatResult = lstat(driverPath.c_str(), &fileStatus);
    if (lstatResult != 0 || !S_ISREG(fileStatus.st_mode)) {
        EDM_LOGW(MODULE_SERVICE, "ScanSandbox: reject non-regular entry=%{public}s errno=%{public}d", entryName.c_str(),
            lstatResult != 0 ? errno : 0);
        return false;
    }
    std::string resolvedDriverPath;
    if (!ResolveRealPath(driverPath, resolvedDriverPath)) {
        EDM_LOGW(
            MODULE_SERVICE, "ScanSandbox: realpath failed entry=%{public}s errno=%{public}d", entryName.c_str(), errno);
        return false;
    }
    if (normalizedSandboxRoot.empty() || normalizedSandboxRoot == "/" ||
        resolvedDriverPath.rfind(normalizedSandboxRoot, 0) != 0) {
        EDM_LOGW(MODULE_SERVICE, "ScanSandbox: reject outside sandbox entry=%{public}s", entryName.c_str());
        return false;
    }
    driverRecord.driverPath = driverPath;
    driverRecord.fileName = entryName;
    driverRecord.fileSize = static_cast<uint64_t>(fileStatus.st_size);
    driverRecord.mtime = static_cast<uint64_t>(fileStatus.st_mtime);
    driverRecord.isValid = true;
    return true;
}

} // namespace

PcieDriverManager::PcieDriverManager(PcieDriverManagerDeps driverManagerDependencies, PcieDriverManagerConfig config)
    : driverManagerDependencies_(std::move(driverManagerDependencies)), config_(std::move(config))
{
}

PcieDriverManager::~PcieDriverManager() = default;

KernelSecurityContext PcieDriverManager::BuildSecurityContext()
{
    KernelSecurityContext securityContext;
    securityContext.kernelVersion = GetKabiVersion();
    securityContext.kernelVersionAvailable = !securityContext.kernelVersion.empty();
    securityContext.kernelRelease = GetKernelVersion();
    securityContext.kernelReleaseAvailable = !securityContext.kernelRelease.empty();

    securityContext.dslSafetyLevel = GetDslSafetyLevel();
    securityContext.dslAvailable = !securityContext.dslSafetyLevel.empty();
    return securityContext;
}

// Read /proc/modules for loaded module names; this does not indicate whether they are bound to boards.
std::vector<std::string> PcieDriverManager::GetLoadedModuleNames(bool *readSucceeded) const
{
    if (readSucceeded != nullptr) {
        *readSucceeded = false;
    }
    std::vector<std::string> loadedModuleNames;
    std::ifstream moduleFile("/proc/modules");
    if (!moduleFile.is_open()) {
        EDM_LOGE(MODULE_SERVICE, "GetLoadedModuleNames: open failed errno=%{public}d", errno);
        return loadedModuleNames;
    }
    std::string moduleLine;
    while (std::getline(moduleFile, moduleLine)) {
        std::istringstream moduleStream(moduleLine);
        std::string moduleName;
        if (moduleStream >> moduleName) {
            loadedModuleNames.push_back(moduleName);
        }
    }
    const bool readFailed = moduleFile.bad();
    if (readFailed) {
        EDM_LOGE(MODULE_SERVICE, "GetLoadedModuleNames: read failed errno=%{public}d", errno);
    }
    if (readSucceeded != nullptr) {
        *readSucceeded = !readFailed;
    }
    return loadedModuleNames;
}

// Read modules.builtin for built-in module names to avoid scheduling them for loading again.
std::set<std::string> PcieDriverManager::GetBuiltinModuleNames(const std::string &builtinModulesPath) const
{
    std::set<std::string> builtinModules;
    if (builtinModulesPath.empty()) {
        EDM_LOGI(MODULE_SERVICE, "GetBuiltinModuleNames: path is not configured");
        return builtinModules;
    }
    std::ifstream builtinFile(builtinModulesPath);
    if (!builtinFile.is_open()) {
        EDM_LOGW(MODULE_SERVICE, "GetBuiltinModuleNames: open failed errno=%{public}d", errno);
        return builtinModules;
    }
    std::string modulePath;
    while (std::getline(builtinFile, modulePath)) {
        while (!modulePath.empty() &&
            (modulePath.back() == '\r' || modulePath.back() == '\n' || modulePath.back() == ' ' ||
                modulePath.back() == '\t')) {
            modulePath.pop_back();
        }
        std::string moduleName = NormalizeModuleName(modulePath);
        if (!moduleName.empty()) {
            builtinModules.insert(moduleName);
        }
    }
    if (builtinFile.bad()) {
        EDM_LOGW(MODULE_SERVICE, "GetBuiltinModuleNames: read failed errno=%{public}d", errno);
    }
    EDM_LOGI(MODULE_SERVICE, "GetBuiltinModuleNames: loaded %{public}zu modules", builtinModules.size());
    return builtinModules;
}

// Find metadata by driver file path, or return nullptr if no match is found.
const PcieDriverMetadata *PcieDriverManager::FindMetadataByDriverPath(
    const std::vector<PcieDriverMetadata> &metadataList, const std::string &driverPath) const
{
    for (const auto &metadata : metadataList) {
        if (metadata.driverPath == driverPath) {
            return &metadata;
        }
    }
    return nullptr;
}

/* ── ScanSandbox ─────────────────────────────────────────────────────── */
bool PcieDriverManager::ScanSandbox(const std::string &sandboxRoot, PcieDriverSandboxSnapshot &snapshot)
{
    DIR *sandboxDirectory = opendir(sandboxRoot.c_str());
    if (sandboxDirectory == nullptr) {
        EDM_LOGE(MODULE_SERVICE, "ScanSandbox: opendir failed errno=%{public}d", errno);
        return false;
    }

    std::string normalizedSandboxRoot;
    if (!ResolveRealPath(sandboxRoot, normalizedSandboxRoot)) {
        EDM_LOGE(MODULE_SERVICE, "ScanSandbox: realpath failed errno=%{public}d", errno);
        closedir(sandboxDirectory);
        return false;
    }
    if (!normalizedSandboxRoot.empty() && normalizedSandboxRoot.back() != '/') {
        normalizedSandboxRoot += '/';
    }

    errno = 0;
    struct dirent *directoryEntry = readdir(sandboxDirectory);
    while (directoryEntry != nullptr) {
        std::string entryName = directoryEntry->d_name;
        PcieDriverRecord driverRecord;
        if (BuildPcieDriverRecord(normalizedSandboxRoot, entryName, driverRecord)) {
            snapshot.driverRecords.push_back(driverRecord);
        }
        errno = 0;
        directoryEntry = readdir(sandboxDirectory);
    }
    const int readError = errno;
    if (readError != 0) {
        EDM_LOGE(MODULE_SERVICE, "ScanSandbox: readdir failed errno=%{public}d", readError);
        closedir(sandboxDirectory);
        snapshot.driverRecords.clear();
        return false;
    }
    closedir(sandboxDirectory);
    return true;
}

void PcieDriverManager::ParseMetadata(const std::vector<PcieDriverRecord> &driverRecords,
    PcieDriverManagerStatus &evaluationStatus, std::vector<PcieDriverCandidate> &driverCandidates) const
{
    driverCandidates.clear();
    for (const auto &driverRecord : driverRecords) {
        if (!driverRecord.isValid) {
            continue;
        }
        // Calls the supplied parser: PcieDriverMetadataParser::Parse in production, a substitute in tests.
        auto metadataParseResult = driverManagerDependencies_.parser(driverRecord);
        if (metadataParseResult.parseSucceeded && metadataParseResult.metadata.isParseable) {
            driverCandidates.push_back({driverRecord, metadataParseResult.metadata});
            continue;
        }
        constexpr PcieDriverLoadStatus statusCode = PcieDriverLoadStatus::SA_METADATA_PARSE_FAILED;
        AddLoadFailure(driverRecord.driverPath, statusCode, "driver metadata parsing failed", evaluationStatus);
    }
    EDM_LOGI(MODULE_SERVICE, "ParseMetadata: metadata parsed %{public}zu/%{public}zu drivers parseable",
        driverCandidates.size(), driverRecords.size());
}

void PcieDriverManager::MergeModprobeAliases(PcieDriverMetadata &driverMetadata) const
{
    for (const auto &[moduleName, aliasPatterns] : driverManagerDependencies_.modprobeConfig.aliasPatternsByModule) {
        if (NormalizePcieModuleName(moduleName) == NormalizePcieModuleName(driverMetadata.moduleName)) {
            driverMetadata.aliases.insert(driverMetadata.aliases.end(), aliasPatterns.begin(), aliasPatterns.end());
        }
    }
}

std::vector<PcieDriverMetadata> PcieDriverManager::BuildMetadataList(
    std::vector<PcieDriverCandidate> &driverCandidates) const
{
    std::vector<PcieDriverMetadata> metadataList;
    metadataList.reserve(driverCandidates.size());
    for (auto &driverCandidate : driverCandidates) {
        MergeModprobeAliases(driverCandidate.metadata);
        metadataList.push_back(driverCandidate.metadata);
    }
    return metadataList;
}

void PcieDriverManager::UpdateModuleStates(const std::vector<std::string> &availableModuleNames,
    const std::vector<PcieDriverMetadata> &metadataList, PcieDriverManagerStatus &evaluationStatus) const
{
    std::set<std::string> loadedModuleNames;
    for (const auto &moduleName : availableModuleNames) {
        loadedModuleNames.insert(NormalizePcieModuleName(moduleName));
    }
    for (const auto &metadata : metadataList) {
        evaluationStatus.moduleStates.push_back({metadata.driverPath, metadata.moduleName,
            loadedModuleNames.count(NormalizePcieModuleName(metadata.moduleName)) != 0});
    }
}

// Group preferred and fallback drivers by board so a fallback can be tried if request construction fails.
PcieDriverManager::DriverCandidatesByBoard PcieDriverManager::GroupCandidates(
    std::vector<PcieDriverPolicyDecision> &decisions)
{
    DriverCandidatesByBoard driverCandidatesByBoard;
    for (auto &decision : decisions) {
        if (decision.checkPassed || decision.blockReason == PcieDriverFailureReason::LOWER_PRECISION) {
            driverCandidatesByBoard[decision.boardId].push_back(&decision);
        }
    }
    return driverCandidatesByBoard;
}

void PcieDriverManager::AddLoadFailure(const std::string &driverPath, PcieDriverLoadStatus statusCode,
    const std::string &statusMessage, PcieDriverManagerStatus &evaluationStatus, const std::string &boardId)
{
    PcieDriverLoadResult loadResult {};
    loadResult.driverIdentity.boardId = boardId;
    loadResult.driverIdentity.driverPath = driverPath;
    loadResult.statusCode = static_cast<int32_t>(statusCode);
    loadResult.statusMessage = statusMessage;
    evaluationStatus.loadResults.push_back(loadResult);
}

// Check kernel compatibility and sandbox paths for all modules to load, including dependencies.
// Record the reason and return false if any check fails.
bool PcieDriverManager::ValidateResolvedModules(const std::vector<PcieDriverResolvedModule> &resolvedModules,
    const KernelSecurityContext &securityContext, PcieDriverManagerStatus &evaluationStatus,
    const std::string &boardId) const
{
    for (const auto &resolvedModule : resolvedModules) {
        if (resolvedModule.metadata == nullptr) {
            continue;
        }
        if (!PcieDriverMatchPolicy::KabiMatches(*resolvedModule.metadata, securityContext)) {
            AddLoadFailure(resolvedModule.metadata->driverPath, PcieDriverLoadStatus::SA_KABI_MISMATCH,
                "driver KABI does not match", evaluationStatus, boardId);
            return false;
        }
        if (!PcieDriverMatchPolicy::PathWithinSandbox(resolvedModule.metadata->driverPath, config_.sandboxRoot)) {
            AddLoadFailure(resolvedModule.metadata->driverPath, PcieDriverLoadStatus::SA_PATH_ESCAPE,
                "driver path escapes sandbox", evaluationStatus, boardId);
            return false;
        }
    }
    return true;
}

void PcieDriverManager::AppendResolvedLoadRequests(const PcieDriverPolicyDecision &candidate,
    const std::vector<PcieDriverResolvedModule> &resolvedModules, std::set<std::string> &plannedModuleNames,
    std::vector<PcieDriverLoadItem> *loadRequests)
{
    for (const auto &resolvedModule : resolvedModules) {
        if (resolvedModule.metadata == nullptr ||
            !plannedModuleNames.insert(NormalizePcieModuleName(resolvedModule.metadata->moduleName)).second) {
            continue;
        }
        if (loadRequests == nullptr) {
            continue;
        }
        PcieDriverLoadItem loadRequest;
        loadRequest.driverIdentity.boardId = candidate.boardId;
        loadRequest.driverIdentity.driverPath = resolvedModule.metadata->driverPath;
        loadRequest.driverIdentity.moduleName = resolvedModule.metadata->moduleName;
        loadRequest.options = resolvedModule.options;
        loadRequests->push_back(loadRequest);
    }
}

bool PcieDriverManager::TryBuildCandidateLoadRequests(PcieDriverPolicyDecision &candidate,
    const DriverLoadRequestContext &loadRequestContext, std::set<std::string> &plannedModuleNames,
    PcieDriverManagerStatus &evaluationStatus, std::vector<PcieDriverLoadItem> *loadRequests) const
{
    candidate.checkPassed = false;
    const PcieDriverMetadata *driverMetadata =
        FindMetadataByDriverPath(loadRequestContext.metadataList, candidate.driverPath);
    if (driverMetadata == nullptr) {
        candidate.blockReason = PcieDriverFailureReason::METADATA_FAILED;
        AddLoadFailure(candidate.driverPath, PcieDriverLoadStatus::SA_METADATA_FAILED, "driver metadata unavailable",
            evaluationStatus, candidate.boardId);
        return false;
    }
    auto resolvedLoadOrder =
        loadRequestContext.moduleIndex.ResolveLoadOrder(*driverMetadata, loadRequestContext.loadedModuleNames);
    if (!resolvedLoadOrder.ok) {
        candidate.blockReason = PcieDriverFailureReason::DEPENDENCY_FAILED;
        AddLoadFailure(candidate.driverPath, PcieDriverLoadStatus::SA_DEPENDENCY_FAILED,
            "driver dependency resolution failed", evaluationStatus, candidate.boardId);
        return false;
    }
    if (!ValidateResolvedModules(resolvedLoadOrder.loadOrder, loadRequestContext.securityContext, evaluationStatus,
        candidate.boardId)) {
        candidate.blockReason = PcieDriverFailureReason::DEPENDENCY_FAILED;
        AddLoadFailure(candidate.driverPath, PcieDriverLoadStatus::SA_DEPENDENCY_FAILED,
            "driver dependency validation failed", evaluationStatus, candidate.boardId);
        return false;
    }
    AppendResolvedLoadRequests(candidate, resolvedLoadOrder.loadOrder, plannedModuleNames, loadRequests);
    candidate.checkPassed = true;
    candidate.blockReason = PcieDriverFailureReason::NONE;
    return true;
}

void PcieDriverManager::BuildLoadRequests(std::vector<PcieDriverPolicyDecision> &decisions,
    const DriverLoadRequestContext &loadRequestContext, PcieDriverManagerStatus &evaluationStatus,
    std::vector<PcieDriverLoadItem> *loadRequests) const
{
    DriverCandidatesByBoard driverCandidatesByBoard = GroupCandidates(decisions);
    std::set<std::string> plannedModuleNames;
    for (const auto &[boardId, candidates] : driverCandidatesByBoard) {
        bool boardServed = false;
        for (auto *candidate : candidates) {
            if (TryBuildCandidateLoadRequests(*candidate, loadRequestContext,
                plannedModuleNames, evaluationStatus, loadRequests)) {
                boardServed = true;
                break;
            }
        }
        if (!boardServed) {
            EDM_LOGE(MODULE_SERVICE, "PcieDriverManager: NO_CANDIDATE board=%{public}s candidates=%{public}zu",
                boardId.c_str(), candidates.size());
        }
    }
}

/* ── RunEvaluation ──────────────────────────────────────────────────── */
PcieDriverManagerStatus PcieDriverManager::RunEvaluation(const std::vector<PcieDeviceInfo> &devices,
    const std::vector<PcieDriverRecord> &driverRecords, std::vector<PcieDriverLoadItem> *loadRequests)
{
    PcieDriverManagerStatus evaluationStatus;
    evaluationStatus.evaluationSucceeded = true;
    // Reloads configuration through the supplied loader; without it, keeps the supplied configuration.
    if (driverManagerDependencies_.modprobeConfigLoader) {
        auto modprobeConfig = driverManagerDependencies_.modprobeConfigLoader(config_.sandboxRoot);
        if (!modprobeConfig.loadSucceeded) {
            EDM_LOGE(MODULE_SERVICE, "RunEvaluation: modprobe directory scan failed");
            evaluationStatus.evaluationSucceeded = false;
            evaluationStatus.statusMessage = "modprobe_scan_failed";
            return evaluationStatus;
        }
        driverManagerDependencies_.modprobeConfig = std::move(modprobeConfig);
    }

    std::vector<PcieDriverCandidate> driverCandidates;
    ParseMetadata(driverRecords, evaluationStatus, driverCandidates);
    std::vector<PcieDriverMetadata> metadataList = BuildMetadataList(driverCandidates);
    KernelSecurityContext securityContext = BuildSecurityContext();
    PcieDriverModuleIndex moduleIndex;
    moduleIndex.BuildDriverModuleIndex(metadataList, driverManagerDependencies_.modprobeConfig);

    auto availableModuleNames = GetLoadedModuleNames(&evaluationStatus.loadedModulesReadable);
    UpdateModuleStates(availableModuleNames, metadataList, evaluationStatus);
    auto builtinModuleNames = GetBuiltinModuleNames(config_.builtinModulesPath);
    availableModuleNames.insert(availableModuleNames.end(), builtinModuleNames.begin(), builtinModuleNames.end());
    auto dependencySnapshot = moduleIndex.BuildAvailableModuleSnapshot(availableModuleNames);

    PcieDriverPolicyInput policyInput;
    policyInput.boardRecords = devices;
    policyInput.driverCandidates = driverCandidates;
    policyInput.securityContext = securityContext;
    policyInput.sandboxRoot = config_.sandboxRoot;
    policyInput.loadedModules = availableModuleNames;
    policyInput.dependencySnapshot = dependencySnapshot;

    std::vector<PcieDriverPolicyDecision> decisions = driverManagerDependencies_.policyEvaluator(policyInput);
    const DriverLoadRequestContext loadRequestContext {
        metadataList, moduleIndex, availableModuleNames, securityContext};
    if (!evaluationStatus.loadedModulesReadable) {
        evaluationStatus.evaluationSucceeded = false;
        evaluationStatus.statusMessage = "loaded_modules_unavailable";
        BuildLoadRequests(decisions, loadRequestContext, evaluationStatus, nullptr);
    } else {
        BuildLoadRequests(decisions, loadRequestContext, evaluationStatus, loadRequests);
    }
    evaluationStatus.decisions = std::move(decisions);
    return evaluationStatus;
}

/* ── Public methods ──────────────────────────────────────────────────── */

PcieDriverMatchResult PcieDriverManager::MatchBoardDrivers(const std::vector<PcieDeviceInfo> &devices)
{
    std::lock_guard<std::mutex> evaluationGuard(evaluationMutex_);
    PcieDriverMatchResult matchResult;
    PcieDriverManagerStatus evaluationStatus;
    // Both callbacks must be supplied by the caller; there is no built-in fallback.
    if (!driverManagerDependencies_.parser || !driverManagerDependencies_.policyEvaluator) {
        matchResult.statusMessage = "dependency_missing";
        return matchResult;
    }

    PcieDriverSandboxSnapshot sandboxInventory;
    if (!ScanSandbox(config_.sandboxRoot, sandboxInventory)) {
        EDM_LOGE(MODULE_SERVICE, "MatchBoardDrivers: sandbox scan failed");
        matchResult.statusMessage = "sandbox_scan_failed";
        return matchResult;
    }

    matchResult.driverRecords = std::move(sandboxInventory.driverRecords);
    evaluationStatus = RunEvaluation(devices, matchResult.driverRecords, &matchResult.loadRequests);

    matchResult.matchingSucceeded = evaluationStatus.evaluationSucceeded;
    matchResult.statusMessage = evaluationStatus.statusMessage;
    matchResult.loadResults = std::move(evaluationStatus.loadResults);
    matchResult.decisions = std::move(evaluationStatus.decisions);
    matchResult.moduleStates = std::move(evaluationStatus.moduleStates);
    matchResult.loadedModulesReadable = evaluationStatus.loadedModulesReadable;
    return matchResult;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
