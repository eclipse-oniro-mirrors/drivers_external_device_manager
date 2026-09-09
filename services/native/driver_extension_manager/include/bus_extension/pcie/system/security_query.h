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
 *
 * Host kernel information queries — simple free functions, no class hierarchy.
 *
 *   GetKernelVersion()       : read /proc/sys/kernel/osrelease → fallback uname -r
 *   GetKabiVersion()         : read /proc/devhost/root/build_version
 *   GetDslSafetyLevel()      : read the kernel-extension signing state, "0" denied and "1" allowed
 */

#ifndef OHOS_EDM_PCIE_DRIVER_SECURITY_QUERY_H
#define OHOS_EDM_PCIE_DRIVER_SECURITY_QUERY_H

#include <string>

namespace OHOS {
namespace ExternalDeviceManager {

std::string GetKernelVersion();
std::string GetKabiVersion();
std::string GetDslSafetyLevel();

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif
