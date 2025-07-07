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

#ifndef NOTIFICATION_LOCALE_H
#define NOTIFICATION_LOCALE_H

#include "nocopyable.h"
#include <atomic>
#include <cJSON.h>
#include <fstream>
#include <mutex>
#include <string>
#include <unistd.h>
#include <unordered_map>

namespace OHOS {
namespace ExternalDeviceManager {
class NotificationLocale : public NoCopyable {
public:
    static NotificationLocale &GetInstance();
    void ParseLocaleCfg();
    void UpdateStringMap();
    std::string GetValueByKey(const std::string &key);
private:
    bool ParseJsonfile(const std::string &targetPath, std::unordered_map<std::string, std::string> &container);
    std::unordered_map<std::string, std::string> languageMap_;
    std::unordered_map<std::string, std::string> stringMap_;
    std::string localeBaseName_;
    std::atomic<bool> islanguageMapInit_ {false};
    mutable std::mutex localeMutex_;
    static std::mutex instanceMutex_;
    static std::shared_ptr<NotificationLocale> instance_;
};
} // namespace ExternalDeviceManager
} // namespace OHOS

#endif // NOTIFICATION_LOCALE_H
