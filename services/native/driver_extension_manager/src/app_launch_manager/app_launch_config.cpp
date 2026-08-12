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

#include "app_launch_config.h"

#include <cJSON.h>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <climits>

#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {

void AppLaunchConfig::Trim(std::string &inputStr)
{
    if (inputStr.empty()) {
        return;
    }
    inputStr.erase(inputStr.begin(), std::find_if(inputStr.begin(), inputStr.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
    inputStr.erase(std::find_if(inputStr.rbegin(), inputStr.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), inputStr.end());
}

bool AppLaunchConfig::IsNumber(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(),
        [](unsigned char c) { return std::isdigit(c); });
}

std::vector<std::string> AppLaunchConfig::SplitString(const std::string &str, char pattern)
{
    std::stringstream iss(str);
    std::vector<std::string> result;
    std::string token;
    while (getline(iss, token, pattern)) {
        result.emplace_back(token);
    }
    return result;
}

std::vector<std::string> AppLaunchConfig::GetVersionNums(const std::string &filePath)
{
    struct stat fileStat;
    int ret = stat(filePath.c_str(), &fileStat);
    if (ret != 0) {
        EDM_LOGE(MODULE_BUS_USB, "stat failed, errno=%{public}d(%{public}s), path=%{public}s",
            errno, strerror(errno), filePath.c_str());
        return {};
    }
    EDM_LOGI(MODULE_BUS_USB, "stat success, size=%{public}ld, path=%{public}s",
        static_cast<long>(fileStat.st_size), filePath.c_str());
    if (fileStat.st_size <= 0 || fileStat.st_size > MAX_VERSION_FILE_LEN) {
        EDM_LOGE(MODULE_BUS_USB, "version file stat failed or invalid size: %{public}s", filePath.c_str());
        return {};
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        EDM_LOGE(MODULE_BUS_USB, "failed to open version file: %{public}s", filePath.c_str());
        return {};
    }

    std::string line;
    std::getline(file, line);
    file.close();

    if (line.empty()) {
        EDM_LOGE(MODULE_BUS_USB, "version file line is empty: %{public}s", filePath.c_str());
        return {};
    }

    std::vector<std::string> versionStr = SplitString(line, '=');
    const size_t expectedSize = 2;
    if (versionStr.size() != expectedSize) {
        EDM_LOGE(MODULE_BUS_USB, "version format invalid, expected 'key=value': %{public}s", filePath.c_str());
        return {};
    }

    if (versionStr[1].empty()) {
        EDM_LOGE(MODULE_BUS_USB, "version value is empty: %{public}s", filePath.c_str());
        return {};
    }

    Trim(versionStr[1]);
    std::vector<std::string> versionNum = SplitString(versionStr[1], '.');
    EDM_LOGI(MODULE_BUS_USB, "read version from %{public}s: %{public}s",
        filePath.c_str(), versionStr[1].c_str());
    return versionNum;
}

bool AppLaunchConfig::CompareVersion(const std::vector<std::string> &localVersion,
    const std::vector<std::string> &cloudVersion)
{
    if (localVersion.size() != VERSION_LEN || cloudVersion.size() != VERSION_LEN) {
        EDM_LOGI(MODULE_BUS_USB, "version segment count mismatch, use preset path");
        return false;
    }

    for (int32_t i = 0; i < VERSION_LEN; i++) {
        if (localVersion[i] != cloudVersion[i]) {
            if (!IsNumber(localVersion[i]) || !IsNumber(cloudVersion[i])) {
                EDM_LOGE(MODULE_BUS_USB, "version segment contains non-numeric chars");
                return false;
            }
            errno = 0;
            long localValue = std::strtol(localVersion[i].c_str(), nullptr, 10);
            if (errno == ERANGE || localValue > INT32_MAX || localValue < 0) {
                EDM_LOGE(MODULE_BUS_USB, "local version segment overflow: %{public}s", localVersion[i].c_str());
                return false;
            }
            errno = 0;
            long cloudValue = std::strtol(cloudVersion[i].c_str(), nullptr, 10);
            if (errno == ERANGE || cloudValue > INT32_MAX || cloudValue < 0) {
                EDM_LOGE(MODULE_BUS_USB, "cloud version segment overflow: %{public}s", cloudVersion[i].c_str());
                return false;
            }
            return localValue < cloudValue;
        }
    }

    EDM_LOGI(MODULE_BUS_USB, "versions equal, cloud path takes priority");
    return true;
}

std::string AppLaunchConfig::GetHigherVersionPath()
{
    std::string localVersionFile = std::string(PRESET_CONFIG_DATA_PATH) + VERSION_FILE_NAME;
    std::string cloudVersionFile = std::string(CONFIG_DATA_PATH) + VERSION_FILE_NAME;
    std::vector<std::string> localVersionNums = GetVersionNums(localVersionFile);
    std::vector<std::string> cloudVersionNums = GetVersionNums(cloudVersionFile);

    bool localValid = (localVersionNums.size() == VERSION_LEN);
    bool cloudValid = (cloudVersionNums.size() == VERSION_LEN);
    if (!localValid && cloudValid) {
        EDM_LOGI(MODULE_BUS_USB, "local version invalid, cloud version valid, use cloud path");
        return CONFIG_DATA_PATH;
    }
    if (localValid && !cloudValid) {
        EDM_LOGI(MODULE_BUS_USB, "cloud version invalid, local version valid, use preset path");
        return PRESET_CONFIG_DATA_PATH;
    }
    if (!localValid && !cloudValid) {
        EDM_LOGW(MODULE_BUS_USB, "both versions invalid, fallback to cloud path");
        return CONFIG_DATA_PATH;
    }
    if (CompareVersion(localVersionNums, cloudVersionNums)) {
        EDM_LOGI(MODULE_BUS_USB, "cloud version higher or equal, use cloud path");
        return CONFIG_DATA_PATH;
    }
    EDM_LOGI(MODULE_BUS_USB, "local version higher, use preset path");
    return PRESET_CONFIG_DATA_PATH;
}

std::string AppLaunchConfig::MakeKey(uint16_t vid, uint16_t pid)
{
    return std::to_string(vid) + "_" + std::to_string(pid);
}

bool AppLaunchConfig::LoadConfig()
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter", __func__);
    std::string effectivePath = GetHigherVersionPath() + CONFIG_FILE_NAME;
    EDM_LOGI(MODULE_BUS_USB, "effective config path: %{public}s", effectivePath.c_str());
    std::unordered_map<std::string, AppLaunchEntry> newMap;
    if (!ParseConfigFileTo(effectivePath, newMap)) {
        return false;
    }
    std::lock_guard<std::mutex> lock(configMutex_);
    entryMap_.swap(newMap);
    return true;
}

