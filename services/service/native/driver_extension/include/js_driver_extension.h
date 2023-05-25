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

#ifndef OHOS_ABILITY_RUNTIME_JS_DRIVER_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_DRIVER_EXTENSION_H

#include "configuration.h"
#include "driver_extension.h"

class NativeReference;
class NativeValue;
class NativeObject;

namespace OHOS {
namespace AbilityRuntime {
class DriverExtension;
class JsRuntime;
/**
 * @brief Basic driver components.
 */
class JsDriverExtension : public DriverExtension,
                           public std::enable_shared_from_this<JsDriverExtension> {
public:
    explicit JsDriverExtension(JsRuntime& jsRuntime);
    virtual ~JsDriverExtension() override;

    /**
     * @brief Create JsDriverExtension.
     *
     * @param runtime The runtime.
     * @return The JsDriverExtension instance.
     */
    static JsDriverExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Driver extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Driver extension.
     * @return Returns a pointer to the <b>sid</b> of the connected Driver extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Driver extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Driver extension.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     * @return Returns a pointer to the <b>sid</b> of the connected Driver extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback) override;

    /**
     * @brief Called when all abilities connected to this Driver extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this Driver extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    void OnDisconnect(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo,
        bool &isAsyncCallback) override;

    /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop() override;

    /**
     * @brief Called when extension need dump info.
     *
     * @param params The params from driver.
     * @param info The dump info to show.
     */
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

private:
    NativeValue* CallObjectMethod(const char* name, NativeValue* const *argv = nullptr, size_t argc = 0);

    void BindContext(NativeEngine& engine, NativeObject* obj);

    void GetSrcPath(std::string &srcPath);

    NativeValue *CallOnConnect(const AAFwk::Want &want);

    NativeValue *CallOnDisconnect(const AAFwk::Want &want, bool withResult = false);

    bool CheckPromise(NativeValue *result);

    bool CallPromise(NativeValue *result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo);

    JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_DRIVER_EXTENSION_H
