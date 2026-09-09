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

#ifndef OHOS_EDM_PCIE_DRIVER_MODULE_INDEX_H
#define OHOS_EDM_PCIE_DRIVER_MODULE_INDEX_H

#include "bus_extension/pcie/common/pcie_driver_types.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace OHOS {
namespace ExternalDeviceManager {

/* ── modprobe.d configuration ────────────────────────────────────────── */
struct PcieDriverModprobeConfig {
    std::set<std::string>                         denylistedModules;
    std::map<std::string, std::string>            moduleOptions;
    std::map<std::string, std::vector<std::string>> aliasPatternsByModule;
    std::map<std::string, std::vector<std::string>> softPreDeps;
    std::map<std::string, std::vector<std::string>> softPostDeps;
    bool loadSucceeded = true;
};

struct PcieDriverResolvedModule {
    const PcieDriverMetadata* metadata = nullptr;
    std::string           options;
};

struct PcieDriverResolveResult {
    bool                              ok = false;
    bool                              denylisted = false;
    PcieDriverFailureReason               failReason = PcieDriverFailureReason::NONE;
    std::vector<PcieDriverResolvedModule> loadOrder;
};

class PcieDriverModuleIndex {
public:
    void BuildDriverModuleIndex(const std::vector<PcieDriverMetadata>& metadataList,
        const PcieDriverModprobeConfig& modprobeConfig = {});

    PcieDriverResolveResult ResolveLoadOrder(const PcieDriverMetadata& driverMetadata,
        const std::vector<std::string>& loadedModuleNames,
        const std::set<std::string>& managerLoadedModuleNames = {}) const;

    std::vector<std::string> BuildAvailableModuleSnapshot(const std::vector<std::string>& loadedModules) const;

    std::string GetOptions(const std::string& moduleName) const;
    bool        IsDenylisted(const std::string& moduleName) const;

private:
    struct LoadedModuleSnapshot {
        const std::vector<std::string> &loadedModules;
        const std::set<std::string> &managerLoaded;
    };

    bool ResolveDfs(const std::string &moduleName, const LoadedModuleSnapshot &loadedModuleSnapshot,
        std::map<std::string, int32_t> &visitStateByModule, PcieDriverResolveResult &resolveResult,
        const PcieDriverMetadata *preferredMetadata = nullptr) const;

    bool ResolveDependencies(const std::vector<std::string> &dependencies,
        const LoadedModuleSnapshot &loadedModuleSnapshot, std::map<std::string, int32_t> &visitStateByModule,
        PcieDriverResolveResult &resolveResult) const;

    void ResolveOptionalDependencies(const std::map<std::string, std::vector<std::string>> &configuredDependencies,
        const std::string &moduleName, const LoadedModuleSnapshot &loadedModuleSnapshot,
        std::map<std::string, int32_t> &visitStateByModule,
        PcieDriverResolveResult &resolveResult) const;

    bool IsLoaded(const std::string& moduleName, const std::vector<std::string>& loadedModules,
        const std::set<std::string>& managerLoaded) const;

    std::map<std::string, std::vector<const PcieDriverMetadata*>> moduleIndex_;
    PcieDriverModprobeConfig                                       modprobeConfig_;
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif
