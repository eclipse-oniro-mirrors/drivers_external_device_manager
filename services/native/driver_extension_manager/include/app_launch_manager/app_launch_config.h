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

#ifndef APP_LAUNCH_CONFIG_H
#define APP_LAUNCH_CONFIG_H

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "app_launch_entry.h"

typedef struct cJSON cJSON;

namespace OHOS {
namespace ExternalDeviceManager {

class AppLaunchConfig {
public:
    AppLaunchConfig() = default;
    ~AppLaunchConfig() = default;

    bool LoadConfig();

    std::optional<AppLaunchEntry> FindEntry(uint16_t vid, uint16_t pid);

private:
    bool ParseConfigFileTo(const std::string &filePath,
        std::unordered_map<std::string, AppLaunchEntry> &outMap);
    cJSON *ParseJsonRoot(const std::string &filePath);
    bool ParseDeviceItemTo(cJSON *deviceItem,
        std::unordered_map<std::string, AppLaunchEntry> &outMap);
    bool ParseStringField(cJSON *parent, const char *fieldName, std::string &out);
    std::string MakeKey(uint16_t vid, uint16_t pid);

    std::vector<std::string> GetVersionNums(const std::string &filePath);
    bool CompareVersion(const std::vector<std::string> &localVersion,
        const std::vector<std::string> &cloudVersion);
    std::string GetHigherVersionPath();

    static void Trim(std::string &inputStr);
    static bool IsNumber(const std::string &str);
    static std::vector<std::string> SplitString(const std::string &str, char pattern);

    static constexpr const char *PRESET_CONFIG_DATA_PATH =
        "/system/etc/UsbWirelessDisplayAdapterApp/generic/";
    static constexpr const char *CONFIG_DATA_PATH =
        "/data/service/el1/public/update/param_service/install/system/etc/UsbWirelessDisplayAdapterApp/generic/";
    static constexpr const char *CONFIG_FILE_NAME = "app_launch_config.json";
    static constexpr const char *VERSION_FILE_NAME = "version.txt";
    static constexpr const char *VERSION_KEY = "version";
    static constexpr int32_t VERSION_LEN = 4;
    static constexpr int32_t MAX_VERSION_FILE_LEN = 1024 * 5;
    static constexpr int32_t MAX_CONFIG_FILE_LEN = 1024 * 1024;

    std::mutex configMutex_;
    std::unordered_map<std::string, AppLaunchEntry> entryMap_;
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // APP_LAUNCH_CONFIG_H
