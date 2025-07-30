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
#include "ani_driver_extension.h"

#include "ability_info.h"
#include "ani_common_want.h"
#include "ani_remote_object.h"
#include "ani_utils.h"
#include "DriverExtensionContext_ani.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "ets_native_reference.h"

constexpr const char* DRIVER_EXTENSION_CLS = "@ohos.app.ability.DriverExtensionAbility.DriverExtensionAbility";
namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
AniDriverExtension::AniDriverExtension(ETSRuntime& aniRuntime) : stsRuntime_(aniRuntime) {}
AniDriverExtension::~AniDriverExtension() = default;

AniDriverExtension* AniDriverExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new AniDriverExtension(static_cast<ETSRuntime&>(*runtime));
}

void AniDriverExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    DriverExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        HILOG_ERROR("DriverExtensionAbility Init abilityInfo error");
        return;
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    auto pos = srcPath.rfind(".");
    if (pos != std::string::npos) {
        srcPath.erase(pos);
        srcPath.append(".abc");
    }
    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    stsObj_ = stsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
                                     abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE, false,
                                     abilityInfo_->srcEntrance);
    if (stsObj_ == nullptr) {
        HILOG_ERROR("Failed to get etsObj");
        return;
    }
    auto env = stsRuntime_.GetAniEnv();
    BindContext(env, record->GetWant(), application);
    HILOG_DEBUG("%{public}s end.", __func__);
}

