/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "driver_ext_mgr.h"

#include "bus_extension_core.h"
#include "dev_change_callback.h"
#include "driver_extension_controller.h"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "etx_device_mgr.h"
#include "event_config.h"
#include "ext_permission_manager.h"
#include "hilog_wrapper.h"
#include "idriver_change_callback.h"
#include "iservice_registry.h"
#include "notification_peripheral.h"
#include "system_ability_definition.h"
#include "usb_device_info.h"
#include "usb_driver_info.h"
#ifdef ENABLE_PCIE
#include "bus_extension/pcie/manager/pcie_driver_manager.h"
#include "bus_extension/pcie/metadata/pcie_driver_metadata_parser.h"
#include "bus_extension/pcie/policy/pcie_driver_match_policy.h"
#include "bus_extension/pcie/system/security_query.h"
#include "hdf_base.h"
#include "pcie_driver_report_sys_event.h"
#include "v1_0/ipcie_interface.h"
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cJSON.h>
#include <chrono>
#include <thread>
#include <utility>
#endif
#ifdef ENABLE_APP_LAUNCH_MANAGER
#include "app_launch_manager.h"
#endif

namespace OHOS {
namespace ExternalDeviceManager {

const bool G_REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DriverExtMgr>::GetInstance().get());
static const std::string PERMISSION_NAME = "ohos.permission.ACCESS_EXTENSIONAL_DEVICE_DRIVER";
static const std::string ACCESS_DDK_DRIVERS_PERMISSION = "ohos.permission.ACCESS_DDK_DRIVERS";

#ifdef ENABLE_PCIE
/* ── PCIe driver reconcile ─────────────────────────────────────────────── */
namespace {

constexpr size_t CONFIG_TYPE_WORD_INDEX = 0;
constexpr size_t MODULE_NAME_WORD_INDEX = 1;
constexpr size_t CONFIG_VALUE_START_WORD_INDEX = 2;
constexpr size_t ALIAS_MODULE_NAME_WORD_INDEX = 2;
constexpr size_t ALIAS_MIN_WORD_COUNT = 3;
constexpr size_t OPTIONS_MIN_WORD_COUNT = 3;
constexpr size_t SOFTDEP_MIN_WORD_COUNT = 2;
constexpr size_t DENYLIST_MIN_WORD_COUNT = 2;
constexpr size_t MAX_MODPROBE_FILE_SIZE = 1024 * 1024;
constexpr size_t MAX_MODPROBE_LINE_SIZE = 4 * 1024;
constexpr char MODPROBE_DENYLIST_COMMAND[] = "black"
                                             "list";

bool QueryCurrentPcieHostDevices(std::vector<PcieDeviceInfo> &devices)
{
    using OHOS::HDI::Pcie::V1_0::IPcieInterface;
    auto pcieInterface = IPcieInterface::Get();
    if (!pcieInterface) {
        EDM_LOGE(MODULE_SERVICE, "QueryCurrentPcieHostDevices: IPcieInterface::Get() returned null");
        return false;
    }
    std::vector<OHOS::HDI::Pcie::V1_0::PcieDeviceInfo> hdiDevices;
    int32_t queryStatus = pcieInterface->GetPcieDevices(hdiDevices);
    if (queryStatus != HDF_SUCCESS) {
        EDM_LOGE(MODULE_SERVICE, "QueryCurrentPcieHostDevices: GetPcieDevices failed ret=%{public}d", queryStatus);
        return false;
    }
    devices.clear();
    devices.reserve(hdiDevices.size());
    for (const auto &hdiDevice : hdiDevices) {
        PcieDeviceInfo device;
        device.boardId = hdiDevice.boardId;
        device.sysfsPath = hdiDevice.sysfsPath;
        device.vendorId = hdiDevice.vendorId;
        device.deviceId = hdiDevice.deviceId;
        device.classCode = hdiDevice.classCode;
        device.modalias = hdiDevice.modalias;
        device.isModaliasValid = hdiDevice.isModaliasValid;
        device.boundDriver = hdiDevice.boundDriver;
        devices.push_back(std::move(device));
    }
    return true;
}

bool LoadPcieDrivers(const std::vector<PcieDriverLoadItem> &loadRequests,
    std::vector<OHOS::HDI::Pcie::V1_0::PcieDriverLoadResult> &hdiLoadResults)
{
    using OHOS::HDI::Pcie::V1_0::IPcieInterface;
    auto pcieInterface = IPcieInterface::Get();
    if (!pcieInterface) {
        EDM_LOGI(MODULE_SERVICE, "RunPcieDriverReconcile: IPcieInterface::Get() null for execute");
        return false;
    }
    int32_t loadStatus = pcieInterface->LoadPcieDrivers(loadRequests, hdiLoadResults);
    if (loadStatus != HDF_SUCCESS) {
        EDM_LOGE(MODULE_SERVICE, "RunPcieDriverReconcile: HDI LoadPcieDrivers failed ret=%{public}d", loadStatus);
    } else {
        EDM_LOGI(MODULE_SERVICE, "RunPcieDriverReconcile: HDI LoadPcieDrivers succeeded");
    }
    for (const auto &hdiLoadResult : hdiLoadResults) {
        if (hdiLoadResult.statusCode != HDF_SUCCESS) {
            EDM_LOGE(MODULE_SERVICE,
                "RunPcieDriverReconcile: LOAD_FAIL board=%{public}s "
                "module=%{public}s statusCode=%{public}d message=%{public}s",
                hdiLoadResult.driverIdentity.boardId.c_str(),
                hdiLoadResult.driverIdentity.moduleName.c_str(), hdiLoadResult.statusCode,
                hdiLoadResult.statusMessage.c_str());
        }
    }
    EDM_LOGI(MODULE_SERVICE, "RunPcieDriverReconcile: execute done results=%{public}zu", hdiLoadResults.size());
    return loadStatus == HDF_SUCCESS || !hdiLoadResults.empty();
}

void RunPcieDriverReconcile(
    const std::shared_ptr<IPcieDriverManager> &pcieDriverManager, const std::vector<PcieDeviceInfo> &devices)
{
    if (!pcieDriverManager) {
        EDM_LOGE(MODULE_SERVICE, "RunPcieDriverReconcile: PCIe driver manager is null");
        return;
    }
    if (devices.empty()) {
        EDM_LOGI(MODULE_SERVICE, "RunPcieDriverReconcile: no devices found, skipping driver matching");
        return;
    }
    auto matchResult = pcieDriverManager->MatchBoardDrivers(devices);
    auto loadResults = std::move(matchResult.loadResults);
    if (!matchResult.matchingSucceeded) {
        EDM_LOGI(MODULE_SERVICE,
            "RunPcieDriverReconcile: matchingSucceeded=%{public}d loadRequests=%{public}zu -> skip",
            matchResult.matchingSucceeded, matchResult.loadRequests.size());
        return;
    }
    if (matchResult.loadRequests.empty()) {
        EDM_LOGI(MODULE_SERVICE, "RunPcieDriverReconcile: no load requests, reporting decisions");
        PcieDriverReportSysEvent::ReportReconcileRound(matchResult.decisions, loadResults);
        return;
    }

    std::vector<OHOS::HDI::Pcie::V1_0::PcieDriverLoadResult> hdiLoadResults;
    if (!LoadPcieDrivers(matchResult.loadRequests, hdiLoadResults)) {
        PcieDriverReportSysEvent::ReportReconcileRound({}, loadResults);
        return;
    }
    loadResults.insert(loadResults.end(), hdiLoadResults.begin(), hdiLoadResults.end());
    PcieDriverReportSysEvent::ReportReconcileRound(matchResult.decisions, loadResults);
}

std::string PcieDriverFileName(const std::string &driverPath)
{
    size_t slashPosition = driverPath.find_last_of('/');
    return slashPosition == std::string::npos ? driverPath : driverPath.substr(slashPosition + 1);
}

void DumpPcieBoardRecords(int fd, const std::vector<PcieDeviceInfo> &devices)
{
    dprintf(fd, "\n========== Board Records (%zu devices) ==========\n", devices.size());
    for (const auto &board : devices) {
        dprintf(fd, "  board=%s  modalias=%s  boundDriver=%s\n", board.boardId.c_str(), board.modalias.c_str(),
            board.boundDriver.empty() ? "(none)" : board.boundDriver.c_str());
    }
}

void DumpPcieDriverRecords(int fd, const PcieDriverMatchResult &matchResult)
{
    dprintf(fd, "\n========== Driver Records (%zu drivers) ==========\n", matchResult.driverRecords.size());
    for (const auto &driver : matchResult.driverRecords) {
        dprintf(fd, "  driver=%s  size=%llu  mtime=%llu\n", PcieDriverFileName(driver.driverPath).c_str(),
            static_cast<unsigned long long>(driver.fileSize), static_cast<unsigned long long>(driver.mtime));
    }
}

void DumpPcieMatchResults(int fd, const PcieDriverMatchResult &matchResult)
{
    dprintf(fd, "\n========== Match Results ==========\n");
    bool anyMatched = false;
    for (const auto &decision : matchResult.decisions) {
        if (!decision.checkPassed) {
            continue;
        }
        anyMatched = true;
        dprintf(fd, "  board=%s  ->  driver=%s  score=%d\n", decision.boardId.c_str(),
            PcieDriverFileName(decision.driverPath).c_str(), decision.aliasScore);
    }
    if (!anyMatched) {
        dprintf(fd, "  (no board matched any driver)\n");
    }
}

const char *PcieGateStatusToString(bool evaluated, bool passed)
{
    if (!evaluated) {
        return "NOT_RUN";
    }
    return passed ? "PASS" : "BLOCK";
}

void DumpPcieGateDetails(int fd, const PcieDriverMatchResult &matchResult)
{
    dprintf(fd, "\n========== Gate Details (%zu decisions) ==========\n", matchResult.decisions.size());
    for (const auto &decision : matchResult.decisions) {
        dprintf(fd, "  --- board=%s  driver=%s ---\n", decision.boardId.c_str(),
            PcieDriverFileName(decision.driverPath).c_str());
        const bool dslPassed = decision.dslAllowed;
        const bool dslEvaluated = dslPassed || decision.blockReason == PcieDriverFailureReason::DSL_DENIED;
        const bool aliasPassed = dslPassed && !decision.aliasMatched.empty();
        const bool aliasEvaluated = aliasPassed || decision.blockReason == PcieDriverFailureReason::ALIAS_NOT_MATCHED;
        const bool kernelVersionPassed = aliasPassed && (decision.kabiMatched || decision.vermagicMatched);
        const bool dependenciesPassed = kernelVersionPassed && decision.dependencyMet;
        dprintf(fd, "    gate1: dsl=%d  -> %s\n", decision.dslAllowed, PcieGateStatusToString(dslEvaluated, dslPassed));
        dprintf(fd, "    gate2: alias=%s  score=%d  -> %s\n", decision.aliasMatched.c_str(), decision.aliasScore,
            PcieGateStatusToString(aliasEvaluated, aliasPassed));
        dprintf(fd, "    gate3: kabi=%d vermagic=%d  -> %s\n", decision.kabiMatched, decision.vermagicMatched,
            PcieGateStatusToString(aliasPassed, kernelVersionPassed));
        dprintf(fd, "    gate4: deps=%d  -> %s\n", decision.dependencyMet,
            PcieGateStatusToString(kernelVersionPassed, dependenciesPassed));
        dprintf(fd, "    gate5: -> %s\n",
            dependenciesPassed && decision.skipAlreadyLoaded ?
                "BLOCK (DUPLICATE_LOADED)" :
                PcieGateStatusToString(dependenciesPassed, !decision.skipAlreadyLoaded));
        dprintf(fd, "    gate6: sandbox path -> %s\n",
            PcieGateStatusToString(dependenciesPassed && !decision.skipAlreadyLoaded,
                decision.blockReason != PcieDriverFailureReason::PATH_ESCAPE));
        if (decision.boardHasBoundDriver) {
            dprintf(fd, "    gate0: boardHasBoundDriver=%d  reason=%s\n", decision.boardHasBoundDriver,
                PcieDriverFailureReasonToString(decision.blockReason));
        }
        dprintf(fd, "    result: %s  blockReason=%s\n", decision.checkPassed ? "ELIGIBLE" : "BLOCKED",
            PcieDriverFailureReasonToString(decision.blockReason));
    }
}

void DumpPcieModuleStates(int fd, const PcieDriverMatchResult &matchResult)
{
    dprintf(fd, "\n========== Current Module State (%zu drivers) ==========\n", matchResult.driverRecords.size());
    if (!matchResult.loadedModulesReadable) {
        dprintf(fd, "  Loaded module list unavailable; loading state is UNKNOWN.\n");
    }
    for (const auto &driver : matchResult.driverRecords) {
        const PcieDriverModuleState *moduleState = nullptr;
        for (const auto &state : matchResult.moduleStates) {
            if (state.driverPath == driver.driverPath) {
                moduleState = &state;
                break;
            }
        }
        const char *loadedState = "UNKNOWN";
        if (moduleState != nullptr && matchResult.loadedModulesReadable) {
            loadedState = moduleState->loaded ? "YES" : "NO";
        }
        dprintf(fd, "  driver=%s  module=%s  loaded=%s\n", PcieDriverFileName(driver.driverPath).c_str(),
            moduleState != nullptr ? moduleState->moduleName.c_str() : "(unavailable)", loadedState);
    }
    dprintf(fd, "  Current state only; historical load results are not retained.\n");
}

void DumpPcieEvaluationIssues(int fd, const PcieDriverMatchResult &matchResult)
{
    if (matchResult.loadResults.empty()) {
        return;
    }
    dprintf(fd, "\n========== Current Evaluation Issues (%zu issues) ==========\n", matchResult.loadResults.size());
    for (const auto &issue : matchResult.loadResults) {
        dprintf(fd, "  driver=%s  status=%s\n", PcieDriverFileName(issue.driverIdentity.driverPath).c_str(),
            PcieDriverLoadStatusToString(static_cast<PcieDriverLoadStatus>(issue.statusCode)));
    }
}

} // namespace
/* ── File reader ───────────────────────────────────────────────────── */
bool ReadFile(int fileFd, std::string &content)
{
    struct stat fileStatus;
    if (fstat(fileFd, &fileStatus) != 0 || !S_ISREG(fileStatus.st_mode) || fileStatus.st_size < 0 ||
        static_cast<uint64_t>(fileStatus.st_size) > MAX_MODPROBE_FILE_SIZE) {
        return false;
    }

    content.clear();
    content.reserve(static_cast<size_t>(fileStatus.st_size));
    char buffer[4096];
    ssize_t readSize;
    while ((readSize = read(fileFd, buffer, sizeof(buffer))) > 0) {
        if (static_cast<size_t>(readSize) > MAX_MODPROBE_FILE_SIZE - content.size()) {
            return false;
        }
        content.append(buffer, static_cast<size_t>(readSize));
    }
    return readSize == 0;
}

/* ── modprobe.d loader ─────────────────────────────────────────────── */
std::vector<std::string> SplitModprobeLine(std::string line)
{
    size_t commentPosition = line.find('#');
    if (commentPosition != std::string::npos) {
        line.erase(commentPosition);
    }

    std::vector<std::string> words;
    std::istringstream wordStream(line);
    std::string word;
    while (wordStream >> word) {
        words.push_back(word);
    }
    return words;
}

void AddModuleOptions(const std::vector<std::string> &words, PcieDriverModprobeConfig &config)
{
    std::string options;
    for (size_t wordIndex = CONFIG_VALUE_START_WORD_INDEX; wordIndex < words.size(); ++wordIndex) {
        if (!options.empty()) {
            options += " ";
        }
        options += words[wordIndex];
    }
    config.moduleOptions[NormalizePcieModuleName(words[MODULE_NAME_WORD_INDEX])] = options;
}

void AddSoftDependencies(const std::vector<std::string> &words, PcieDriverModprobeConfig &config)
{
    std::vector<std::string> *dependencyList = nullptr;
    for (size_t wordIndex = CONFIG_VALUE_START_WORD_INDEX; wordIndex < words.size(); ++wordIndex) {
        if (words[wordIndex] == "pre:") {
            dependencyList = &config.softPreDeps[NormalizePcieModuleName(words[MODULE_NAME_WORD_INDEX])];
            continue;
        }
        if (words[wordIndex] == "post:") {
            dependencyList = &config.softPostDeps[NormalizePcieModuleName(words[MODULE_NAME_WORD_INDEX])];
            continue;
        }
        if (dependencyList != nullptr) {
            dependencyList->push_back(NormalizePcieModuleName(words[wordIndex]));
        }
    }
}

void AddModprobeConfigLine(const std::vector<std::string> &words, PcieDriverModprobeConfig &config)
{
    if (words.empty()) {
        return;
    }
    // Parse module exclusion entries and add normalized module names to the denylist.
    if (words[CONFIG_TYPE_WORD_INDEX] == MODPROBE_DENYLIST_COMMAND && words.size() >= DENYLIST_MIN_WORD_COUNT) {
        config.denylistedModules.insert(NormalizePcieModuleName(words[MODULE_NAME_WORD_INDEX]));
        return;
    }
    if (words[CONFIG_TYPE_WORD_INDEX] == "alias" && words.size() >= ALIAS_MIN_WORD_COUNT) {
        config.aliasPatternsByModule[NormalizePcieModuleName(words[ALIAS_MODULE_NAME_WORD_INDEX])].push_back(
            words[MODULE_NAME_WORD_INDEX]);
        return;
    }
    if (words[CONFIG_TYPE_WORD_INDEX] == "options" && words.size() >= OPTIONS_MIN_WORD_COUNT) {
        AddModuleOptions(words, config);
        return;
    }
    if (words[CONFIG_TYPE_WORD_INDEX] == "softdep" && words.size() >= SOFTDEP_MIN_WORD_COUNT) {
        AddSoftDependencies(words, config);
    }
}

void LoadModprobeFile(int directoryFd, const std::string &fileName, PcieDriverModprobeConfig &config)
{
    int fileFd = openat(directoryFd, fileName.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
    if (fileFd < 0) {
        return;
    }

    std::string content;
    bool readSuccess = ReadFile(fileFd, content);
    close(fileFd);
    if (!readSuccess) {
        return;
    }

    std::istringstream lineStream(content);
    std::string line;
    while (std::getline(lineStream, line)) {
        if (line.size() > MAX_MODPROBE_LINE_SIZE) {
            EDM_LOGW(MODULE_SERVICE, "LoadModprobeFile: skip oversized line length=%{public}zu", line.size());
            continue;
        }
        // Strip comments, split on whitespace, then parse and store the entry according to its type.
        AddModprobeConfigLine(SplitModprobeLine(line), config);
    }
}

PcieDriverModprobeConfig LoadModprobeConfig(const std::string &sandboxRoot)
{
    PcieDriverModprobeConfig config;
    std::string configDirectory = sandboxRoot + "/modprobe.d";
    int configDirectoryFd = open(configDirectory.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY | O_NOFOLLOW);
    if (configDirectoryFd < 0) {
        return config;
    }

    DIR *directory = fdopendir(configDirectoryFd);
    if (!directory) {
        const int openError = errno;
        EDM_LOGE(MODULE_SERVICE, "LoadModprobeConfig: fdopendir failed errno=%{public}d", openError);
        close(configDirectoryFd);
        config.loadSucceeded = false;
        return config;
    }

    while (true) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (entry == nullptr) {
            const int readError = errno;
            if (readError != 0) {
                EDM_LOGE(MODULE_SERVICE, "LoadModprobeConfig: readdir failed errno=%{public}d", readError);
                closedir(directory);
                config = {};
                config.loadSucceeded = false;
                return config;
            }
            break;
        }
        std::string fileName = entry->d_name;
        if (fileName == "." || fileName == "..") {
            continue;
        }
        LoadModprobeFile(configDirectoryFd, fileName, config);
    }
    closedir(directory);
    return config;
}

/* ── Build deps ────────────────────────────────────────────────────── */
std::shared_ptr<PcieDriverManager> BuildPcieDriverManagerDeps(const PcieDriverManagerConfig &pcieDriverConfig)
{
    auto parser = std::make_shared<PcieDriverMetadataParser>();
    auto policy = std::make_shared<PcieDriverMatchPolicy>();

    // Provides the production implementations for metadata parsing, match evaluation, and configuration loading.
    PcieDriverManagerDeps driverManagerDependencies;
    // Captures the parser to keep it alive while the manager uses this callback.
    driverManagerDependencies.parser = [parser](const PcieDriverRecord &driver) {
        return parser->Parse(driver);
    };
    // Uses the actual configuration loader for each evaluation round.
    driverManagerDependencies.modprobeConfigLoader = LoadModprobeConfig;
    // Captures the production matching policy; tests may supply a different evaluator.
    driverManagerDependencies.policyEvaluator = [policy](const PcieDriverPolicyInput &policyInput) {
        return policy->Evaluate(policyInput);
    };

    auto pcieDriverManager = std::make_shared<PcieDriverManager>(driverManagerDependencies, pcieDriverConfig);
    return pcieDriverManager;
}

void DriverExtMgr::PcieDriverReconcileWorker()
{
    EDM_LOGI(MODULE_SERVICE, "ReconcileWorker: started");
    while (!pcieDriverReconcileStop_.load()) {
        std::vector<PcieDeviceInfo> devices;
        {
            std::unique_lock<std::mutex> lock(pcieDriverReconcileMutex_);
            pcieDriverReconcileCv_.wait(lock, [this] {
                return pcieDriverReconcileDirty_.load() || pcieDriverReconcileStop_.load();
            });
            if (pcieDriverReconcileStop_.load()) {
                break;
            }
            pcieDriverReconcileDirty_ = false;
            prctl(PR_SET_NAME, "SA:wake");
        }
        if (!QueryCurrentPcieHostDevices(devices)) {
            EDM_LOGE(MODULE_SERVICE, "ReconcileWorker: skip evaluation because Host query failed");
            continue;
        }
        ExtDeviceManager::GetInstance().SetPcieDevicesPresent(!devices.empty());
        RunPcieDriverReconcile(pcieDriverManager_, devices);
    }
}
#endif

DriverExtMgr::DriverExtMgr() : SystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, true) {}
DriverExtMgr::~DriverExtMgr()
{
#ifdef ENABLE_PCIE
    StopPcieDriverReconcileWorker();
#endif
}

#ifdef ENABLE_PCIE
void DriverExtMgr::StopPcieDriverReconcileWorker()
{
    {
        std::lock_guard<std::mutex> lock(pcieDriverReconcileMutex_);
        pcieDriverReconcileStop_ = true;
    }
    DriverPkgManager::GetInstance().SetBundleChangeCallback(nullptr);
    pcieDriverReconcileCv_.notify_all();
    if (pcieDriverReconcileWorker_.joinable()) {
        pcieDriverReconcileWorker_.join();
    }
}

void DriverExtMgr::StartPcieDriverManager()
{
    PcieDriverManagerConfig pcieDriverConfig;
    pcieDriverConfig.sandboxRoot = "/data/service/el1/public/pcie_driver";
    pcieDriverConfig.builtinModulesPath = "/lib/modules/" + GetKernelVersion() + "/modules.builtin";
    EDM_LOGI(MODULE_SERVICE, "OnStart: building PCIE driver manager");
    pcieDriverManager_ = BuildPcieDriverManagerDeps(pcieDriverConfig);
    if (!pcieDriverManager_) {
        EDM_LOGE(MODULE_SERVICE, "OnStart: pcieDriverManager_ is NULL");
        return;
    }
    EDM_LOGI(MODULE_SERVICE, "OnStart: creating reconcile worker");
    {
        std::lock_guard<std::mutex> lock(pcieDriverReconcileMutex_);
        pcieDriverReconcileStop_ = false;
    }
    pcieDriverReconcileWorker_ = std::thread(&DriverExtMgr::PcieDriverReconcileWorker, this);
    DriverPkgManager::GetInstance().SetBundleChangeCallback([this] {
        {
            std::lock_guard<std::mutex> lock(pcieDriverReconcileMutex_);
            pcieDriverReconcileDirty_ = true;
        }
        pcieDriverReconcileCv_.notify_one();
    });
    EDM_LOGI(MODULE_SERVICE, "OnStart: kick-off initial reconcile");
    {
        std::lock_guard<std::mutex> lock(pcieDriverReconcileMutex_);
        pcieDriverReconcileDirty_ = true;
    }
    pcieDriverReconcileCv_.notify_one();
}
#endif

void DriverExtMgr::OnStart()
{
    int32_t ret;
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStart");
#ifdef ENABLE_APP_LAUNCH_MANAGER
    AppLaunchManager::GetInstance().LoadConfig();
#endif
    BusExtensionCore::GetInstance().LoadBusExtensionLibs();
    std::shared_ptr<IDriverChangeCallback> driverChangeCallback =
        BusExtensionCore::GetInstance().AcquireDriverChangeCallback(BUS_TYPE_USB);
    ExtDeviceManager::GetInstance().SetDriverChangeCallback(driverChangeCallback);
    ret = DriverPkgManager::GetInstance().Init(bmsFuture_, accountFuture_, commEventFuture_);
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
    AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "DriverPkgManager Init failed %{public}d", ret);
    }
    ret = ExtDeviceManager::GetInstance().Init();
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "ExtDeviceManager Init failed %{public}d", ret);
    }
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    ret = BusExtensionCore::GetInstance().Init(callback);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_SERVICE, "BusExtensionCore Init failed %{public}d", ret);
    }

#ifdef ENABLE_PCIE
    StartPcieDriverManager();
#endif

    if (!Publish(AsObject())) {
        EDM_LOGE(MODULE_DEV_MGR, "OnStart register to system ability manager failed.");
#ifdef ENABLE_PCIE
        StopPcieDriverReconcileWorker();
#endif
        return;
    }
    eventConfig_ = EventConfig::GetInstance();
    if (!eventConfig_.ParseJsonFile()) {
        EDM_LOGE(MODULE_SERVICE, "ParseJsonFile failed");
    }
}

void DriverExtMgr::OnStop()
{
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStop");
#ifdef ENABLE_PCIE
    StopPcieDriverReconcileWorker();
#endif
#ifdef ENABLE_APP_LAUNCH_MANAGER
    AppLaunchManager::GetInstance().UnInit();
#endif
}

int DriverExtMgr::Dump(int fd, const std::vector<std::u16string> &args)
{
    (void)args;
#ifdef ENABLE_PCIE
    if (!pcieDriverManager_) {
        dprintf(fd, "\n========== PCIE Driver Manager: not initialized ==========\n");
        return 0;
    }
    std::vector<PcieDeviceInfo> devices;
    bool boardsReadable = QueryCurrentPcieHostDevices(devices);
    if (boardsReadable) {
        DumpPcieBoardRecords(fd, devices);
    } else {
        dprintf(fd, "\n========== Board Records: Host query failed ==========\n");
    }
    auto matchResult = pcieDriverManager_->MatchBoardDrivers(devices);
    DumpPcieDriverRecords(fd, matchResult);
    if (!boardsReadable) {
        dprintf(fd, "\nMatch Results and Gate Details unavailable: Host query failed.\n");
    } else if (!matchResult.matchingSucceeded) {
        dprintf(fd, "\nMatch Results and Gate Details unavailable: %s.\n", matchResult.statusMessage.c_str());
    } else {
        DumpPcieMatchResults(fd, matchResult);
        DumpPcieGateDetails(fd, matchResult);
    }
    if (!matchResult.matchingSucceeded) {
        dprintf(fd, "  Driver evaluation failed: %s.\n", matchResult.statusMessage.c_str());
    }
    DumpPcieEvaluationIssues(fd, matchResult);
    DumpPcieModuleStates(fd, matchResult);
#endif
    return 0;
}

void DriverExtMgr::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility systemAbilityId: %{public}d", systemAbilityId);
    std::lock_guard<std::mutex> lock(promiseMutex_);
    switch (systemAbilityId) {
        case SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN: {
            EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility accountMgr");
            if (!accountPromiseUsed_) {
                DriverPkgManager::GetInstance().SubscribeOsAccountSwitch();
                accountPromise_.set_value(systemAbilityId);
                accountPromiseUsed_ = true;
            }
            break;
        }
        case BUNDLE_MGR_SERVICE_SYS_ABILITY_ID: {
            EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility BMS");
            if (!bmsPromiseUsed_) {
                bmsPromise_.set_value(systemAbilityId);
                bmsPromiseUsed_ = true;
            }
            break;
        }
        case COMMON_EVENT_SERVICE_ID: {
            EDM_LOGI(MODULE_SERVICE, "OnAddSystemAbility CommonEventService");
            DriverPkgManager::GetInstance().RegisterBundleStatusCallback();
#ifdef ENABLE_APP_LAUNCH_MANAGER
            AppLaunchManager::GetInstance().SubscribeCommonEvent();
#endif
            if (!cesPromiseUsed_) {
                commEventPromise_.set_value(systemAbilityId);
                cesPromiseUsed_ = true;
            }
            break;
        }
        default:
            break;
    }
}

ErrCode DriverExtMgr::QueryDevice(
    int32_t &errorCode, uint32_t busType, std::vector<std::shared_ptr<DeviceData>> &devices)
{
    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    if (busType == BusType::BUS_TYPE_INVALID) {
        EDM_LOGE(MODULE_DEV_MGR, "invalid busType:%{public}d", static_cast<int32_t>(busType));
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_INVALID_PARAM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    std::vector<std::shared_ptr<DeviceInfo>> deviceInfos =
        ExtDeviceManager::GetInstance().QueryDevice(static_cast<BusType>(busType));
    for (const auto &deviceInfo : deviceInfos) {
        switch (deviceInfo->GetBusType()) {
            case BusType::BUS_TYPE_USB: {
                std::shared_ptr<UsbDeviceInfo> usbDeviceInfo = std::static_pointer_cast<UsbDeviceInfo>(deviceInfo);
                std::shared_ptr<USBDevice> device = std::make_shared<USBDevice>();
                device->busType = usbDeviceInfo->GetBusType();
                device->deviceId = usbDeviceInfo->GetDeviceId();
                device->descripton = usbDeviceInfo->GetDeviceDescription();
                device->productId = usbDeviceInfo->GetProductId();
                device->vendorId = usbDeviceInfo->GetVendorId();
                devices.push_back(device);
                break;
            }
            default: {
                break;
            }
        }
    }
    errorCode = static_cast<int32_t>(UsbErrCode::EDM_OK);
    return errorCode;
}

ErrCode DriverExtMgr::BindDevice(
    int32_t &errorCode, uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    uint32_t callingTokenId = ExtPermissionManager::GetCallingTokenID();
    UsbErrCode ret = static_cast<UsbErrCode>(
        ExtDeviceManager::GetInstance().ConnectDevice(deviceId, callingTokenId, connectCallback));
    if (ret == UsbErrCode::EDM_OK) {
        errorCode = static_cast<int32_t>(ret);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }
    errorCode = static_cast<int32_t>(ret);
    return static_cast<int32_t>(UsbErrCode::EDM_OK);
}

ErrCode DriverExtMgr::UnBindDevice(int32_t &errorCode, uint64_t deviceId)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    uint32_t callingTokenId = ExtPermissionManager::GetCallingTokenID();
    auto ret = static_cast<UsbErrCode>(ExtDeviceManager::GetInstance().DisConnectDevice(deviceId, callingTokenId));
    errorCode = static_cast<int32_t>(ret);
    return static_cast<int32_t>(UsbErrCode::EDM_OK);
}

