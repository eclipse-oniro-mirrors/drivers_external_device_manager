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
#ifndef DDK_EMULATOR_UTILS_H
#define DDK_EMULATOR_UTILS_H

#include <cstdint>

#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
constexpr int32_t DDK_EMULATOR_NOT_SUPPORT = 804;

inline bool IsEmulatorNotSupported(UsbMgrSubModule module)
{
#ifdef IS_EMULATOR
    EDM_LOGW(module, "The capability is not supported on the emulator at this time.");
    return true;
#else
    (void)module;
    return false;
#endif
}
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DDK_EMULATOR_UTILS_H