std::optional<AppLaunchEntry> AppLaunchConfig::FindEntry(uint16_t vid, uint16_t pid)
{
    EDM_LOGI(MODULE_BUS_USB, "%{public}s enter, vid=%{public}04x, pid=%{public}04x", __func__, vid, pid);
    std::lock_guard<std::mutex> lock(configMutex_);
    for (const auto &item : entryMap_) {
        EDM_LOGI(MODULE_BUS_USB, "entryMap_ key=%{public}s vid=%{public}04x pid=%{public}04x bundle=%{public}s",
            item.first.c_str(), item.second.vid, item.second.pid, item.second.bundleName.c_str());
    }
    std::string key = MakeKey(vid, pid);
    std::unordered_map<std::string, AppLaunchEntry>::iterator it = entryMap_.find(key);
    if (it != entryMap_.end()) {
        return it->second;
    }
    EDM_LOGI(MODULE_BUS_USB, "entry not found for key %{public}s", key.c_str());
    return std::nullopt;
}

cJSON *AppLaunchConfig::ParseJsonRoot(const std::string &filePath)
{
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        EDM_LOGE(MODULE_BUS_USB, "stat config file failed, errno=%{public}d(%{public}s), path=%{public}s",
            errno, strerror(errno), filePath.c_str());
        return nullptr;
    }
    if (fileStat.st_size <= 0 || fileStat.st_size > MAX_CONFIG_FILE_LEN) {
        EDM_LOGE(MODULE_BUS_USB, "config file size=%{public}ld invalid, path=%{public}s",
            static_cast<long>(fileStat.st_size), filePath.c_str());
        return nullptr;
    }

    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        EDM_LOGE(MODULE_BUS_USB, "failed to open config file %{public}s", filePath.c_str());
        return nullptr;
    }

    std::string content(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});
    file.close();
    EDM_LOGI(MODULE_BUS_USB, "config file content length=%{public}zu", content.size());

    cJSON *root = cJSON_Parse(content.c_str());
    if (root == nullptr) {
        const char *errPtr = cJSON_GetErrorPtr();
        EDM_LOGE(MODULE_BUS_USB, "failed to parse config json %{public}s, error at: %{public}s",
            filePath.c_str(), errPtr ? errPtr : "unknown");
        return nullptr;
    }

    if (cJSON_IsNull(root) || !cJSON_IsArray(root)) {
        EDM_LOGE(MODULE_BUS_USB, "config json root is not array %{public}s", filePath.c_str());
        cJSON_Delete(root);
        return nullptr;
    }
    return root;
}

