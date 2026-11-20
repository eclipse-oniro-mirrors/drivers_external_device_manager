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

#ifndef OHOS_EDM_PCIE_DRIVER_MANAGER_H
#define OHOS_EDM_PCIE_DRIVER_MANAGER_H

#include "bus_extension/pcie/common/pcie_driver_types.h"
#include "bus_extension/pcie/modprobe/pcie_driver_module_index.h"
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace OHOS {
namespace ExternalDeviceManager {

// Allows tests to inject parsing, evaluation, and configuration loading implementations.
struct PcieDriverManagerDeps {
    // Parses metadata from a single driver file.
    std::function<PcieDriverMetadataResult(const PcieDriverRecord&)> parser;
    // Evaluates matches using board information, driver metadata, and system state.
    std::function<std::vector<PcieDriverPolicyDecision>(const PcieDriverPolicyInput&)> policyEvaluator;
    // Directly supplied modprobe configuration, typically used by tests.
    PcieDriverModprobeConfig                    modprobeConfig;
    // Loads modprobe configuration from the driver sandbox directory.
    std::function<PcieDriverModprobeConfig(const std::string &)> modprobeConfigLoader;
};

/* ── Manager orchestrator ────────────────────────────────────────────── */

class PcieDriverManager : public IPcieDriverManager {
public:
    // Accepts parsing and evaluation implementations and PCIe configuration for production or tests.
    PcieDriverManager(PcieDriverManagerDeps driverManagerDependencies, PcieDriverManagerConfig config);
    ~PcieDriverManager() override;

    PcieDriverManager(const PcieDriverManager &) = delete;
    PcieDriverManager &operator=(const PcieDriverManager &) = delete;

    PcieDriverMatchResult   MatchBoardDrivers(
        const std::vector<PcieDeviceInfo>& devices) override;

private:
    struct DriverLoadRequestContext {
        const std::vector<PcieDriverMetadata> &metadataList;
        const PcieDriverModuleIndex &moduleIndex;
        const std::vector<std::string> &loadedModuleNames;
        const KernelSecurityContext &securityContext;
    };

    using DriverCandidatesByBoard = std::map<std::string, std::vector<PcieDriverPolicyDecision *>>;

    PcieDriverManagerStatus RunEvaluation(
        const std::vector<PcieDeviceInfo>& devices,
        const std::vector<PcieDriverRecord>&    driverRecords,
        std::vector<PcieDriverLoadItem>* loadRequests = nullptr);

    static bool ScanSandbox(const std::string& sandboxRoot, PcieDriverSandboxSnapshot& snapshot);
    KernelSecurityContext BuildSecurityContext();
    std::vector<std::string> GetLoadedModuleNames(bool *readSucceeded = nullptr) const;
    std::set<std::string> GetBuiltinModuleNames(const std::string& builtinModulesPath) const;
    void ParseMetadata(const std::vector<PcieDriverRecord>& driverRecords,
        PcieDriverManagerStatus& evaluationStatus, std::vector<PcieDriverCandidate>& driverCandidates) const;
    void MergeModprobeAliases(PcieDriverMetadata& driverMetadata) const;
    std::vector<PcieDriverMetadata> BuildMetadataList(
        std::vector<PcieDriverCandidate>& driverCandidates) const;
    void UpdateModuleStates(const std::vector<std::string>& availableModuleNames,
        const std::vector<PcieDriverMetadata>& metadataList, PcieDriverManagerStatus& evaluationStatus) const;
    static DriverCandidatesByBoard GroupCandidates(std::vector<PcieDriverPolicyDecision> &decisions);
    static void AddLoadFailure(const std::string& driverPath, PcieDriverLoadStatus statusCode,
        const std::string& statusMessage, PcieDriverManagerStatus& evaluationStatus,
        const std::string& boardId = "");
    bool ValidateResolvedModules(const std::vector<PcieDriverResolvedModule>& resolvedModules,
        const KernelSecurityContext& securityContext, PcieDriverManagerStatus& evaluationStatus,
        const std::string& boardId) const;
    static void AppendResolvedLoadRequests(const PcieDriverPolicyDecision& candidate,
        const std::vector<PcieDriverResolvedModule>& resolvedModules, std::set<std::string>& plannedModuleNames,
        std::vector<PcieDriverLoadItem>* loadRequests);
    bool TryBuildCandidateLoadRequests(PcieDriverPolicyDecision &candidate,
        const DriverLoadRequestContext &loadRequestContext, std::set<std::string> &plannedModuleNames,
        PcieDriverManagerStatus &evaluationStatus, std::vector<PcieDriverLoadItem> *loadRequests) const;
    void BuildLoadRequests(std::vector<PcieDriverPolicyDecision> &decisions,
        const DriverLoadRequestContext &loadRequestContext, PcieDriverManagerStatus &evaluationStatus,
        std::vector<PcieDriverLoadItem> *loadRequests) const;
    const PcieDriverMetadata* FindMetadataByDriverPath(
        const std::vector<PcieDriverMetadata>& metadataList,
        const std::string& driverPath) const;

    // Retains the supplied callbacks and configuration for later matching rounds.
    PcieDriverManagerDeps    driverManagerDependencies_;
    PcieDriverManagerConfig  config_;
    std::mutex evaluationMutex_;
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif
