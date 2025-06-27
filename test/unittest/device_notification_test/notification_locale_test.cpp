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

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "locale_config.h"
#include "locale_matcher.h"
#include "notification_locale.h"
#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace ExternalDeviceManager {
static std::string g_localJson = R"(
{
    "string": [
        {
            "name": "usb_transmission_error_title",
            "value": "USB Transmission Error"
        },
        {
            "name": "usb_troubleshoot_message",
            "value": "Click to view error details and solutions"
        }
    ]
})";

const std::string localJsonFilePath = "./local_string.json";

class NotificationLocaleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NotificationLocaleTest::SetUpTestCase(void)
{
    std::ofstream ofs(localJsonFilePath);
    if (ofs.is_open()) {
        ofs << g_localJson;
        ofs.close();
    }
}
void NotificationLocaleTest::TearDownTestCase(void)
{
    if (access(localJsonFilePath.c_str(), F_OK) == 0) {
        if (remove(localJsonFilePath.c_str()) != 0) {
            EDM_LOGE(MODULE_SERVICE, "Failed to remove file: %{public}s", localJsonFilePath.c_str());
        }
    }
}
void NotificationLocaleTest::SetUp(void) {}
void NotificationLocaleTest::TearDown(void) {}

/**
 * @tc.name: ParseLocaleCfg001
 * @tc.desc: Test ParseLocaleCfg
 * @tc.type: FUNC
 */
HWTEST_F(NotificationLocaleTest, ParseLocaleCfg001, TestSize.Level1)
{
    EDM_LOGI(MODULE_SERVICE, "ParseLocaleCfg001 begin");
    auto &notificationLocale = NotificationLocale::GetInstance();
    EXPECT_NE(&notificationLocale, nullptr);
    EXPECT_TRUE(notificationLocale.languageMap_.size() == 0);

    std::unordered_map<std::string, std::string> originalLanguageMap;
    for (const auto &pair : notificationLocale.languageMap_) {
        originalLanguageMap[pair.first] = pair.second;
    }

    notificationLocale.islanguageMapInit_ = true;
    notificationLocale.ParseLocaleCfg();
    EXPECT_TRUE(notificationLocale.languageMap_.size() == 0);
    notificationLocale.islanguageMapInit_ = false;
    notificationLocale.ParseLocaleCfg();
    EXPECT_TRUE(notificationLocale.islanguageMapInit_);
    EXPECT_GT(notificationLocale.languageMap_.size(), 0);
    EDM_LOGI(MODULE_SERVICE, "ParseLocaleCfg001 end");
}

/**
 * @tc.name: GetStringByKey001
 * @tc.desc: Test GetStringByKey
 * @tc.type: FUNC
 */
HWTEST_F(NotificationLocaleTest, GetStringByKey001, TestSize.Level1)
{
    EDM_LOGI(MODULE_SERVICE, "GetStringByKey001 begin");
    auto &notificationLocale = NotificationLocale::GetInstance();
    EXPECT_NE(&notificationLocale, nullptr);
    notificationLocale.islanguageMapInit_ = false;
    notificationLocale.ParseLocaleCfg();
    notificationLocale.UpdateStringMap();
    EXPECT_GT(notificationLocale.languageMap_.size(), 0);
    EXPECT_GT(notificationLocale.stringMap_.size(), 0);
    EXPECT_TRUE(notificationLocale.GetStringByKey("key").empty());
    EXPECT_FALSE(notificationLocale.GetStringByKey("usb_transmission_error_title").empty());
    EDM_LOGI(MODULE_SERVICE, "GetStringByKey001 end");
}

/**
 * @tc.name: UpdateStringMap001
 * @tc.desc: Test UpdateStringMap
 * @tc.type: FUNC
 */
HWTEST_F(NotificationLocaleTest, UpdateStringMap001, TestSize.Level1)
{
    EDM_LOGI(MODULE_SERVICE, "UpdateStringMap001 begin");
    auto &notificationLocale = NotificationLocale::GetInstance();
    EXPECT_NE(&notificationLocale, nullptr);
    OHOS::Global::I18n::LocaleInfo locale(Global::I18n::LocaleConfig::GetSystemLocale());
    std::string curBaseName = locale.GetBaseName();
    std::string localeBaseNameBak = curBaseName;
    std::unordered_map<std::string, std::string> languageMap;
    notificationLocale.ParseLocaleCfg();
    EXPECT_TRUE(notificationLocale.islanguageMapInit_);
    EXPECT_GT(notificationLocale.languageMap_.size(), 0);

    notificationLocale.stringMap_.clear();
    // same langauge when the size of stringMap_ is 0. stringMap_ should be empty
    notificationLocale.UpdateStringMap();
    EXPECT_EQ(notificationLocale.stringMap_.size(), 0);

    // change langauge to en-Latn-US or zh-Hans-CN when the size of stringMap_ is 0.
    // after UpdateStringMap, stringMap_ should not be empty
    std::string language1 = "en-Latn-US";
    std::string language2 = "zh-Hans-CN";
    std::string language = language1;
    if (curBaseName == language1) {
        language = language2;
    }
    notificationLocale.localeBaseName_ = language;
    notificationLocale.UpdateStringMap();
    EXPECT_GT(notificationLocale.stringMap_.size(), 0);

    std::unordered_map<std::string, std::string> stringMapBak;
    for (const auto &pair : notificationLocale.stringMap_) {
        stringMapBak[pair.first] = pair.second;
    }

    // change language between en-Latn-US and zh-Hans-CN. stringMap_ should have same size and same content
    if (notificationLocale.localeBaseName_ == language1) {
        notificationLocale.localeBaseName_ = language2;
    } else {
        notificationLocale.localeBaseName_ = language1;
    }

    notificationLocale.UpdateStringMap();
    EXPECT_GT(notificationLocale.stringMap_.size(), 0);
    EXPECT_EQ(notificationLocale.stringMap_.size(), stringMapBak.size());
    std::string key = "usb_transmission_error_title";
    std::string value = notificationLocale.GetStringByKey(key);
    EXPECT_EQ(value, stringMapBak[key]);

    // restore
    if (notificationLocale.localeBaseName_ != localeBaseNameBak) {
        notificationLocale.localeBaseName_ = localeBaseNameBak;
        notificationLocale.UpdateStringMap();
    }
    EDM_LOGI(MODULE_SERVICE, "UpdateStringMap001 end");
}

/**
 * @tc.name: ParseJsonfile001
 * @tc.desc: Test ParseJsonfile
 * @tc.type: FUNC
 */
HWTEST_F(NotificationLocaleTest, ParseJsonfile001, TestSize.Level1)
{
    EDM_LOGI(MODULE_SERVICE, "ParseJsonfile001 begin");
    auto &notificationLocale = NotificationLocale::GetInstance();
    EXPECT_NE(&notificationLocale, nullptr);
    std::unordered_map<std::string, std::string> languageMap;
    std::string path = "";
    bool bRet = notificationLocale.ParseJsonfile(path, languageMap);
    EXPECT_FALSE(bRet);

    bRet = notificationLocale.ParseJsonfile(localJsonFilePath, languageMap);
    EXPECT_TRUE(bRet);
    EXPECT_GT(languageMap.size(), 0);
    EDM_LOGI(MODULE_SERVICE, "ParseJsonfile001 end");
}
}
}
