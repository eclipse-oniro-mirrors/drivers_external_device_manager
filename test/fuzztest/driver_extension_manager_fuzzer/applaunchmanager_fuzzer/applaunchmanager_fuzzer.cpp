/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "securec.h"
#define private public
#include "app_launch_manager.h"
#include "hilog_wrapper.h"
#include "applaunchmanager_fuzzer.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {

static constexpr size_t MIN_FUZZ_DATA_SIZE = 4;

bool OnDeviceConnectedFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < MIN_FUZZ_DATA_SIZE) {
        return false;
    }

    uint16_t vid = static_cast<uint16_t>((data[0] << 8) | data[1]);
    uint16_t pid = static_cast<uint16_t>((data[2] << 8) | data[3]);

    auto &mgr = AppLaunchManager::GetInstance();
    mgr.OnDeviceConnected(vid, pid);
    return true;
}

} // namespace ExternalDeviceManager
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::ExternalDeviceManager::OnDeviceConnectedFuzzTest(data, size);
    return 0;
}