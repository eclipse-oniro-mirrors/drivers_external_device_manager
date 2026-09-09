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

#include "bus_extension/pcie/modprobe/pcie_driver_module_index.h"

#include <algorithm>

namespace OHOS {
namespace ExternalDeviceManager {

namespace {

enum class ModuleVisitState : int32_t {
    UNVISITED = 0,
    VISITING = 1,
    RESOLVED = 2,
};

} // namespace

void PcieDriverModuleIndex::BuildDriverModuleIndex(
    const std::vector<PcieDriverMetadata> &metadataList, const PcieDriverModprobeConfig &modprobeConfig)
{
    moduleIndex_.clear();
    modprobeConfig_ = {};
    for (const auto &moduleName : modprobeConfig.denylistedModules) {
        modprobeConfig_.denylistedModules.insert(NormalizePcieModuleName(moduleName));
    }
    for (const auto &[moduleName, options] : modprobeConfig.moduleOptions) {
        modprobeConfig_.moduleOptions[NormalizePcieModuleName(moduleName)] = options;
    }
    for (const auto &[moduleName, dependencies] : modprobeConfig.softPreDeps) {
        auto &normalizedDependencies = modprobeConfig_.softPreDeps[NormalizePcieModuleName(moduleName)];
        for (const auto &dependency : dependencies) {
            normalizedDependencies.push_back(NormalizePcieModuleName(dependency));
        }
    }
    for (const auto &[moduleName, dependencies] : modprobeConfig.softPostDeps) {
        auto &normalizedDependencies = modprobeConfig_.softPostDeps[NormalizePcieModuleName(moduleName)];
        for (const auto &dependency : dependencies) {
            normalizedDependencies.push_back(NormalizePcieModuleName(dependency));
        }
    }
    for (const auto &metadata : metadataList) {
        if (metadata.moduleName.empty()) {
            continue;
        }
        if (IsDenylisted(metadata.moduleName)) {
            continue;
        }
        moduleIndex_[NormalizePcieModuleName(metadata.moduleName)].push_back(&metadata);
    }
}

bool PcieDriverModuleIndex::IsDenylisted(const std::string &moduleName) const
{
    return modprobeConfig_.denylistedModules.count(NormalizePcieModuleName(moduleName)) > 0;
}

std::string PcieDriverModuleIndex::GetOptions(const std::string &moduleName) const
{
    auto it = modprobeConfig_.moduleOptions.find(NormalizePcieModuleName(moduleName));
    return it != modprobeConfig_.moduleOptions.end() ? it->second : "";
}

// Resolve module load order recursively, skipping loaded modules and checking cycles and the denylist.
bool PcieDriverModuleIndex::ResolveDfs(const std::string &moduleName, const LoadedModuleSnapshot &loadedModuleSnapshot,
    std::map<std::string, int32_t> &visitStateByModule, PcieDriverResolveResult &resolveResult,
    const PcieDriverMetadata *preferredMetadata) const
{
    if (moduleName.empty() ||
        IsLoaded(moduleName, loadedModuleSnapshot.loadedModules, loadedModuleSnapshot.managerLoaded)) {
        return true;
    }
    if (IsDenylisted(moduleName)) {
        resolveResult.denylisted = true;
        return false;
    }

    const std::string normalizedModuleName = NormalizePcieModuleName(moduleName);
    auto moduleEntry = moduleIndex_.find(normalizedModuleName);
    if (moduleEntry == moduleIndex_.end() || moduleEntry->second.empty()) {
        resolveResult.failReason = PcieDriverFailureReason::DEPENDENCY_FAILED;
        return false;
    }

    int32_t &visitState = visitStateByModule[normalizedModuleName];
    if (visitState == static_cast<int32_t>(ModuleVisitState::VISITING)) {
        resolveResult.failReason = PcieDriverFailureReason::DEPENDENCY_FAILED;
        return false;
    }
    if (visitState == static_cast<int32_t>(ModuleVisitState::RESOLVED)) {
        return true;
    }
    visitState = static_cast<int32_t>(ModuleVisitState::VISITING);

    /* softpre dependencies are optional and must not block the main module. */
    ResolveOptionalDependencies(modprobeConfig_.softPreDeps, moduleName, loadedModuleSnapshot,
        visitStateByModule, resolveResult);

    /* hard depends */
    const auto &dependencies = preferredMetadata ? preferredMetadata->depends : moduleEntry->second[0]->depends;
    if (!ResolveDependencies(dependencies, loadedModuleSnapshot, visitStateByModule, resolveResult)) {
        visitState = static_cast<int32_t>(ModuleVisitState::UNVISITED);
        return false;
    }

    visitState = static_cast<int32_t>(ModuleVisitState::RESOLVED);

    const PcieDriverMetadata *metadata = preferredMetadata ? preferredMetadata : moduleEntry->second[0];
    resolveResult.loadOrder.push_back({metadata, GetOptions(moduleName)});

    /* softpost dependencies are optional and must not block the main module. */
    ResolveOptionalDependencies(modprobeConfig_.softPostDeps, moduleName, loadedModuleSnapshot,
        visitStateByModule, resolveResult);

    return true;
}

// Resolve each dependency's load order and return false if any dependency cannot be resolved.
bool PcieDriverModuleIndex::ResolveDependencies(const std::vector<std::string> &dependencies,
    const LoadedModuleSnapshot &loadedModuleSnapshot, std::map<std::string, int32_t> &visitStateByModule,
    PcieDriverResolveResult &resolveResult) const
{
    for (const auto &moduleName : dependencies) {
        if (!ResolveDfs(moduleName, loadedModuleSnapshot, visitStateByModule, resolveResult)) {
            return false;
        }
    }
    return true;
}

// Find the module's pre- or post-dependencies in the modprobe configuration and resolve their load order.
void PcieDriverModuleIndex::ResolveOptionalDependencies(
    const std::map<std::string, std::vector<std::string>> &configuredDependencies,
    const std::string &moduleName, const LoadedModuleSnapshot &loadedModuleSnapshot,
    std::map<std::string, int32_t> &visitStateByModule,
    PcieDriverResolveResult &resolveResult) const
{
    auto dependencies = configuredDependencies.find(NormalizePcieModuleName(moduleName));
    if (dependencies == configuredDependencies.end()) {
        return;
    }
    for (const auto &dependency : dependencies->second) {
        const auto normalizedDependency = NormalizePcieModuleName(dependency);
        const bool alreadyResolved = std::any_of(resolveResult.loadOrder.begin(), resolveResult.loadOrder.end(),
            [&normalizedDependency](const PcieDriverResolvedModule &resolvedModule) {
                return resolvedModule.metadata != nullptr &&
                    NormalizePcieModuleName(resolvedModule.metadata->moduleName) == normalizedDependency;
            });
        if (alreadyResolved) {
            continue;
        }
        PcieDriverResolveResult optionalResult;
        if (!ResolveDfs(dependency, loadedModuleSnapshot, visitStateByModule, optionalResult)) {
            continue;
        }
        resolveResult.loadOrder.insert(resolveResult.loadOrder.end(), optionalResult.loadOrder.begin(),
            optionalResult.loadOrder.end());
    }
}

// Resolve load order for the driver and its dependencies, skipping loaded and built-in modules.
PcieDriverResolveResult PcieDriverModuleIndex::ResolveLoadOrder(const PcieDriverMetadata &driverMetadata,
    const std::vector<std::string> &loadedModuleNames, const std::set<std::string> &managerLoadedModuleNames) const
{
    PcieDriverResolveResult resolveResult;
    if (driverMetadata.moduleName.empty()) {
        resolveResult.failReason = PcieDriverFailureReason::METADATA_FAILED;
        return resolveResult;
    }
    if (IsDenylisted(driverMetadata.moduleName)) {
        resolveResult.denylisted = true;
        resolveResult.failReason = PcieDriverFailureReason::POLICY_REJECTED;
        return resolveResult;
    }

    std::map<std::string, int32_t> visitStateByModule;
    const LoadedModuleSnapshot loadedModuleSnapshot {loadedModuleNames, managerLoadedModuleNames};
    resolveResult.ok =
        ResolveDfs(driverMetadata.moduleName, loadedModuleSnapshot, visitStateByModule, resolveResult, &driverMetadata);
    if (!resolveResult.ok && resolveResult.failReason == PcieDriverFailureReason::NONE) {
        resolveResult.failReason = PcieDriverFailureReason::DEPENDENCY_FAILED;
    }
    return resolveResult;
}

// Merge and deduplicate loaded, built-in and sandbox module names for subsequent dependency checks.
std::vector<std::string> PcieDriverModuleIndex::BuildAvailableModuleSnapshot(
    const std::vector<std::string> &loadedModules) const
{
    std::vector<std::string> availableModules;
    for (const auto &moduleName : loadedModules) {
        const auto normalizedModuleName = NormalizePcieModuleName(moduleName);
        if (std::find(availableModules.begin(), availableModules.end(), normalizedModuleName) ==
            availableModules.end()) {
            availableModules.push_back(normalizedModuleName);
        }
    }
    for (const auto &moduleEntry : moduleIndex_) {
        const std::string &moduleName = moduleEntry.first;
        if (std::find(availableModules.begin(), availableModules.end(), moduleName) == availableModules.end()) {
            availableModules.push_back(moduleName);
        }
    }
    return availableModules;
}

// Normalize the module name and check the loaded, built-in and manager-loaded lists to avoid loading it again.
bool PcieDriverModuleIndex::IsLoaded(const std::string &moduleName, const std::vector<std::string> &loadedModules,
    const std::set<std::string> &managerLoaded) const
{
    const auto matchesModule = [&moduleName](const std::string &loadedModuleName) {
        return NormalizePcieModuleName(loadedModuleName) == NormalizePcieModuleName(moduleName);
    };
    return std::any_of(loadedModules.begin(), loadedModules.end(), matchesModule) ||
        std::any_of(managerLoaded.begin(), managerLoaded.end(), matchesModule);
}

} // namespace ExternalDeviceManager
} // namespace OHOS
