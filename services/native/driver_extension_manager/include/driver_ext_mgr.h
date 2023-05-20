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

#ifndef DRIVER_EXT_MGR_H
#define DRIVER_EXT_MGR_H
#include "driver_ext_mgr_stub.h"
#include "singleton.h"
#include "system_ability.h"

namespace OHOS {
namespace ExternalDeviceManager {
class DriverExtMgr : public SystemAbility, public DriverExtMgrStub {
    DECLARE_SYSTEM_ABILITY(DriverExtMgr)
    DECLARE_DELAYED_SINGLETON(DriverExtMgr);

public:
    void OnStart() override;
    void OnStop() override;
    int Dump(int fd, const std::vector<std::u16string> &args) override;

    int32_t QueryDevice() override;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // DRIVER_EXT_MGR_H
