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

#ifndef OHOS_EXTERNAL_DEVICE_MANAGER_PCIE_DRIVER_TYPES_H
#define OHOS_EXTERNAL_DEVICE_MANAGER_PCIE_DRIVER_TYPES_H

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "v1_0/pcie_types.h"

namespace OHOS {
namespace ExternalDeviceManager {

// Replace hyphens with underscores for module name comparisons without changing the actual file name.
inline std::string NormalizePcieModuleName(std::string moduleName)
{
    std::replace(moduleName.begin(), moduleName.end(), '-', '_');
    return moduleName;
}

/* ── Failure reason enumeration ────────────────────────────────────────────
 *
 * Error codes shared by all modules, grouped by origin:
 *   1-5  metadata parsing
 *   6-12 policy gates (gate 0b–6 plus ranking)
 *   13-14 module resolution
 *   15-16 preemption (not a hard failure)
 */
enum class PcieDriverFailureReason : int32_t {
    NONE = 0,
    /* metadata parsing */
    INVALID_RECORD = 1,
    FILE_NOT_READABLE = 2,
    UNSUPPORTED_FORMAT = 3,
    NO_ALIAS = 4,
    NO_DRIVER_KABI_METADATA = 5,
    /* policy gates */
    PATH_ESCAPE = 6,
    ALIAS_NOT_MATCHED = 7,
    KABI_MISMATCH = 8,
    DSL_DENIED = 9,
    DEPENDENCY_FAILED = 10,
    DUPLICATE_LOADED = 11,
    LOWER_PRECISION = 12,
    /* module resolution */
    METADATA_FAILED = 13,
    POLICY_REJECTED = 14,
    /* preemption */
    ALREADY_BOUND = 15,
    RESERVED = 16,
    /* execution */
    LOAD_FAILED = 17,
};

struct PcieDriverFailureReasonName {
    PcieDriverFailureReason reason;
    const char* name;
};

inline constexpr PcieDriverFailureReasonName PCIE_DRIVER_FAILURE_REASON_NAMES[] = {
    {PcieDriverFailureReason::NONE, "NONE"},
    {PcieDriverFailureReason::INVALID_RECORD, "INVALID_RECORD"},
    {PcieDriverFailureReason::FILE_NOT_READABLE, "FILE_NOT_READABLE"},
    {PcieDriverFailureReason::UNSUPPORTED_FORMAT, "UNSUPPORTED_FORMAT"},
    {PcieDriverFailureReason::NO_ALIAS, "NO_ALIAS"},
    {PcieDriverFailureReason::NO_DRIVER_KABI_METADATA, "NO_DRIVER_KABI_METADATA"},
    {PcieDriverFailureReason::PATH_ESCAPE, "PATH_ESCAPE"},
    {PcieDriverFailureReason::ALIAS_NOT_MATCHED, "ALIAS_NOT_MATCHED"},
    {PcieDriverFailureReason::KABI_MISMATCH, "KABI_MISMATCH"},
    {PcieDriverFailureReason::DSL_DENIED, "DSL_DENIED"},
    {PcieDriverFailureReason::DEPENDENCY_FAILED, "DEPENDENCY_FAILED"},
    {PcieDriverFailureReason::DUPLICATE_LOADED, "DUPLICATE_LOADED"},
    {PcieDriverFailureReason::LOWER_PRECISION, "LOWER_PRECISION"},
    {PcieDriverFailureReason::METADATA_FAILED, "METADATA_FAILED"},
    {PcieDriverFailureReason::POLICY_REJECTED, "POLICY_REJECTED"},
    {PcieDriverFailureReason::ALREADY_BOUND, "ALREADY_BOUND"},
    {PcieDriverFailureReason::RESERVED, "RESERVED"},
    {PcieDriverFailureReason::LOAD_FAILED, "LOAD_FAILED"},
};

inline const char* PcieDriverFailureReasonToString(PcieDriverFailureReason reason)
{
    for (const auto& reasonName : PCIE_DRIVER_FAILURE_REASON_NAMES) {
        if (reasonName.reason == reason) {
            return reasonName.name;
        }
    }
    return "UNKNOWN";
}

enum class PcieDriverLoadStatus : int32_t {
    LOADED = 0,
    PATH_REJECTED = 10,
    OPEN_FAILED = 11,
    PATH_TAMPERED = 12,
    LOAD_RESOURCE_BUSY = 13,
    LOAD_UNKNOWN = 14,
    LOAD_OPERATION_NOT_PERMITTED = 15,
    LOAD_OUT_OF_MEMORY = 16,
    LOAD_IO_ERROR = 17,
    LOAD_INVALID_FILE_DESCRIPTOR = 18,
    LOAD_NOT_SUPPORTED = 19,
    LOAD_ACCESS_DENIED = 20,
    LOAD_INVALID_MEMORY_ADDRESS = 21,
    LOAD_INVALID_ARGUMENT = 22,
    LOAD_INVALID_MODULE_FORMAT = 23,
    LOAD_MODULE_ALREADY_EXISTS = 24,
    LOAD_SIGNING_KEY_MISSING = 25,
    LOAD_SIGNATURE_REJECTED = 26,
    LOAD_INVALID_DATA = 27,
    LOAD_FILE_TOO_LARGE = 28,
    LOAD_FILE_BUSY = 29,
    LOAD_REQUIRED_OBJECT_MISSING = 30,
    LOAD_INSUFFICIENT_SPACE = 31,
    LOAD_DEVICE_UNAVAILABLE = 32,
    LOAD_SIGNING_KEY_EXPIRED = 33,
    LOAD_SIGNING_KEY_REVOKED = 34,
    /* SA-side failure codes (50-99) */
    SA_METADATA_PARSE_FAILED = 50,
    SA_METADATA_FAILED = 51,
    SA_DEPENDENCY_FAILED = 52,
    SA_KABI_MISMATCH = 53,
    SA_PATH_ESCAPE = 54,
};

struct PcieDriverLoadStatusName {
    PcieDriverLoadStatus status;
    const char* name;
};

inline constexpr PcieDriverLoadStatusName PCIE_DRIVER_LOAD_STATUS_NAMES[] = {
    {PcieDriverLoadStatus::LOADED, "LOADED"},
    {PcieDriverLoadStatus::PATH_REJECTED, "PATH_REJECTED"},
    {PcieDriverLoadStatus::OPEN_FAILED, "OPEN_FAILED"},
    {PcieDriverLoadStatus::PATH_TAMPERED, "PATH_TAMPERED"},
    {PcieDriverLoadStatus::LOAD_RESOURCE_BUSY, "LOAD_RESOURCE_BUSY"},
    {PcieDriverLoadStatus::LOAD_UNKNOWN, "LOAD_UNKNOWN"},
    {PcieDriverLoadStatus::LOAD_OPERATION_NOT_PERMITTED, "LOAD_OPERATION_NOT_PERMITTED"},
    {PcieDriverLoadStatus::LOAD_OUT_OF_MEMORY, "LOAD_OUT_OF_MEMORY"},
    {PcieDriverLoadStatus::LOAD_IO_ERROR, "LOAD_IO_ERROR"},
    {PcieDriverLoadStatus::LOAD_INVALID_FILE_DESCRIPTOR, "LOAD_INVALID_FILE_DESCRIPTOR"},
    {PcieDriverLoadStatus::LOAD_NOT_SUPPORTED, "LOAD_NOT_SUPPORTED"},
    {PcieDriverLoadStatus::LOAD_ACCESS_DENIED, "LOAD_ACCESS_DENIED"},
    {PcieDriverLoadStatus::LOAD_INVALID_MEMORY_ADDRESS, "LOAD_INVALID_MEMORY_ADDRESS"},
    {PcieDriverLoadStatus::LOAD_INVALID_ARGUMENT, "LOAD_INVALID_ARGUMENT"},
    {PcieDriverLoadStatus::LOAD_INVALID_MODULE_FORMAT, "LOAD_INVALID_MODULE_FORMAT"},
    {PcieDriverLoadStatus::LOAD_MODULE_ALREADY_EXISTS, "LOAD_MODULE_ALREADY_EXISTS"},
    {PcieDriverLoadStatus::LOAD_SIGNING_KEY_MISSING, "LOAD_SIGNING_KEY_MISSING"},
    {PcieDriverLoadStatus::LOAD_SIGNATURE_REJECTED, "LOAD_SIGNATURE_REJECTED"},
    {PcieDriverLoadStatus::LOAD_INVALID_DATA, "LOAD_INVALID_DATA"},
    {PcieDriverLoadStatus::LOAD_FILE_TOO_LARGE, "LOAD_FILE_TOO_LARGE"},
    {PcieDriverLoadStatus::LOAD_FILE_BUSY, "LOAD_FILE_BUSY"},
    {PcieDriverLoadStatus::LOAD_REQUIRED_OBJECT_MISSING, "LOAD_REQUIRED_OBJECT_MISSING"},
    {PcieDriverLoadStatus::LOAD_INSUFFICIENT_SPACE, "LOAD_INSUFFICIENT_SPACE"},
    {PcieDriverLoadStatus::LOAD_DEVICE_UNAVAILABLE, "LOAD_DEVICE_UNAVAILABLE"},
    {PcieDriverLoadStatus::LOAD_SIGNING_KEY_EXPIRED, "LOAD_SIGNING_KEY_EXPIRED"},
    {PcieDriverLoadStatus::LOAD_SIGNING_KEY_REVOKED, "LOAD_SIGNING_KEY_REVOKED"},
    {PcieDriverLoadStatus::SA_METADATA_PARSE_FAILED, "SA_METADATA_PARSE_FAILED"},
    {PcieDriverLoadStatus::SA_METADATA_FAILED, "SA_METADATA_FAILED"},
    {PcieDriverLoadStatus::SA_DEPENDENCY_FAILED, "SA_DEPENDENCY_FAILED"},
    {PcieDriverLoadStatus::SA_KABI_MISMATCH, "SA_KABI_MISMATCH"},
    {PcieDriverLoadStatus::SA_PATH_ESCAPE, "SA_PATH_ESCAPE"},
};

inline const char* PcieDriverLoadStatusToString(PcieDriverLoadStatus status)
{
    for (const auto& statusName : PCIE_DRIVER_LOAD_STATUS_NAMES) {
        if (statusName.status == status) {
            return statusName.name;
        }
    }
    return "UNKNOWN";
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Data Transfer Objects
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── PCIe board record ─────────────────────────────────────────────────── */
struct PcieDeviceInfo {
    std::string boardId;         /* BDF slot, e.g. "0002:21:00.0" */
    std::string sysfsPath;       /* /sys/bus/pci/devices/0002:21:00.0 */
    std::string vendorId;        /* "0x10EC" */
    std::string deviceId;        /* "0x8168" */
    std::string classCode;       /* "0x020000" */
    std::string modalias;        /* "pci:v000010ECd00008168sv..." */
    bool        isModaliasValid = false;
    std::string boundDriver;     /* kernel driver name bound to this board, empty if none */
};

/* ── Driver file record ─────────────────────────────────────────────────── */
struct PcieDriverRecord {
    std::string driverPath;      /* absolute path */
    std::string fileName;
    uint64_t    fileSize = 0;
    uint64_t    mtime = 0;       /* modification timestamp */
    bool        isValid = false;
};

struct PcieDriverSandboxSnapshot {
    std::vector<PcieDriverRecord> driverRecords;
};

/* ── Driver metadata (parsed from .modinfo) ─────────────────────────────── */
struct PcieDriverMetadata {
    std::string              driverPath;
    std::vector<std::string> aliases;   /* PCI aliases, e.g. "pci:v000010ECd..." */
    std::string              kabi;      /* kernel ABI version string */
    std::string              vermagic;  /* vermagic string (compiler-enforced, more reliable than kabi) */
    std::string              moduleName;
    std::string              license;
    std::vector<std::string> depends;   /* module names this driver depends on */
    bool                     isParseable = false;
    PcieDriverFailureReason      failReason = PcieDriverFailureReason::NONE;
};

struct PcieDriverCandidate {
    PcieDriverRecord   driver;
    PcieDriverMetadata metadata;
};

/* ── Kernel security context ───────────────────────────────────────────── */
struct KernelSecurityContext {
    std::string kernelVersion;      /* KABI version, e.g. "HongMengKernel-1.1.160" */
    std::string kernelRelease;      /* kernel release, e.g. "6.6.127-oh" */
    std::string dslSafetyLevel;     /* "0" denied / "1" allowed */
    bool        kernelVersionAvailable = false;
    bool        kernelReleaseAvailable = false;
    bool        dslAvailable = false;
};

/* ── Policy decision (one per board x driver pair) ──────────────────────── */
struct PcieDriverPolicyDecision {
    std::string boardId;
    std::string driverPath;
    std::string moduleName;
    bool        checkPassed = false;
    bool        skipAlreadyLoaded = false; /* candidate driver KO is already loaded in the kernel */
    PcieDriverFailureReason blockReason = PcieDriverFailureReason::NONE;
    std::string aliasMatched;
    int32_t     aliasScore = 0;       /* 0-7, count of non-wildcard PCI fields */
    int64_t     driverMtime = 0;      /* tie-breaker: newer driver preferred */
    bool        boardHasBoundDriver = false; /* PCIe board currently has a bound kernel driver. */
    /* Gate 2: KABI matching — structured fields, not stringly-typed */
    bool        kabiAvailable = false; /* kernel exposes KABI info */
    bool        kabiMatched = false;   /* metadata.kabi matches host */
    bool        vermagicMatched = false; /* metadata.vermagic matches host */
    /* Gate 3/4: structured gate results */
    bool        dslAllowed = false;      /* DSL safety-level allows this driver */
    bool        dependencyMet = false;   /* all depends entries found */
};

using OHOS::HDI::Pcie::V1_0::PcieDriverLoadItem;
using OHOS::HDI::Pcie::V1_0::PcieDriverLoadResult;

struct PcieDriverModuleState {
    std::string driverPath;
    std::string moduleName;
    bool loaded = false;
};

struct PcieDriverMatchResult {
    bool matchingSucceeded = false;
    std::string statusMessage;
    std::vector<PcieDriverLoadItem>         loadRequests;
    std::vector<PcieDriverPolicyDecision>   decisions;
    std::vector<PcieDriverLoadResult> loadResults;
    std::vector<PcieDriverRecord> driverRecords;
    std::vector<PcieDriverModuleState> moduleStates;
    bool loadedModulesReadable = false;
};

/* ── Input/output compound types ───────────────────────────────────────── */
struct PcieDriverPolicyInput {
    std::vector<PcieDeviceInfo>   boardRecords;
    std::vector<PcieDriverCandidate> driverCandidates;
    KernelSecurityContext          securityContext;
    std::vector<std::string>       loadedModules;       /* kernel modules already available: loaded or built-in */
    std::vector<std::string>       dependencySnapshot;  /* sandbox + kernel modules */
    std::string                    sandboxRoot;
};

struct PcieDriverMetadataResult {
    PcieDriverMetadata      metadata;
    bool                parseSucceeded = false;
    PcieDriverFailureReason failReason = PcieDriverFailureReason::NONE;
};

struct PcieDriverManagerStatus {
    bool evaluationSucceeded = false;
    std::vector<PcieDriverLoadResult> loadResults;
    std::string statusMessage;
    std::vector<PcieDriverPolicyDecision> decisions;
    std::vector<PcieDriverModuleState> moduleStates;
    bool loadedModulesReadable = false;
};

struct PcieDriverManagerConfig {
    std::string sandboxRoot;
    std::string builtinModulesPath; /* optional platform-provided modules.builtin path */
};

/* ── Manager orchestrator ──────────────────────────────────────────────── */
class IPcieDriverManager {
public:
    virtual ~IPcieDriverManager() = default;
    virtual PcieDriverMatchResult MatchBoardDrivers(
        const std::vector<PcieDeviceInfo>& devices)
    {
        (void)devices;
        return {};
    }
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // OHOS_EXTERNAL_DEVICE_MANAGER_PCIE_DRIVER_TYPES_H
