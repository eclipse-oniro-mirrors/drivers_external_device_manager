/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "ext_permission_manager.h"

#include <sstream>

#include "accesstoken_kit.h"
#include "cJSON.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace OHOS::Security::AccessToken;

static std::string Trim(const std::string& str)
{
    if (str.empty()) {
        return str;
    }

    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}

bool ExtPermissionManager::VerifyPermission(std::string permissionName)
{
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int result = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (result == PERMISSION_GRANTED) {
        EDM_LOGI(MODULE_DEV_MGR, "%{public}s VerifyAccessToken: %{public}d", __func__, result);
        return true;
    }
    return false;
}

bool ExtPermissionManager::IsSystemApp()
{
    uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        return true;
    }
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    return tokenType != ATokenTypeEnum::TOKEN_HAP;
}

bool ExtPermissionManager::IsSa()
{
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    return tokenType == ATokenTypeEnum::TOKEN_NATIVE;
}

uint32_t ExtPermissionManager::GetCallingTokenID()
{
    return IPCSkeleton::GetCallingTokenID();
}

bool ExtPermissionManager::GetPermissionValues(const std::string &permissionName,
    std::unordered_set<std::string> &permissionValues)
{
    std::string bundleNames;
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t ret = AccessTokenKit::GetReqPermissionByName(callerToken, permissionName, bundleNames);
    if (ret != 0 || bundleNames.empty()) {
        EDM_LOGE(MODULE_DEV_MGR, "%{public}s GetReqPermissionByName: %{public}d", __func__, ret);
        return false;
    }

    cJSON* jsonObj = cJSON_Parse(bundleNames.c_str());
    if (jsonObj == nullptr) {
        EDM_LOGE(MODULE_DEV_MGR, "JSON parse failed for bundleNames");
        return false;
    }
    std::string keyStr = "bundleNames";
    cJSON* jsonItem = cJSON_GetObjectItem(jsonObj, keyStr.c_str());
    if (jsonItem == nullptr || !cJSON_IsString(jsonItem)) {
        EDM_LOGE(MODULE_DEV_MGR, "value is not string, key:%{public}s", keyStr.c_str());
        cJSON_Delete(jsonObj);
        return false;
    }
    std::istringstream bundleNamesStram(jsonItem->valuestring);
    std::string bundleName;
    while (std::getline(bundleNamesStram, bundleName, ',')) {
        permissionValues.insert(Trim(bundleName));
    }
    cJSON_Delete(jsonObj);
    return !permissionValues.empty();
}
} // namespace ExternalDeviceManager
} // namespace OHOS