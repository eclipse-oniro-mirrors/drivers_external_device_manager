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

#ifndef EVENT_CONFIG_H
#define EVENT_CONFIG_H

#include <fstream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include <cJSON.h>
#include <unistd.h>

namespace OHOS {
namespace ExternalDeviceManager {
struct FaultInfo {
    std::string faultName;
    std::string type;
    std::string title;
    std::string msg;
    std::string uri;
    std::string GetInfo() const
    {
        return "faultName: " + faultName + ", type: " + type + ", title: " + title + ", msg: " + msg +
            ", uri: " + uri;
    }
};

class EventConfig {
public:
    static EventConfig &GetInstance();
    FaultInfo ExtractFaultInfo(const cJSON *faultItem);
    std::vector<FaultInfo> GetFaultsInfoByDomain(const std::string &domain);
    bool ParseJsonFile();
    FaultInfo GetFaultInfo(const std::string &domain, const std::string &faultName) const;

private:
    void DeleteJsonObj(cJSON *obj);
    bool ParseJsonFile(
        const std::string &targetPath, std::unordered_map<std::string, std::vector<FaultInfo>> &peripheralFaultNap);
    // domain -> FaultInfo vector map
    std::unordered_map<std::string, std::vector<FaultInfo>> peripheralFaultsMap_;
    static std::mutex mutex_;
    static std::shared_ptr<EventConfig> instance_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS
#endif // EVENT_CONFIG_H
