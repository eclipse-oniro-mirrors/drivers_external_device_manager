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

#include "DriverExtensionContext.impl.hpp"
#include "DriverExtensionContext.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

#include "DriverExtensionContext_ani.h"
#include "ani_utils.h"
#include "ets_extension_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char *DRIVER_EXTENSION_CONTEXT_CLS = "Lapplication/DriverExtensionContext/DriverExtensionContext;";
void UpdateDriverState([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    HILOG_DEBUG("UpdateDriverState begin");
    DriverExtensionContext *driverExtensionContext = AniObjectUtils::Unwrap<DriverExtensionContext>(env, object);
    if (driverExtensionContext == nullptr) {
        return;
    }
    ErrCode innerErrorCode = driverExtensionContext->UpdateDriverState();
    if (innerErrorCode != ANI_OK) {
        HILOG_ERROR("UpdateDriverState error: %{public}d", innerErrorCode);
        return;
    }
    HILOG_DEBUG("UpdateDriverState end");
}

ani_object CreateAniDriverExtensionContext(ani_env *env, std::shared_ptr<DriverExtensionContext> context,
    const std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> &application)
{
    HILOG_DEBUG("CreateAniDriverExtensionContext begin");
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((env->FindClass(DRIVER_EXTENSION_CONTEXT_CLS, &cls)) != ANI_OK) {
        HILOG_ERROR("FindClass err: %{public}s", DRIVER_EXTENSION_CONTEXT_CLS);
        return nullptr;
    }
    ani_object contextObj = AniObjectUtils::Create(env, cls);
    std::array methods = {
        ani_native_function {"UpdateDriverState", nullptr, reinterpret_cast<void *>(UpdateDriverState)},
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        HILOG_ERROR("Cannot bind native methods to %{public}s", DRIVER_EXTENSION_CONTEXT_CLS);
        return nullptr;
    }
    if (ANI_OK != AniObjectUtils::Wrap(env, contextObj, context.get())) {
        HILOG_ERROR("Cannot wrap native object");
        return nullptr;
    }
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, "nativeContext", &field)) != ANI_OK) {
        HILOG_ERROR("status: %{public}d", status);
        return nullptr;
    }
    ani_long nativeContextLong = reinterpret_cast<ani_long>(context.get());
    if ((status = env->Object_SetField_Long(contextObj, field, nativeContextLong)) != ANI_OK) {
        HILOG_ERROR("status: %{public}d", status);
        return nullptr;
    }
    if (application == nullptr) {
        HILOG_ERROR("application null");
        return nullptr;
    }
    OHOS::AbilityRuntime::CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    HILOG_DEBUG("CreateAniDriverExtensionContext end");
    return contextObj;
}
} // namespace AbilityRuntime
} // namespace OHOS
// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
// NOLINTEND
