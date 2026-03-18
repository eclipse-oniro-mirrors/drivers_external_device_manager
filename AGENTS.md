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

# Specific unit tests (targets in test/unittest/BUILD.gn)
./build.sh --product-name {product_name} --build-target bus_extension_usb_test
./build.sh --product-name {product_name} --build-target device_manager_test
./build.sh --product-name {product_name} --build-target drivers_pkg_manager_test

# Module tests
./build.sh --product-name {product_name} --build-target external_device_manager_mt

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

### Include Order
1. Standard library (alphabetically) 2. System headers 3. Project headers

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

if (callback == nullptr) {
    EDM_LOGE(MODULE_DEV_MGR, "invalid callback");
    return UsbErrCode::EDM_ERR_INVALID_OBJECT;
}
```

### Single Instance (`single_instance.h`)
```cpp
class MyClass { DECLARE_SINGLE_INSTANCE(MyClass) };
IMPLEMENT_SINGLE_INSTANCE(MyClass)
MyClass::GetInstance().DoSomething();
```

### Smart Pointers & Thread Safety
- Use `std::shared_ptr`/`std::weak_ptr` for object ownership
- Use `sptr`/`wptr` for OpenHarmony remote objects
- Use `std::lock_guard<std::recursive_mutex>` for thread safety

## Testing Guidelines

### Unit Tests
```cpp
#include <gtest/gtest.h>
#include "edm_errors.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace testing::ext;

class MyTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(MyTest, TestName001, TestSize.Level1)
{
    ASSERT_EQ(SomeFunction(), EDM_OK);
}
} // namespace ExternalDeviceManager
} // namespace OHOS
```

### Fuzz Tests
```cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::ExternalDeviceManager::FuzzTest(data, size);
    return 0;
}
```

## TypeScript/ArkTS
```typescript
import { DriverExtensionAbility } from '@kit.DriverDevelopmentKit';
import { Want } from '@kit.AbilityKit';
import { rpc } from '@kit.IPCKit';

export default class DriverExtAbility extends DriverExtensionAbility {
  onInit(want: Want): void {
    console.info('testTag', `onInit, want: ${want.abilityName}`);
  }
  onConnect(want: Want): rpc.RemoteObject {
    return new StubTest('test');
  }
}
```

## Key Directories

| Path | Purpose |
|------|---------|
| `bundle.json` | Component config |
| `extdevmgr.gni` | GN build variables |
| `interfaces/innerkits/` | Internal interfaces |
| `services/native/` | Native implementations |
| `frameworks/` | Framework/bridge code |
| `utils/include/` | Logging, errors, patterns |
| `test/unittest/` | Unit tests |
| `test/moduletest/` | Module tests |
| `test/fuzztest/` | Fuzz tests |

## Adding New Files
1. Create files with snake_case naming + Apache 2.0 license
2. Update `BUILD.gn` with sources, include_dirs, deps, external_deps
3. For tests: use `ohos_unittest()` template
