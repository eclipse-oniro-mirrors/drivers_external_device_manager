/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "iostream"
#include "string"
#include "securec.h"
#define private public
#include "ibus_extension.h"
#include "driver_pkg_manager.h"
#include "hilog_wrapper.h"
#include "driverpkgmanager_fuzzer.h"
#undef private
namespace OHOS {
namespace ExtDevMgr {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS::ExternalDeviceManager;

bool QueryMatchDriverIllegalBusTest(const uint8_t *data, size_t size)
{
    DriverPkgManager drvPkgMgrInstance;
    bool ret = drvPkgMgrInstance.Init();
    if (!ret) {
        return false;
    }

    int busType = *(static_cast<const uint8_t *>(data));
    DeviceInfo devInfo = DeviceInfo(busType);
    devInfo.devInfo_.devBusInfo.busType = BUS_TYPE_USB;
    BundleInfoNames* bundle = drvPkgMgrInstance.QueryMatchDriver(devInfo);
    if (bundle == nullptr) {
        return false;
    }
    
    return true;
}

using TestFuncDef = bool (*)(const uint8_t *data, size_t size);

TestFuncDef g_allTestFunc[] = {
    QueryMatchDriverIllegalBusTest,
};

bool DoSomethingInterestingWithMyAPI(const uint8_t *rawData, size_t size)
{
    if (size < sizeof(int)) {
        return false;
    }
    int index = *(static_cast<const uint8_t *>(rawData));
    rawData += sizeof(int);
    size -= sizeof(int);
    int funcCount = sizeof(g_allTestFunc) / sizeof(g_allTestFunc[0]);

    auto func = g_allTestFunc[index % funcCount];
    if (func != nullptr) {
        auto ret = func(rawData, size);
        return ret;
    }
    return false;
}
} // namespace ExtDevMgr
} // namespace OHOS
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::ExtDevMgr::ExternalDeviceManager::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}