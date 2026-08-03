# AGENTS.md - External Device Manager Codebase Guide

Guidance for AI coding agents in the OpenHarmony External Device Manager (HDF subsystem, component @ohos/external_device_manager, SA 5110, process hdf_ext_devmgr).

## Before You Edit (mandatory)

Before writing any code, state in your response:
1. **Task category** - which row of the "Where To Look" table below applies.
2. **Files you will read** for context (per Knowledge Routing).
3. **Constraints that apply** (per Do-Not / Ask-Before / Invariants).

If you cannot determine the category or applicable constraints, ask the user before proceeding.

## Where To Look

| Task type | Primary paths | High-risk? |
| --- | --- | --- |
| Change SA service logic (QueryDevice/BindDevice/UnBindDevice/QueryDeviceInfo/QueryDriverInfo) | services/native/driver_extension_manager/src/driver_ext_mgr.cpp, include/driver_ext_mgr.h | **Yes** - public IDL API |
| Change IPC interface (add/remove/renumber methods) | interfaces/innerkits/IDriverExtMgr.idl, hdf_ext_devmgr_interface_code.h | **Yes** - wire protocol |
| Change device-manager matching / binding logic | services/native/driver_extension_manager/src/device_manager/etx_device_mgr.cpp, include/device_manager/ | Yes |
| Change driver-package parsing / metadata | services/native/driver_extension_manager/src/drivers_pkg_manager/, include/drivers_pkg_manager/ | Yes - persistent DB |
| Change bus extension (hot-plug, enumeration) | services/native/driver_extension_manager/src/bus_extension/, include/bus_extension/ | Yes |
| Change USB bus extension plugin | services/native/driver_extension_manager/src/bus_extension/usb/, include/bus_extension/usb/ | Yes |
| Change DriverExtensionAbility / context | services/native/driver_extension/include/, src/ | Yes - public ability |
| Change DDK C API (USB/HID/SCSI/UsbSerial/Base) | interfaces/ddk/<bus>/<bus>_api.h, <bus>_types.h, frameworks/ddk/<bus>/ | **Yes** - public C API with @since |
| Change client SDK | frameworks/native/driver_ext_mgr_client.cpp, interfaces/innerkits/driver_ext_mgr_client.h | **Yes** - inner_kits |
| Change JS NAPI bindings | frameworks/js/napi/device_manager/, driver_extension_ability/, driver_extension_context/ | Yes |
| Change ANI / Taihe FFI bindings | frameworks/js/ani/, frameworks/js/taihe/ | Yes - generated |
| Change permissions / access control | services/native/driver_extension_manager/src/ext_permission_manager.cpp, include/ext_permission_manager.h, sa_profile/hdf_ext_devmgr.cfg | **Yes** - security |
| Change DFX / HiSysEvent | hisysevent.yaml, services/native/driver_extension_manager/src/drivers_hisysevent/ | **Yes** - observability contract |
| Change build config / feature flags | extdevmgr.gni, bundle.json, relevant BUILD.gn | Yes |
| Change logging / error codes | utils/include/hilog_wrapper.h, utils/include/edm_errors.h | Yes |
| Add unit / module / fuzz tests | test/unittest/, test/moduletest/, test/fuzztest/ | No |

## Key Directories

| Path | Responsibility |
| --- | --- |
| services/native/driver_extension_manager/ | Core SA 5110 service (src/ + include/ for all sub-modules) |
| services/native/driver_extension/ | DriverExtensionAbility and DriverExtensionContext definitions |
| interfaces/innerkits/ | IDL, IPC codes, shared types, client header (published as inner_kits) |
| interfaces/ddk/ | Public DDK C API headers: usb/, hid/, scsi/, usb_serial/, base/ |
| frameworks/native/ | Client implementation (driver_ext_mgr_client.cpp) |
| frameworks/ddk/ | DDK framework implementations: usb/, hid/, scsi/, usb_serial/, base/ |
| frameworks/js/napi/ | NAPI bindings: device_manager/, driver_extension_ability/, driver_extension_context/ |
| frameworks/js/ani/ | ANI bindings (generated via taihe_ffi_gen) |
| frameworks/js/taihe/ | Taihe FFI bindings |
| utils/include/ | hilog_wrapper.h, edm_errors.h, ibus_extension.h, common patterns |
| sa_profile/ | SA 5110 JSON profile + init CFG (UID, GID, permissions, SELinux) |
| extdevmgr.gni | Build feature flags (see Invariants) |
| hisysevent.yaml | HiSysEvent domain definitions (observability contract) |
| test/unittest/ | Unit tests (group target external_device_manager_ut) |
| test/moduletest/ | Module tests (group target external_device_manager_mt) |
| test/fuzztest/ | Fuzz tests (group target fuzztest) |

