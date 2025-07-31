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

#ifndef OHOS_ABILITY_RUNTIME_ANI_DRIVER_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ANI_DRIVER_EXTENSION_CONTEXT_H

#include "ability_connect_callback.h"
#include "driver_extension_context.h"
#include "event_handler.h"
#include "js_free_install_observer.h"
#include "native_engine/native_engine.h"
#include "ohos_application.h"
#include <memory>

namespace OHOS {
namespace AbilityRuntime {
ani_object CreateAniDriverExtensionContext(ani_env *env, std::shared_ptr<DriverExtensionContext> context,
    const std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> &application);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_DRIVER_EXTENSION_CONTEXT_H