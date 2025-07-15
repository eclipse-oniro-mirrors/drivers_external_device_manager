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

#ifndef OHOS_ABILITY_RUNTIME_ANI_DRIVER_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_ANI_DRIVER_EXTENSION_H

#include "ani.h
#include "driver_extension.h"
#include "driver_extension_context.h"
#include "runtime.h"
#include "ets_runtime.h"

class ETSRuntime;

namespace OHOS {
namespace AbilityRuntime {
class AniDriverExtension : public DriverExtension, public std::enable_shared_from_this<AniDriverExtension> {
public:
    explicit AniDriverExtension(ETSRuntime &stsRuntime);
    virtual ~AniDriverExtension() override;
    static AniDriverExtension *Create(const std::unique_ptr<Runtime> &runtime);
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;
    virtual void OnStart(const AAFwk::Want &want) override;
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback) override;
    virtual void OnDisconnect(const AAFwk::Want &want) override;
    void OnDisconnect(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo,
        bool &isAsyncCallback) override;
    virtual void OnStop() override;
    ani_array_ref ToAniStringList(ani_env *env, const std::vector<std::string> &params, const uint32_t length);
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;
    void BindContext(
        ani_env *env, std::shared_ptr<AAFwk::Want> want, const std::shared_ptr<OHOSApplication> &application);

private:
    std::unique_ptr<AppExecFwk::ETSNativeReference> stsObj_;
    ETSRuntime &stsRuntime_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ANI_DRIVER_EXTENSION_H