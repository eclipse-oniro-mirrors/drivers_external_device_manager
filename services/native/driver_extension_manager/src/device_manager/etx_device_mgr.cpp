/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "etx_device_mgr.h"

namespace OHOS {
namespace ExternalDeviceManager {
BusExtensionCore::BusExtensionCore()
{
    return;
}
BusExtensionCore::~BusExtensionCore()
{
    return;
}

int32_t BusExtensionCore::Init()
{
    return 0;
}
int32_t BusExtensionCore::Register(
    BusType busType, std::shared_ptr<IBusExtension> busExtension)
{
    return 0;
}
} // namespace ExternalDeviceManager
} // namespace OHOS