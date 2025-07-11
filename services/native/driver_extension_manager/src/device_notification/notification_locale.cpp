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

#include "notification_locale.h"
#include "config_policy_utils.h"
#include "hilog_wrapper.h"
#include "locale_config.h"
#include "locale_matcher.h"

namespace {
constexpr const char *LOCALE_CONFIG_PATH = "/system/etc/peripheral/resources/locale_path.json";
constexpr const char *SYSTEM_PERIPHERAL_RESOURCE_PATH = "/system/etc/peripheral/resources/";
constexpr const char *ELEMENT_STRING_FILE = "/element/string.json";
constexpr const char *DEFAULT_LANGUAGE_EN = "base";
} // namespace

namespace OHOS {
namespace ExternalDeviceManager {
namespace {
bool IsPathValid(const std::string &path)
{
    // Check for directory traversal patterns
    if (path.find("..") != std::string::npos) {
        return false;
    }

    // Check for absolute paths
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) {
        return false;
    }

    // Check for other suspicious characters
    const std::string forbiddenChars = "\\:*?\"<>|";
    return path.find_first_of(forbiddenChars) == std::string::npos;
}
} // namespace

std::shared_ptr<NotificationLocale> NotificationLocale::instance_ = nullptr;
std::mutex NotificationLocale::instanceMutex_;
NotificationLocale &NotificationLocale::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (instance_ == nullptr) {
        instance_ = std::make_shared<NotificationLocale>();
    }
    return *(instance_.get());
}

bool NotificationLocale::ParseJsonfile(
    const std::string &targetPath, std::unordered_map<std::string, std::string> &container)
{
    if (access(targetPath.c_str(), F_OK) != 0) {
        EDM_LOGE(MODULE_SERVICE, "targetPath %{public}s invalid", targetPath.c_str());
        return false;
    }
    std::ifstream inputStream(targetPath.c_str(), std::ios::in | std::ios::binary);
    std::string fileStr(std::istreambuf_iterator<char> {inputStream}, std::istreambuf_iterator<char> {});
    cJSON *root = cJSON_Parse(fileStr.c_str());
    if (!root) {
        EDM_LOGE(MODULE_SERVICE, "%{public}s json parse error", targetPath.c_str());
        return false;
    }
    if (cJSON_IsNull(root) || !cJSON_IsObject(root)) {
        EDM_LOGE(MODULE_SERVICE, "%{public}s json root error", targetPath.c_str());
        cJSON_Delete(root);
        return false;
    }
    cJSON *stringConf = cJSON_GetObjectItemCaseSensitive(root, "string");
    if (!stringConf || cJSON_IsNull(stringConf) || !cJSON_IsArray(stringConf)) {
        EDM_LOGE(MODULE_SERVICE, "%{public}s stringConf invalid", targetPath.c_str());
        cJSON_Delete(root);
        return false;
    }
    cJSON *conf = nullptr;
    cJSON_ArrayForEach(conf, stringConf)
    {
        cJSON *nameObj = cJSON_GetObjectItemCaseSensitive(conf, "name");
        cJSON *valueObj = cJSON_GetObjectItemCaseSensitive(conf, "value");
        if (nameObj && valueObj && cJSON_IsString(nameObj) && cJSON_IsString(valueObj) &&
            (strlen(nameObj->valuestring) > 0) && (strlen(valueObj->valuestring) > 0)) {
            container.insert(std::make_pair(nameObj->valuestring, valueObj->valuestring));
        }
    }
    cJSON_Delete(root);
    return true;
}

void NotificationLocale::ParseLocaleCfg()
{
    if (islanguageMapInit_) {
        return;
    }
    languageMap_.clear();
    if (ParseJsonfile(LOCALE_CONFIG_PATH, languageMap_)) {
        islanguageMapInit_ = true;
    }
}

void NotificationLocale::UpdateStringMap()
{
    std::lock_guard<std::mutex> lock(localeMutex_);
    OHOS::Global::I18n::LocaleInfo locale(Global::I18n::LocaleConfig::GetSystemLocale());
    std::string curBaseName = locale.GetBaseName();
    if (localeBaseName_ == curBaseName) {
        return;
    }

    localeBaseName_ = curBaseName;
    std::string language = DEFAULT_LANGUAGE_EN;

    if (languageMap_.find(localeBaseName_) != languageMap_.end()) {
        language = languageMap_[localeBaseName_];
    }

    // Validate language path to prevent directory traversal
    if (!IsPathValid(language)) {
        EDM_LOGE(MODULE_SERVICE, "Invalid language path detected: %{public}s", language.c_str());
        language = DEFAULT_LANGUAGE_EN;
    }

    stringMap_.clear();
    std::string resourcePath = SYSTEM_PERIPHERAL_RESOURCE_PATH + language + ELEMENT_STRING_FILE;
    // Additional path validation
    if (resourcePath.find("/../") != std::string::npos) {
        EDM_LOGE(MODULE_SERVICE, "Potential path traversal attack detected");
        return;
    }
    ParseJsonfile(resourcePath, stringMap_);
}

std::string NotificationLocale::GetValueByKey(const std::string &key)
{
    std::lock_guard<std::mutex> lock(localeMutex_);
    auto iter = stringMap_.find(key);
    if (iter != stringMap_.end()) {
        return iter->second;
    }
    EDM_LOGE(MODULE_SERVICE, "fail to get related string by key(%{public}s)", key.c_str());
    return "";
}
} // namespace ExternalDeviceManager
} // namespace OHOS
