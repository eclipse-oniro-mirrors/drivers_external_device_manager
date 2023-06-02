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

#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "driver_extension_controller.h"
using namespace OHOS;
using namespace OHOS::ExternalDeviceManager;
int32_t DriverExtensionController::StartDriverExtension(
    std::string bundleName,
    std::string abilityName)
{
    EDM_LOGI(MODULE_EA_MGR, "Begin to start DriverExtension, bundle:%{public}s, ability:%{public}s", \
        bundleName.c_str(), abilityName.c_str());
    auto abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "Get AMC Instance failed");
        return -1;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    auto ret = abmc->StartExtensionAbility(want, nullptr);
    if (ret != 0) {
        EDM_LOGE(MODULE_EA_MGR, "StartExtensionAbility failed %{public}d", ret);
        return ret;
    }
    EDM_LOGI(MODULE_EA_MGR, "StartExtensionAbility success");
    return 0;
}


int32_t DriverExtensionController::StopDriverExtension(
    std::string bundleName,
    std::string abilityName)
{
    EDM_LOGI(MODULE_EA_MGR, "Begin to stop DriverExtension, bundle:%{public}s, ability:%{public}s", \
        bundleName.c_str(), abilityName.c_str());
    auto abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        EDM_LOGE(MODULE_EA_MGR, "Get AMC Instance failed");
        return -1;
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    auto ret = abmc->StopExtensionAbility(want, nullptr);
    if (ret != 0) {
        EDM_LOGE(MODULE_EA_MGR, "StopExtensionAbility failed %{public}d", ret);
        return ret;
    }
    EDM_LOGI(MODULE_EA_MGR, "StopExtensionAbility success");
    return 0;
}