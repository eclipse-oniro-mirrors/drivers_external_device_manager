/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "taihe/runtime.hpp"
#include "ohos.driver.deviceManager.ani.hpp"
#include "hilog_wrapper.h"
#include "ohos.driver.deviceManager.impl.h"

using namespace OHOS::ExternalDeviceManager;

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    if (ANI_OK != ohos::driver::deviceManager::ANIRegister(env)) {
        EDM_LOGE(MODULE_DEV_MGR, "Error from ohos::driver::deviceManager::ANIRegister");
        return ANI_ERROR;
    }

    static const char *namespaceName = "L@ohos/driver/deviceManager/deviceManager;";
    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(namespaceName, &ns)) {
        EDM_LOGE(MODULE_DEV_MGR, "Not found '%{public}s'", namespaceName);
        return ANI_NOT_FOUND;
    }

    std::array methods = {
        ani_native_function {"bindDriverWithDeviceId", nullptr,
            reinterpret_cast<void *>(OHOS::ExternalDeviceManager::BindDriverWithDeviceIdSync)},
    };

    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size())) {
        EDM_LOGE(MODULE_DEV_MGR, "Cannot bind native methods to '%{public}s'", namespaceName);
        return ANI_NOT_FOUND;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}