void AniDriverExtension::BindContext(ani_env *env, std::shared_ptr<AAFwk::Want> want,
    const std::shared_ptr<OHOSApplication> &application)
{
    HILOG_DEBUG("StsServiceExtension BindContext Call");
    if (env == nullptr || want == nullptr) {
        HILOG_ERROR("Want info is null or env is null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    ani_object contextObj = CreateAniDriverExtensionContext(env, context, application);
    if (contextObj == nullptr) {
        HILOG_ERROR("null contextObj");
        return;
    }
    ani_field contextField;
    ani_class cls = nullptr;
    if ((env->FindClass(DRIVER_EXTENSION_CLS, &cls)) != ANI_OK) {
        HILOG_ERROR("FindClass err: %{public}s", DRIVER_EXTENSION_CLS);
        return;
    }
    auto status = env->Class_FindField(cls, "context", &contextField);
    if (status != ANI_OK) {
        HILOG_ERROR("Class_GetField context failed");
        return;
    }
    ani_ref contextRef = nullptr;
    if (env->GlobalReference_Create(contextObj, &contextRef) != ANI_OK) {
        HILOG_ERROR("GlobalReference_Create contextObj failed");
        return;
    }
    if (env->Object_SetField_Ref(stsObj_->aniObj, contextField, contextRef) != ANI_OK) {
        HILOG_ERROR("Object_SetField_Ref contextObj failed");
        return;
    }
    HILOG_DEBUG("BindContext end");
}

void AniDriverExtension::OnStart(const AAFwk::Want &want)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    Extension::OnStart(want);
    auto env = stsRuntime_.GetAniEnv();
    ani_object ani_want = OHOS::AppExecFwk::WrapWant(env, want);
    if (ANI_OK != env->Object_CallMethodByName_Void(stsObj_->aniObj, "onInit", nullptr, ani_want)) {
        HILOG_ERROR("Failed to call the method: onInit");
        return;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

void AniDriverExtension::OnStop()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    DriverExtension::OnStop();
    auto env = stsRuntime_.GetAniEnv();
    if (ANI_OK != env->Object_CallMethodByName_Void(stsObj_->aniObj, "onRelease", nullptr)) {
        HILOG_ERROR("Failed to call the method: onRelease");
        return;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

sptr<IRemoteObject> AniDriverExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin.", __func__);
    Extension::OnConnect(want);
    ani_ref result = nullptr;
    auto env = stsRuntime_.GetAniEnv();
    ani_object ani_want = OHOS::AppExecFwk::WrapWant(env, want);
    if (ANI_OK != env->Object_CallMethodByName_Ref(stsObj_->aniObj, "onConnect", nullptr, &result, ani_want)) {
        HILOG_ERROR("Failed to call the method: onConnect");
        return nullptr;
    }
    if (result == nullptr) {
        HILOG_ERROR("Failed to call onConnect : result == nullptr");
        return nullptr;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    auto obj = reinterpret_cast<ani_object>(result);
    auto remoteObj = AniGetNativeRemoteObject(env, obj);
    if (remoteObj == nullptr) {
        HILOG_ERROR("remoteObj null");
        return nullptr;
    }
    HILOG_DEBUG("end");
    return remoteObj;
}

sptr<IRemoteObject> AniDriverExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (callbackInfo == nullptr) {
        HILOG_DEBUG("%{public}s is not AsyncCallback.", __func__);
        isAsyncCallback = false;
        return this->OnConnect(want);
    }
    Extension::OnConnect(want);
    ani_boolean isAsync = false;
    auto env = stsRuntime_.GetAniEnv();
    if (ANI_OK != env->Object_SetFieldByName_Long(stsObj_->aniObj, "connectCbInfo",
        reinterpret_cast<ani_long>(callbackInfo))) {
        HILOG_ERROR("Failed to Set the connectCbInfo");
        return nullptr;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    ani_ref result = nullptr;
    if (ANI_OK !=
        env->Object_CallMethodByName_Ref(stsObj_->aniObj, "callOnConnect", nullptr, &result, wantRef)) {
        HILOG_ERROR("2 Failed to call the method: callOnConnect");
        return nullptr;
    }
    auto obj = reinterpret_cast<ani_object>(result);
    auto remoteObj = AniGetNativeRemoteObject(env, obj);
    if (remoteObj == nullptr) {
        HILOG_ERROR("remoteObj null");
        return nullptr;
    }
    if (ANI_OK != env->Object_GetFieldByName_Boolean(stsObj_->aniObj, "isOnConnectAsync", &isAsync)) {
        HILOG_ERROR("Failed to Get the isOnConnectAsync");
        return nullptr;
    }
    if (isAsync) {
        isAsyncCallback = true;
        callbackInfo->Call(remoteObj);
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
        return nullptr;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return remoteObj;
}

void AniDriverExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin.", __func__);
    Extension::OnDisconnect(want);
    auto env = stsRuntime_.GetAniEnv();
    ani_object ani_want = OHOS::AppExecFwk::WrapWant(env, want);
    if (ANI_OK != env->Object_CallMethodByName_Void(stsObj_->aniObj, "callOnDisConnect", nullptr, ani_want)) {
        HILOG_ERROR("Failed to call the method: onDisconnect");
        return;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

void AniDriverExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (callbackInfo == nullptr) {
        this->OnDisconnect(want);
        return;
    }
    auto env = stsRuntime_.GetAniEnv();
    Extension::OnDisconnect(want);
    if (ANI_OK != env->Object_SetFieldByName_Long(stsObj_->aniObj, "disConnectCbInfo",
        reinterpret_cast<ani_long>(callbackInfo))) {
        HILOG_ERROR("Failed to Set the disConnectCbInfo");
        return;
    }
    ani_object ani_want = OHOS::AppExecFwk::WrapWant(env, want);
    if (ANI_OK != env->Object_CallMethodByName_Void(stsObj_->aniObj, "callOnDisConnect", nullptr, ani_want)) {
        HILOG_ERROR("Failed to call the method: callOnDisConnect");
        return;
    }
    ani_boolean isAsync = false;
    if (ANI_OK != env->Object_GetFieldByName_Boolean(stsObj_->aniObj, "isOnDisconnectAsync", &isAsync)) {
        HILOG_ERROR("Failed to Get the isOnDisconnectAsync");
        return;
    }
    if (isAsync) {
        isAsyncCallback = true;
        callbackInfo->Call();
        AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
        return;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

ani_array_ref AniDriverExtension::ToAniStringList(ani_env *env,
    const std::vector<std::string> &params, const uint32_t length)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (env == nullptr) {
        return nullptr;
    }
    ani_array_ref result = nullptr;
    ani_class stringCls = nullptr;
    if (ANI_OK != env->FindClass("std.core.String", &stringCls)) {
        HILOG_ERROR("FindClass Lstd/core/String Failed");
        return result;
    }
    if (env->Array_New_Ref(stringCls, length, nullptr, &result) != ANI_OK) {
        return result;
    }
    for (auto i = 0; i < length;  ++i) {
        if (ANI_OK != env->Array_Set_Ref(result, i, AniStringUtils::ToAni(env, params[i]))) {
            return result;
        }
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return result;
}

void AniDriverExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    Extension::Dump(params, info);
    auto env = stsRuntime_.GetAniEnv();
    ani_array_ref params_ = ToAniStringList(env, params, params.size());
    ani_ref result = nullptr;
    if (ANI_OK != env->Object_CallMethodByName_Ref(stsObj_->aniObj, "onDump", nullptr, &result, params_)) {
        HILOG_ERROR("Failed to call the method: Dump");
        return;
    }
    ani_double length;
    if (ANI_OK != env->Object_GetPropertyByName_Double(static_cast<ani_object>(result), "length", &length)) {
        HILOG_ERROR("Object_GetPropertyByName_Double length Failed");
        return;
    }
    for (int i = 0; i < int(length); i++) {
        ani_ref stringEntryRef;
        if (ANI_OK != env->Object_CallMethodByName_Ref(static_cast<ani_object>(result), "$_get",
            "i:C{std.core.Object}", &stringEntryRef, (ani_int)i)) {
            HILOG_ERROR("Object_CallMethodByName_Ref get Failed");
            return;
        }
        info.push_back(AniStringUtils::ToStd(env, static_cast<ani_string>(stringEntryRef)));
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}
} // AbilityRuntime
} // OHOS