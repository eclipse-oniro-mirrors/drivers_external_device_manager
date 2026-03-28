/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "usb_ddk_fuzzer.h"
#include "usb_ddk_api.h"
#include "usb_ddk_types.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {

bool FuzzUsbControlTransfer(const uint8_t *data, size_t size)
{
    if (size < sizeof(UsbControlRequestSetup) + sizeof(uint64_t) + sizeof(uint32_t)) {
        return false;
    }
    
    uint64_t deviceId = *reinterpret_cast<const uint64_t*>(data);
    const UsbControlRequestSetup* setup = reinterpret_cast<const UsbControlRequestSetup*>(data + sizeof(uint64_t));
    uint32_t timeout = *reinterpret_cast<const uint32_t*>(
        data + sizeof(uint64_t) + sizeof(UsbControlRequestSetup));
    
    uint8_t buffer[256] = {0};
    int32_t ret = OH_Usb_ControlTransfer(deviceId, setup, buffer, timeout);
    EDM_LOGD(MODULE_USB_DDK, "FuzzUsbControlTransfer result: %{public}d", ret);
    
    return true;
}

bool FuzzUsbGetNonRootHubs(const uint8_t *data, size_t size)
{
    Usb_NonRootHubArray hubArray;
    hubArray.nonRootHubIds = new uint64_t[128];
    hubArray.num = 0;
    
    int32_t ret = OH_Usb_GetNonRootHubs(&hubArray);
    EDM_LOGD(MODULE_USB_DDK, "FuzzUsbGetNonRootHubs result: %{public}d, num: %{public}u", ret, hubArray.num);
    
    delete[] hubArray.nonRootHubIds;
    return true;
}

using TestFuncDef = bool (*)(const uint8_t *data, size_t size);

TestFuncDef g_allTestFunc[] = {
    FuzzUsbControlTransfer,
    FuzzUsbGetNonRootHubs,
};

bool DoSomethingInterestingWithMyAPI(const uint8_t *rawData, size_t size)
{
    if (size < sizeof(int)) {
        return false;
    }
    const int index = *(static_cast<const uint8_t*>(rawData));
    rawData += sizeof(int);
    size -= sizeof(int);
    int funcCount = sizeof(g_allTestFunc) / sizeof(g_allTestFunc[0]);

    auto func = g_allTestFunc[index % funcCount];
    if (func != nullptr) {
        return func(rawData, size);
    }
    return false;
}

} // namespace ExternalDeviceManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OH_Usb_Init();
    OHOS::ExternalDeviceManager::DoSomethingInterestingWithMyAPI(data, size);
    OH_Usb_Release();
    return 0;
}
