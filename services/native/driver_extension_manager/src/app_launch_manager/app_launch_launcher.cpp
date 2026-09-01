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

#include "app_launch_launcher.h"

#include "ability_manager_client.h"
#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "want.h"

namespace OHOS {
namespace ExternalDeviceManager {

int32_t AppLaunchLauncher::LaunchApp(const std::string &bundleName, const std::string &abilityName)
{
    if (bundleName.empty() || abilityName.empty()) {
        EDM_LOGE(MODULE_BUS_USB, "Invalid bundleName or abilityName");
        return EDM_ERR_INVALID_PARAM;
    }
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);

    std::shared_ptr<AAFwk::AbilityManagerClient> abmc = AAFwk::AbilityManagerClient::GetInstance();
    if (abmc == nullptr) {
        EDM_LOGE(MODULE_BUS_USB, "Get AbilityManagerClient instance failed");
        return EDM_ERR_INVALID_OBJECT;
    }

    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    int32_t ret = abmc->StartAbility(want);
    if (ret != 0) {
        EDM_LOGE(MODULE_BUS_USB, "StartAbility failed, ret=%{public}d", ret);
        return EDM_NOK;
    }

    EDM_LOGI(MODULE_BUS_USB, "StartAbility success");
    return EDM_OK;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