## Knowledge Routing

Read the relevant files **before** editing.

### Task-based routing

| When the task involves... | Read these first |
| --- | --- |
| Changing IDL method signatures or adding/removing SA methods | interfaces/innerkits/IDriverExtMgr.idl, IDriverExtMgrCallback.idl, hdf_ext_devmgr_interface_code.h - IDL is the source of truth; stub/proxy are generated |
| Changing DDK C API (signatures, types, return codes, @since version) | interfaces/ddk/<bus>/<bus>_api.h and <bus>_types.h; check @since and @syscap tags - these are public, versioned APIs |
| Changing permission checks or adding new permissions | driver_ext_mgr.cpp (permission constants at top of file), ext_permission_manager.cpp, sa_profile/hdf_ext_devmgr.cfg (service-level permissions) |
| Changing DFX events (adding/removing/changing HiSysEvent fields) | hisysevent.yaml - event schemas are a public observability contract; changing field names/types is a compatibility break |
| Changing build behavior or feature flags | extdevmgr.gni, bundle.json, relevant BUILD.gn |
| Changing USB bus extension behavior | include/bus_extension/usb/, src/bus_extension/usb/; check extdevmgr_usb_pass_through in extdevmgr.gni - it switches USB proxy 1.0 vs 2.0 |
| Changing driver-package metadata parsing | include/drivers_pkg_manager/, src/drivers_pkg_manager/ - metadata fields (driver_uid, VID/PID, bus type) are persistent and cross-version |
| Changing error codes | utils/include/edm_errors.h - EDM_NOK = ErrCodeOffset(SUBSYS_DRIVERS, EDM_MODULE_ID), not a simple increment; inserting codes shifts all subsequent values |
| Changing logging domains or module IDs | utils/include/hilog_wrapper.h - 13 module IDs in UsbMgrSubModule enum; log domain base 0xD002550 |

### Path-based routing

| When editing files in... | Also read |
| --- | --- |
| interfaces/innerkits/*.idl | interfaces/innerkits/BUILD.gn (generated stub/proxy); hdf_ext_devmgr_interface_code.h (IPC code enum) |
| interfaces/ddk/*/ | frameworks/ddk/<same_bus>/ (framework impl); test/unittest/ddk_*_test/ (existing tests) |
| frameworks/js/napi/ | frameworks/js/ani/ and frameworks/js/taihe/ - API surface must stay in sync across binding technologies |
| sa_profile/ | hdf_ext_devmgr.cfg (permissions, SELinux), 5110.json (SA config, min_hdi_proxy_version) |
| services/native/driver_extension_manager/src/device_manager/ | include/device_manager/ headers; include/bus_extension/ for bus contracts |
| test/unittest/ | test/unittest/BUILD.gn - test group external_device_manager_ut gates on external_device_manager_enable_service |

### Vocabulary-based routing

| Term in task, log, issue, or code | Meaning and where to look |
| --- | --- |
| **DDK** (Driver Development Kit) | Public C API for device vendors: interfaces/ddk/ (headers), frameworks/ddk/ (impl). Each bus has its own SysCap and @since version. |
| **DriverExtensionAbility** | App-ability hosting a vendor driver: services/native/driver_extension/. Lifecycle managed by DriverExtensionController in device_manager/. |
| **SA 5110** | System Ability ID for hdf_ext_devmgr: sa_profile/5110.json, sa_profile/hdf_ext_devmgr.cfg. |
| **HAP** | Harmony Ability Package - driver package format installed via BundleManager. Parsed in drivers_pkg_manager/. |
| **VID/PID** | Vendor ID / Product ID - USB device identification for driver-device matching. See UsbDeviceInfo, UsbDriverInfo in interfaces/innerkits/driver_ext_mgr_types.h. |
| **bus extension** | Pluggable bus-type module: IBusExtension in utils/include/ibus_extension.h, BusExtensionCore loads plugins. Currently USB; HID/SCSI/Serial extensible. |
| **bundleMatchMap_** | In-memory device-to-driver matching table in ExtDeviceManager. Updated on device connect and driver-package install. |
| **driver_uid** | Unique identifier for a driver package; used in QueryDriverInfo() and matching. Persistent in metadata. |
| **APL** (App Permission Level) | Service runs at system_basic APL. See sa_profile/hdf_ext_devmgr.cfg. |
| **taihe_ffi_gen** | Code generator for ANI bindings; frameworks/js/taihe/ and frameworks/js/ani/ contain generated code. |
| **extdevmgr_usb_pass_through** | Build flag in extdevmgr.gni; when true uses USB proxy 2.0, when false uses proxy 1.0. Affects which mock files tests use. |

