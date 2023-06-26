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

#ifndef DRIVER_EXTENSION_MANAGER_CALLBACK_DEATH_RECIPIENT_H
#define DRIVER_EXTENSION_MANAGER_CALLBACK_DEATH_RECIPIENT_H

#include <iremote_object.h>
#include <singleton.h>

namespace OHOS {
namespace ExternalDeviceManager {
class DriverExtMgrCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    DriverExtMgrCallbackDeathRecipient() = default;
    ~DriverExtMgrCallbackDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    DISALLOW_COPY_AND_MOVE(DriverExtMgrCallbackDeathRecipient);
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXTENSION_MANAGER_CALLBACK_DEATH_RECIPIENT_H