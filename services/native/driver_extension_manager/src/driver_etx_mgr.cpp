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

#include "driver_ext_mgr.h"
#include "driver_pkg_manager.h"
#include "edm_errors.h"
#include "etx_device_mgr.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "dev_change_callback.h"
#include "bus_extension_core.h"

namespace OHOS {
namespace ExternalDeviceManager {
const bool G_REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DriverExtMgr>::GetInstance().get());

DriverExtMgr::DriverExtMgr() : SystemAbility(HDF_EXTERNAL_DEVICE_MANAGER_SA_ID, true) {}
DriverExtMgr::~DriverExtMgr() {}

void DriverExtMgr::OnStart()
{
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStart");
    DriverPkgManager::GetInstance().Init();
    ExtDeviceManager::GetInstance().Init();
    std::shared_ptr<DevChangeCallback> callback = std::make_shared<DevChangeCallback>();
    BusExtensionCore::GetInstance().Init(callback);
}

void DriverExtMgr::OnStop()
{
    EDM_LOGI(MODULE_SERVICE, "hdf_ext_devmgr OnStop");
    delete &(DriverPkgManager::GetInstance());
    delete &(ExtDeviceManager::GetInstance());
    delete &(BusExtensionCore::GetInstance());
}

int DriverExtMgr::Dump(int fd, const std::vector<std::u16string> &args)
{
    return 0;
}

int32_t DriverExtMgr::QueryDevice()
{
    return 0;
}
} // namespace ExternalDeviceManager
} // namespace OHOS