bool AppLaunchConfig::ParseStringField(cJSON *parent, const char *fieldName, std::string &out)
{
    cJSON *obj = cJSON_GetObjectItemCaseSensitive(parent, fieldName);
    if (obj && cJSON_IsString(obj)) {
        std::string value = obj->valuestring;
        Trim(value);
        if (!value.empty()) {
            out = value;
            return true;
        }
    }
    EDM_LOGE(MODULE_BUS_USB, "device entry %{public}s missing or invalid, skipping", fieldName);
    return false;
}

bool AppLaunchConfig::ParseDeviceItemTo(cJSON *deviceItem,
    std::unordered_map<std::string, AppLaunchEntry> &outMap)
{
    cJSON *vidObj = cJSON_GetObjectItemCaseSensitive(deviceItem, "vid");
    cJSON *pidObj = cJSON_GetObjectItemCaseSensitive(deviceItem, "pid");
    if (!vidObj || !cJSON_IsString(vidObj) || !pidObj || !cJSON_IsString(pidObj)) {
        EDM_LOGE(MODULE_BUS_USB, "device entry vid or pid missing or invalid");
        return false;
    }

    errno = 0;
    unsigned long vidVal = std::strtoul(vidObj->valuestring, nullptr, 16);
    if (errno != 0 || vidVal > UINT16_MAX) {
        EDM_LOGE(MODULE_BUS_USB, "vid value invalid: %{public}s", vidObj->valuestring);
        return false;
    }
    errno = 0;
    unsigned long pidVal = std::strtoul(pidObj->valuestring, nullptr, 16);
    if (errno != 0 || pidVal > UINT16_MAX) {
        EDM_LOGE(MODULE_BUS_USB, "pid value invalid: %{public}s", pidObj->valuestring);
        return false;
    }

    AppLaunchEntry entry;
    entry.vid = static_cast<uint16_t>(vidVal);
    entry.pid = static_cast<uint16_t>(pidVal);

    if (!ParseStringField(deviceItem, "bundleName", entry.bundleName) ||
        !ParseStringField(deviceItem, "abilityName", entry.abilityName) ||
        !ParseStringField(deviceItem, "appStoreUri", entry.appStoreUri)) {
        return false;
    }

    std::string key = MakeKey(entry.vid, entry.pid);
    EDM_LOGI(MODULE_BUS_USB, "loaded entry: vid=%{public}04x pid=%{public}04x bundle=%{public}s",
        entry.vid, entry.pid, entry.bundleName.c_str());
    outMap[key] = std::move(entry);
    return true;
}

bool AppLaunchConfig::ParseConfigFileTo(const std::string &filePath,
    std::unordered_map<std::string, AppLaunchEntry> &outMap)
{
    cJSON *devicesArray = ParseJsonRoot(filePath);
    if (devicesArray == nullptr) {
        return false;
    }

    cJSON *deviceItem = nullptr;
    cJSON_ArrayForEach(deviceItem, devicesArray)
    {
        if (!ParseDeviceItemTo(deviceItem, outMap)) {
            EDM_LOGW(MODULE_BUS_USB, "a device item parse failed, skipped");
        }
    }

    cJSON_Delete(devicesArray);
    EDM_LOGI(MODULE_BUS_USB, "parsed %{public}zu entries from config", outMap.size());
    return true;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
