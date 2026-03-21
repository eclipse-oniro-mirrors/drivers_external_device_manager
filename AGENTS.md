# AGENTS.md - External Device Manager Codebase Guide

Guidance for AI coding agents working in the OpenHarmony External Device Manager codebase (HDF subsystem).

## Build Commands

```bash
# Build 32-bit ARM
./build.sh --product-name {product_name} --ccache --build-target external_device_manager

# Build 64-bit ARM
./build.sh --product-name {product_name} --ccache --target-cpu arm64 --build-target external_device_manager
```

## Test Commands

```bash
# All unit tests
./build.sh --product-name {product_name} --build-target external_device_manager_ut

# Specific unit tests
./build.sh --product-name {product_name} --build-target bus_extension_usb_test
./build.sh --product-name {product_name} --build-target device_manager_test
./build.sh --product-name {product_name} --build-target drivers_pkg_manager_test

# Fuzz tests
./build.sh --product-name {product_name} --build-target fuzztest
```

## Code Style Guidelines

### File Headers
Apache 2.0 license header with copyright years (e.g., `2023-2025`).

### Namespaces
```cpp
namespace OHOS {
namespace ExternalDeviceManager {
// code
} // namespace ExternalDeviceManager
} // namespace OHOS
```

### Header Guards
```cpp
#ifndef OHOS_EXTERNAL_DEVICE_MANAGER_<FILENAME>_H
#define OHOS_EXTERNAL_DEVICE_MANAGER_<FILENAME>_H
#endif // OHOS_EXTERNAL_DEVICE_MANAGER_<FILENAME>_H
```

### Naming Conventions
| Type | Convention | Example |
|------|------------|---------|
| Classes/Structs | PascalCase | `DeviceManager` |
| Functions/Methods | PascalCase | `ConnectDevice()` |
| Member Variables | camelCase + `_` | `deviceInfo_` |
| Local Variables | camelCase | `deviceId` |
| Constants | UPPER_SNAKE_CASE | `EDM_OK` |
| Files | snake_case | `device_manager.cpp` |

### Logging (`hilog_wrapper.h`)
```cpp
EDM_LOGI(MODULE_DEV_MGR, "%{public}s enter", __func__);
EDM_LOGE(MODULE_DEV_MGR, "failed %{public}d", ret);
EDM_LOGW(MODULE_DEV_MGR, "warning");
EDM_LOGD(MODULE_DEV_MGR, "debug");
```
Modules: `MODULE_FRAMEWORK`, `MODULE_SERVICE`, `MODULE_DEV_MGR`, `MODULE_PKG_MGR`, `MODULE_BUS_USB`

### Error Handling (`edm_errors.h`)
```cpp
enum UsbErrCode : int32_t {
    EDM_OK = 0, EDM_NOK, EDM_ERR_INVALID_PARAM,
    EDM_ERR_INVALID_OBJECT, EDM_ERR_TIMEOUT,
};
```

### Smart Pointers & Thread Safety
- Use `std::shared_ptr`/`std::weak_ptr` for object ownership
- Use `sptr`/`wptr` for OpenHarmony remote objects
- Use `std::lock_guard<std::recursive_mutex>` for thread safety

## Business Logic

### Architecture Overview

The External Device Manager provides plug-and-play capability for non-standard protocol devices. It manages the full lifecycle of external device driver packages (development, deployment, installation, operation, capability exposure).

### Core Modules

#### 1. Driver Extension Manager Service (DriverExtMgr)
**Location**: `services/native/driver_extension_manager/include/driver_ext_mgr.h`

- **Purpose**: System ability entry point, main service for external device management
- **Key Responsibilities**:
  - Service lifecycle management (OnStart/OnStop)
  - Device query operations
  - Device bind/unbind operations
  - Driver information query
  - Callback management for device connections

- **Main Interfaces** (from `IDriverExtMgr.idl`):
  - `QueryDevice()`: Query devices by bus type
  - `BindDevice()`: Bind a device with its driver package
  - `UnBindDevice()`: Unbind a device
  - `BindDriverWithDeviceId()`: Bind driver with device ID
  - `UnBindDriverWithDeviceId()`: Unbind driver with device ID
  - `QueryDeviceInfo()`: Query device information
  - `QueryDriverInfo()`: Query driver package information

#### 2. Device Manager Module
**Location**: `services/native/driver_extension_manager/include/device_manager/`

- **Purpose**: Core device and driver matching management
- **Key Responsibilities**:
  - Device list management (deviceMap_)
  - Driver matching table management (bundleMatchMap_)
  - Device registration/unregistration
  - Device-driver binding/unbinding
  - Permission verification for driver access
  - Automatic driver matching on device connect

- **Key Classes**:
  - `ExtDeviceManager` (etx_device_mgr.h): Main device manager singleton
  - `Device` (device.h): Represents a physical device with driver binding state

- **Device-Driver Matching Flow**:
  1. Bus extension detects device connect → `RegisterDevice()`
  2. `MatchDriverInfos()` matches device to driver packages using DriverPkgManager
  3. Matching result stored in `bundleMatchMap_`
  4. Application calls `BindDevice()` → `ConnectDevice()` → starts DriverExtensionAbility
  5. Application gets driver remote object for direct interaction

#### 3. Driver Package Manager Module
**Location**: `services/native/driver_extension_manager/include/drivers_pkg_manager/`

