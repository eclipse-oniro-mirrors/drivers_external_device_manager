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

#include "js_driver_extension_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "napi_common_start_options.h"
#include "start_options.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ZERO = 0;

class JsDriverExtensionContext final {
public:
    explicit JsDriverExtensionContext(const std::shared_ptr<DriverExtensionContext>& context) : context_(context) {}
    ~JsDriverExtensionContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsDriverExtensionContext>(static_cast<JsDriverExtensionContext*>(data));
    }

    static NativeValue* UpdateDriverState(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsDriverExtensionContext* me = CheckParamsAndGetThis<JsDriverExtensionContext>(engine, info);
        return (me != nullptr) ? me->OnUpdateDriverState(*engine, *info) : nullptr;
    }

private:
    std::weak_ptr<DriverExtensionContext> context_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;

    NativeValue* OnUpdateDriverState(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("OnUpdateDriverState is called");
        
        AsyncTask::CompleteCallback complete =
            [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("UpdateDriverState begin");
                auto context = weak.lock();
                if (!context) {
                    HILOG_WARN("context is released");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                    return;
                }

                ErrCode innerErrorCode = context->UpdateDriverState();
                if (innerErrorCode == 0) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSDriverExtensionContext::OnUpdateDriverState",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* CreateJsDriverExtensionContext(NativeEngine& engine, std::shared_ptr<DriverExtensionContext> context)
{
    HILOG_INFO("CreateJsDriverExtensionContext begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    NativeValue* objValue = CreateJsExtensionContext(engine, context, abilityInfo);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsDriverExtensionContext> jsContext = std::make_unique<JsDriverExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), JsDriverExtensionContext::Finalizer, nullptr);

    const char *moduleName = "JsDriverExtensionContext";
    BindNativeFunction(engine, *object, "updateDriverState", moduleName, JsDriverExtensionContext::UpdateDriverState);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
