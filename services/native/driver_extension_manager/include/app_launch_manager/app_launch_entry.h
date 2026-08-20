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

#ifndef APP_LAUNCH_ENTRY_H
#define APP_LAUNCH_ENTRY_H

#include <string>

namespace OHOS {
namespace ExternalDeviceManager {

struct AppLaunchEntry {
    uint16_t vid = 0;
    uint16_t pid = 0;
    std::string bundleName;
    std::string abilityName;
    std::string appStoreUri;
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // APP_LAUNCH_ENTRY_H