ErrCode DriverExtMgr::BindDriverWithDeviceId(
    int32_t &errorCode, uint64_t deviceId, const sptr<IDriverExtMgrCallback> &connectCallback)
{
    EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::VerifyPermission(ACCESS_DDK_DRIVERS_PERMISSION)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    uint32_t callingTokenId = ExtPermissionManager::GetCallingTokenID();
    unordered_set<std::string> accessibleAppIds;
    if (!ExtPermissionManager::GetPermissionValues(ACCESS_DDK_DRIVERS_PERMISSION, accessibleAppIds)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s failed to get permission value", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }
    errorCode = static_cast<int32_t>(ExtDeviceManager::GetInstance().ConnectDriverWithDeviceId(
        deviceId, callingTokenId, accessibleAppIds, connectCallback));
    return static_cast<int32_t>(UsbErrCode::EDM_OK);
}

ErrCode DriverExtMgr::UnBindDriverWithDeviceId(int32_t &errorCode, uint64_t deviceId)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::VerifyPermission(ACCESS_DDK_DRIVERS_PERMISSION)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    uint32_t callingTokenId = ExtPermissionManager::GetCallingTokenID();
    errorCode =
        static_cast<int32_t>(ExtDeviceManager::GetInstance().DisConnectDriverWithDeviceId(deviceId, callingTokenId));
    return static_cast<int32_t>(UsbErrCode::EDM_OK);
}

static std::shared_ptr<DeviceInfoData> ParseToDeviceInfoData(const std::shared_ptr<Device> &device)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (device == nullptr || device->GetDeviceInfo() == nullptr) {
        EDM_LOGD(MODULE_DEV_MGR, "device or deviceInfo is null");
        return nullptr;
    }
    auto deviceInfo = device->GetDeviceInfo();
    auto busType = deviceInfo->GetBusType();
    if (busType <= BusType::BUS_TYPE_INVALID || busType >= BusType::BUS_TYPE_MAX) {
        EDM_LOGD(MODULE_DEV_MGR, "invalid busType:%{public}u", busType);
        return nullptr;
    }
    std::shared_ptr<DeviceInfoData> tempDeviceInfo = nullptr;

    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            std::shared_ptr<UsbDeviceInfo> usbDeviceInfo = std::static_pointer_cast<UsbDeviceInfo>(deviceInfo);
            std::shared_ptr<USBDeviceInfoData> tempUsbDeviceInfo = std::make_shared<USBDeviceInfoData>();
            tempUsbDeviceInfo->productId = usbDeviceInfo->GetProductId();
            tempUsbDeviceInfo->vendorId = usbDeviceInfo->GetVendorId();
            std::transform(usbDeviceInfo->interfaceDescList_.begin(), usbDeviceInfo->interfaceDescList_.end(),
                std::back_inserter(tempUsbDeviceInfo->interfaceDescList), [](UsbInterfaceDescriptor desc) {
                    std::shared_ptr<USBInterfaceDesc> interfaceDesc = std::make_shared<USBInterfaceDesc>();
                    interfaceDesc->bInterfaceNumber = desc.bInterfaceNumber;
                    interfaceDesc->bClass = desc.bInterfaceClass;
                    interfaceDesc->bSubClass = desc.bInterfaceSubClass;
                    interfaceDesc->bProtocol = desc.bInterfaceProtocol;
                    return interfaceDesc;
                });
            tempDeviceInfo = tempUsbDeviceInfo;
            break;
        }
        default:
            break;
    }
    if (tempDeviceInfo != nullptr) {
        tempDeviceInfo->deviceId = deviceInfo->GetDeviceId();
        tempDeviceInfo->isDriverMatched = device->HasDriver();
        if (tempDeviceInfo->isDriverMatched) {
            tempDeviceInfo->driverUid = device->GetDriverUid();
        }
    }

    return tempDeviceInfo;
}

static std::shared_ptr<DriverInfoData> ParseToDriverInfoData(const std::shared_ptr<DriverInfo> &driverInfo)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (driverInfo == nullptr || driverInfo->GetInfoExt() == nullptr) {
        EDM_LOGD(MODULE_DEV_MGR, "driverInfo or extInfo is null");
        return nullptr;
    }
    auto busType = driverInfo->GetBusType();
    if (busType <= BusType::BUS_TYPE_INVALID || busType >= BusType::BUS_TYPE_MAX) {
        EDM_LOGD(MODULE_DEV_MGR, "invalid busType:%{public}u", busType);
        return nullptr;
    }
    std::shared_ptr<DriverInfoData> tempDriverInfo = nullptr;

    switch (busType) {
        case BusType::BUS_TYPE_USB: {
            std::shared_ptr<UsbDriverInfo> usbDriverInfo =
                std::static_pointer_cast<UsbDriverInfo>(driverInfo->GetInfoExt());
            std::shared_ptr<USBDriverInfoData> tempUsbDriverInfo = std::make_shared<USBDriverInfoData>();
            tempUsbDriverInfo->pids = usbDriverInfo->GetProductIds();
            tempUsbDriverInfo->vids = usbDriverInfo->GetVendorIds();
            tempDriverInfo = tempUsbDriverInfo;
            break;
        }
        default:
            break;
    }
    if (tempDriverInfo != nullptr) {
        tempDriverInfo->busType = busType;
        tempDriverInfo->driverUid = driverInfo->GetDriverUid();
        tempDriverInfo->driverName = driverInfo->GetDriverName();
        tempDriverInfo->bundleSize = driverInfo->GetDriverSize();
        tempDriverInfo->version = driverInfo->GetVersion();
        tempDriverInfo->description = driverInfo->GetDescription();
    }
    return tempDriverInfo;
}

ErrCode DriverExtMgr::QueryDeviceInfo(int32_t &errorCode, std::vector<std::shared_ptr<DeviceInfoData>> &deviceInfos,
    bool isByDeviceId, const uint64_t deviceId)
{
    EDM_LOGD(MODULE_DEV_MGR, "%{public}s enter", __func__);
    if (!ExtPermissionManager::IsSystemApp()) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s none system app", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NOT_SYSTEM_APP);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    vector<shared_ptr<Device>> devices;
    if (isByDeviceId) {
        devices = ExtDeviceManager::GetInstance().QueryDevicesById(deviceId);
    } else {
        devices = ExtDeviceManager::GetInstance().QueryAllDevices();
    }

    for (const auto &device : devices) {
        auto tempDeviceInfo = ParseToDeviceInfoData(device);
        if (tempDeviceInfo != nullptr) {
            deviceInfos.push_back(tempDeviceInfo);
        }
    }
    errorCode = static_cast<int32_t>(UsbErrCode::EDM_OK);
    return static_cast<int32_t>(UsbErrCode::EDM_OK);
}

ErrCode DriverExtMgr::QueryDriverInfo(int32_t &errorCode, std::vector<std::shared_ptr<DriverInfoData>> &driverInfos,
    bool isByDriverUid, const std::string &driverUid)
{
    if (!ExtPermissionManager::IsSystemApp()) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s none system app", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NOT_SYSTEM_APP);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    if (!ExtPermissionManager::VerifyPermission(PERMISSION_NAME)) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s no permission", __func__);
        errorCode = static_cast<int32_t>(UsbErrCode::EDM_ERR_NO_PERM);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }

    vector<shared_ptr<DriverInfo>> tempDriverInfos;
    int32_t ret = DriverPkgManager::GetInstance().QueryDriverInfo(tempDriverInfos, isByDriverUid, driverUid);
    if (ret != UsbErrCode::EDM_OK) {
        errorCode = static_cast<int32_t>(ret);
        return static_cast<int32_t>(UsbErrCode::EDM_OK);
    }
    for (const auto &driverInfo : tempDriverInfos) {
        auto tempDriverInfo = ParseToDriverInfoData(driverInfo);
        if (tempDriverInfo != nullptr) {
            driverInfos.push_back(tempDriverInfo);
        }
    }
    EDM_LOGD(MODULE_DEV_MGR, "driverInfos size: %{public}zu enter", driverInfos.size());

    errorCode = static_cast<int32_t>(UsbErrCode::EDM_OK);
    return static_cast<int32_t>(UsbErrCode::EDM_OK);
}

ErrCode DriverExtMgr::NotifyUsbPeripheralFault(const std::string &domain, const std::string &faultName)
{
    if (!ExtPermissionManager::IsSystemApp()) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s none system app", __func__);
        return static_cast<int32_t>(UsbErrCode::EDM_ERR_NOT_SYSTEM_APP);
    }

    if (domain.empty() || faultName.empty()) {
        EDM_LOGE(MODULE_SERVICE, "Invalid domain or faultName");
        return static_cast<int32_t>(UsbErrCode::EDM_ERR_INVALID_PARAM);
    }

    EDM_LOGI(MODULE_SERVICE, "HandleFaultEvent domain = %{public}s, faultName = %{public}s", domain.c_str(),
        faultName.c_str());

    auto faultInfo = eventConfig_.GetFaultInfo(domain, faultName);
    if (faultInfo.faultName.empty() || faultInfo.type.empty() || faultInfo.title.empty() || faultInfo.msg.empty()) {
        EDM_LOGE(MODULE_SERVICE, "GetFaultInfo failed");
        return static_cast<int32_t>(UsbErrCode::EDM_ERR_INVALID_PARAM);
    }

    if (!DeviceNotification::GetInstance().HandleNotification(faultInfo)) {
        EDM_LOGE(MODULE_SERVICE, "Failed to send handle notification");
        return static_cast<int32_t>(UsbErrCode::EDM_NOK);
    }
    return static_cast<int32_t>(UsbErrCode::EDM_OK);
}
} // namespace ExternalDeviceManager
} // namespace OHOS