- **Purpose**: Driver package lifecycle and metadata management
- **Key Responsibilities**:
  - Monitor driver package installation/update/uninstall (via BundleManagerService)
  - Parse driver package metadata (driver_uid, supported devices)
  - Provide driver query capability
  - Driver-device matching logic

- **Key Classes**:
  - `DriverPkgManager` (driver_pkg_manager.h): Package manager singleton
  - `DrvBundleStateCallback`: Listens for bundle state changes
  - `BundleMonitor`: Monitors bundle updates

- **Driver Metadata**:
  - Bus type (USB, HID, etc.)
  - Vendor ID/Product ID for matching
  - Driver UID
  - Driver name/version/description

#### 4. Bus Extension Core Module
**Location**: `services/native/driver_extension_manager/include/bus_extension/`

- **Purpose**: Manages different bus types and device enumeration
- **Key Responsibilities**:
  - Load and register bus extension plugins
  - Manage bus extension lifecycle
  - Device enumeration coordination
  - Device change notification forwarding

- **Key Classes**:
  - `BusExtensionCore` (bus_extension_core.h): Core manager for bus extensions
  - `IBusExtension` (utils/include/ibus_extension.h): Interface for all bus extensions

- **Supported Bus Types**:
  - USB (UsbBusExtension)
  - HID, SCSI, Serial (extensible framework)

#### 5. USB Bus Extension Plugin
**Location**: `services/native/driver_extension_manager/include/bus_extension/usb/`

- **Purpose**: USB-specific device management
- **Key Responsibilities**:
  - USB device hot-plug monitoring (via UsbDevSubscriber)
  - USB device information reading (vendor ID, product ID, interfaces)
  - Driver-device matching for USB devices
  - USB DDK interaction for device operations

- **Key Classes**:
  - `UsbBusExtension` (usb_bus_extension.h): USB bus extension implementation
  - `UsbDevSubscriber` (usb_dev_subscriber.h): Device change subscriber
  - `UsbDeviceInfo`, `UsbDriverInfo`: USB-specific data structures

#### 6. Driver Extension Controller
**Location**: `services/native/driver_extension_manager/include/device_manager/driver_extension_controller.h`

- **Purpose**: Manages DriverExtensionAbility lifecycle
- **Key Responsibilities**:
  - Start/stop DriverExtensionAbility
  - Connect/disconnect to DriverExtensionAbility
  - Manage ability connections and callbacks

- **Key Classes**:
  - `DriverExtensionController`: Singleton for ability lifecycle management
  - `IDriverExtensionConnectCallback`: Connection callback interface

### Data Flow Examples

#### Device Connection Flow
```
1. USB device inserted
   ↓
2. UsbDevSubscriber detects device
   ↓
3. UsbBusExtension reports to BusExtensionCore
   ↓
4. BusExtensionCore notifies ExtDeviceManager → RegisterDevice()
   ↓
5. ExtDeviceManager queries DriverPkgManager → QueryMatchDriver()
   ↓
6. DriverPkgManager parses driver metadata, returns matched drivers
   ↓
7. ExtDeviceManager stores match in bundleMatchMap_
   ↓
8. Application calls BindDevice()
   ↓
9. DriverExtensionController starts DriverExtensionAbility → StartDriverExtension()
   ↓
10. DriverExtensionController connects to ability → ConnectDriverExtension()
    ↓
11. Application receives remote object, can call driver functions
```

#### Driver Package Installation Flow
```
1. User installs driver package (HAP)
   ↓
2. BundleManagerService triggers DrvBundleStateCallback
   ↓
3. DriverPkgManager parses metadata from bundle
   ↓
4. DriverPkgManager queries for matching devices → QueryMatchDriver()
   ↓
5. ExtDeviceManager updates device matching status
   ↓
6. Devices auto-bind if matching drivers found
```

### Key Data Structures

| Type | Location | Purpose |
|------|----------|---------|
| `DeviceData` | interfaces/innerkits/driver_ext_mgr_types.h | Base device info |
| `USBDevice` | interfaces/innerkits/driver_ext_mgr_types.h | USB device data |
| `DeviceInfoData` | interfaces/innerkits/driver_ext_mgr_types.h | Device info with driver match status |
| `USBDeviceInfoData` | interfaces/innerkits/driver_ext_mgr_types.h | USB device with interface descriptors |
| `DriverInfoData` | interfaces/innerkits/driver_ext_mgr_types.h | Base driver info |
| `USBDriverInfoData` | interfaces/innerkits/driver_ext_mgr_types.h | USB driver with VID/PID lists |

## Key Directories

| Path | Purpose |
|------|---------|
| `services/native/driver_extension_manager/` | Core service implementation |
| `interfaces/innerkits/` | Internal interfaces and types |
| `frameworks/native/` | Bridge code (client implementation) |
| `utils/include/` | Logging, errors, common patterns |
| `interfaces/ddk/` | DDK interfaces (USB, HID, SCSI) |
| `test/unittest/` | Unit tests |
| `test/moduletest/` | Module tests |
| `test/fuzztest/` | Fuzz tests |

## Adding New Files
1. Create files with snake_case naming + Apache 2.0 license
2. Update `BUILD.gn` with sources, include_dirs, deps, external_deps
3. For tests: use `ohos_unittest()` template