## Constraints and Boundaries

### Do NOT

1. **Do NOT** edit generated stub/proxy code directly. IDL files (interfaces/innerkits/*.idl) are the source of truth; driver_ext_mgr_stub.h, driver_ext_mgr_proxy.cpp, and callback stubs are generated. Edit the .idl and regenerate.
2. **Do NOT** edit frameworks/js/ani/ or frameworks/js/taihe/ by hand - these are generated by taihe_ffi_gen. Edit the source .h/.cpp and regenerate.
3. **Do NOT** change DDK C API function signatures, parameter types, return-code values, or @since versions without escalation. These are public APIs (usb_ddk_api.h @since 10, ddk_api.h @since 12, scsi_peripheral_api.h @since 16). Breaking changes require compatibility review.
4. **Do NOT** change IDL method signatures or IPC code enum values (hdf_ext_devmgr_interface_code.h) without escalation. IPC codes 1-8 are a wire-protocol contract; renumbering breaks cross-version communication.
5. **Do NOT** remove or weaken permission checks. ACCESS_EXTENSIONAL_DEVICE_DRIVER gates BindDevice/UnBindDevice; ACCESS_DDK_DRIVERS gates QueryDeviceInfo/QueryDriverInfo. See driver_ext_mgr.cpp:38-39 for constants.
6. **Do NOT** change HiSysEvent field names, types, or levels in hisysevent.yaml without escalation. Downstream consumers depend on the event schema (4 events: DRIVER_PACKAGE_CYCLE_MANAGER, EXT_DEVICE_EVENT, EXTERNAL_DEVICE_SA_EVENT, EXTERNAL_DEVICE_DDK_EVENT).
7. **Do NOT** insert new UsbErrCode enum values before existing ones in edm_errors.h - this shifts all subsequent error-code values and breaks compatibility.
8. **Do NOT** change the service UID/GID, SELinux label (u:r:hdf_ext_devmgr:s0), or APL (system_basic) in sa_profile/hdf_ext_devmgr.cfg without escalation.
9. **Do NOT** add new external_deps to BUILD.gn without checking bundle.json - undeclared component dependencies will fail CI.
10. **Do NOT** remove the Apache 2.0 license header from any file.

### Ask Before

1. **Ask before** adding a new bus type (e.g., HID, SCSI, Serial). Requires new IBusExtension implementation, BusExtensionCore registration, new DDK SysCap, and bundle.json update.
2. **Ask before** adding a new permission. Requires sa_profile/hdf_ext_devmgr.cfg update, ext_permission_manager.cpp integration, and security review.
3. **Ask before** changing the persistent data format of driver-package metadata (stored via relational_store). Old data must be migrated.
4. **Ask before** changing extdevmgr_usb_pass_through or external_device_manager_enable_service defaults in extdevmgr.gni - these affect build configuration across products.
5. **Ask before** changing min_hdi_proxy_version in sa_profile/5110.json - this affects HDI version compatibility.

### Invariants

1. **Module layering**: interfaces/ (public API) -> frameworks/ (client/DDK impl) -> services/ (SA implementation). Do not reverse dependency direction; interfaces/ must not depend on services/ or frameworks/.
2. **Bus extension pattern**: All bus types implement IBusExtension (utils/include/ibus_extension.h). BusExtensionCore loads and manages plugins. New bus types must follow this pattern.
3. **Ownership**: Use std::shared_ptr/std::weak_ptr for C++ objects; use sptr/wptr for OpenHarmony remote objects (IRemoteObject).
4. **Thread safety**: Use std::lock_guard<std::recursive_mutex> or std::lock_guard<std::mutex> for shared state. The service is multi-threaded (IPC + bus-event callbacks + bundle-state callbacks).
5. **Error codes**: EDM_OK = 0; all other codes are offset by ErrCodeOffset(SUBSYS_DRIVERS, EDM_MODULE_ID). DDK APIs use DDK_RetCode / bus-specific return codes, not UsbErrCode.
6. **Logging**: Use EDM_LOGI/LOGE/LOGW/LOGD/LOGF(module, fmt, ...) from hilog_wrapper.h. Use %{public}s for non-sensitive strings; do not log device serial numbers, token IDs, or permission values in plain text. Available modules: MODULE_FRAMEWORK, MODULE_SERVICE, MODULE_DEV_MGR, MODULE_PKG_MGR, MODULE_EA_MGR, MODULE_BUS_USB, MODULE_COMMON, MODULE_USB_DDK, MODULE_HID_DDK, MODULE_BASE_DDK, MODULE_USB_SERIAL_DDK, MODULE_SCSIPERIPHERAL_DDK.
7. **File naming**: snake_case for all .cpp/.h files. Header guard format: OHOS_EXTERNAL_DEVICE_MANAGER_<FILENAME>_H.
8. **Feature flag awareness**: extdevmgr_usb_pass_through (default true) switches between USB proxy 2.0 and 1.0 - test mock files differ (usb_host_impl_mock.cpp vs usb_impl_mock.cpp). external_device_manager_enable_service (default true) gates whether the service and tests are built. extdevmgr_feature_metrics_enable is auto-set based on hiviewdfx_api_metrics presence.

## Code Style

### File Headers
Apache 2.0 license header with copyright years (e.g., 2023-2025).

### Namespaces
```cpp
namespace OHOS {
namespace ExternalDeviceManager {
// code
} // namespace ExternalDeviceManager
} // namespace OHOS
```

### Naming Conventions
| Type | Convention | Example |
|------|------------|---------|
| Classes/Structs | PascalCase | DeviceManager |
| Functions/Methods | PascalCase | ConnectDevice() |
| Member Variables | camelCase + _ | deviceInfo_ |
| Local Variables | camelCase | deviceId |
| Constants | UPPER_SNAKE_CASE | EDM_OK |
| Files | snake_case | device_manager.cpp |

### Clang-Format
Style file: .clang-format (WebKit base, ColumnLimit: 120, PointerAlignment: Right). Run clang-format on changed files before submitting.

## Build Commands

```bash
# Build 32-bit ARM (e.g., product-name rk3568)
./build.sh --product-name {product_name} --ccache --build-target external_device_manager

# Build 64-bit ARM
./build.sh --product-name {product_name} --ccache --target-cpu arm64 --build-target external_device_manager
```

{product_name} example: rk3568. See bundle.json for the full sub_component list.

## Test Commands

```bash
# All unit tests (group)
./build.sh --product-name {product_name} --build-target external_device_manager_ut

# Specific unit tests
./build.sh --product-name {product_name} --build-target bus_extension_usb_test
./build.sh --product-name {product_name} --build-target device_manager_test
./build.sh --product-name {product_name} --build-target drivers_pkg_manager_test
./build.sh --product-name {product_name} --build-target driver_extension_manager_client_test
./build.sh --product-name {product_name} --build-target ddk_base_test
./build.sh --product-name {product_name} --build-target ddk_scsi_test
./build.sh --product-name {product_name} --build-target ddk_usb_serial_test

# Module tests (group)
./build.sh --product-name {product_name} --build-target external_device_manager_mt

# Fuzz tests (group)
./build.sh --product-name {product_name} --build-target fuzztest
```

## Lint

```bash
# Format changed files with clang-format (WebKit style, ColumnLimit: 120)
clang-format -i <changed_files>
```

## Verification and Done Definition

Before declaring a task complete, verify ALL of the following:

1. **Build passes**: Run the build command above for the affected target. If you changed DDK headers, also build the corresponding DDK framework target.
2. **Tests pass**: Run the relevant test target(s) from the Test Commands section. If you added new code, add or update tests in test/unittest/, test/moduletest/, or test/fuzztest/.
3. **Lint passes**: Run clang-format on all changed .cpp/.h files.
4. **No constraint violations**: Re-check the Do-Not and Ask-Before lists. If you touched any file listed as high-risk, confirm escalation was obtained.
5. **API compatibility**: If you changed any IDL, DDK C API, or inner_kits header, confirm the change is backward-compatible or has been escalated.
6. **License headers**: All new files have the Apache 2.0 license header.

### Final Response

In your final response, report:
- What was changed and why.
- Which build/test/lint commands were run and their results.
- Which constraints were checked and confirmed.
- Any escalation needed (e.g., public API change, permission change).

### Fallback

If the build or test environment is unavailable (e.g., no OpenHarmony build toolchain), state this explicitly in your response. Run clang-format if available. List the commands that SHOULD be run and ask the user to verify.
