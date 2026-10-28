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

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include "cJSON.h"
#include "event_config.h"
#include "hilog_wrapper.h"
#include "file_ex.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;

constexpr int32_t ITERATION_FREQUENCY = 100;
constexpr int32_t REPETITION_FREQUENCY = 3;

static const string LOCAL_JSON_STR = R"(
[
    {
        "domain": "USB",
        "fault": [
            {
                "faultName": "TRANSFOR_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            }
        ]
    }
]
)";

static const string CCM_JSON_STR = R"(
[
    {
        "domain": "USB",
        "fault": [
            {
                "faultName": "TRANSFOR_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            },
            {
                "faultName": "READ_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            }
        ]
    },
    {
        "domain": "MOUSE",
        "fault": [
            {
                "faultName": "TRANSFOR_FAULT",
                "type": "FAULT",
                "title": "usb_transmission_error_title",
                "msg": "usb_troubleshoot_message",
                "uri": "www.gitee.com"
            }
        ]
    }
]
)";

static const string LOCAL_JSON_FILE_PATH = "./benchmark_local_event_config.json";
static const string CCM_JSON_FILE_PATH = "./benchmark_ccm_event_config.json";

class EventConfigBenchmarkTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State &state);
    void TearDown(const ::benchmark::State &state);
    static void DeleteFile(const string &filePath);
};

void EventConfigBenchmarkTest::DeleteFile(const string &filePath)
{
    if (access(filePath.c_str(), F_OK) == 0) {
        remove(filePath.c_str());
    }
}

void EventConfigBenchmarkTest::SetUp(const ::benchmark::State &state)
{
    SaveStringToFile(LOCAL_JSON_FILE_PATH, LOCAL_JSON_STR);
    SaveStringToFile(CCM_JSON_FILE_PATH, CCM_JSON_STR);
}

void EventConfigBenchmarkTest::TearDown(const ::benchmark::State &state)
{
    DeleteFile(LOCAL_JSON_FILE_PATH);
    DeleteFile(CCM_JSON_FILE_PATH);
}

/**
 * @tc.name: GetInstance
 * @tc.desc: Test functions to GetInstance benchmark test
 * @tc.desc: static EventConfig &GetInstance();
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, GetInstance)(benchmark::State &state)
{
    EventConfig *config;
    for (auto _ : state) {
        config = &EventConfig::GetInstance();
    }
    ASSERT_NE(config, nullptr);
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, GetInstance)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: ParseJsonFile
 * @tc.desc: Test functions to ParseJsonFile benchmark test
 * @tc.desc: bool ParseJsonFile(const std::string &targetPath, DomainFaultsMap &peripheralFaultNap);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, ParseJsonFile)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    DomainFaultsMap peripheralFaultsMap;
    bool result;
    for (auto _ : state) {
        peripheralFaultsMap.clear();
        result = eventConfig.ParseJsonFile(LOCAL_JSON_FILE_PATH, peripheralFaultsMap);
    }
    EXPECT_TRUE(result);
    EXPECT_GT(peripheralFaultsMap.size(), 0);
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, ParseJsonFile)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: ParseJsonFileNonExist
 * @tc.desc: Test functions to ParseJsonFile with non-exist file benchmark test
 * @tc.desc: bool ParseJsonFile(const std::string &targetPath, DomainFaultsMap &peripheralFaultNap);
 * @tc.desc: Negative test: non-exist file path
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, ParseJsonFileNonExist)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    DomainFaultsMap peripheralFaultsMap;
    bool result;
    for (auto _ : state) {
        peripheralFaultsMap.clear();
        result = eventConfig.ParseJsonFile("/non/exist/path/config.json", peripheralFaultsMap);
    }
    EXPECT_FALSE(result);
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, ParseJsonFileNonExist)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: ParseJsonFileNoArgs
 * @tc.desc: Test functions to ParseJsonFile() benchmark test
 * @tc.desc: bool ParseJsonFile();
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, ParseJsonFileNoArgs)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    bool result;
    for (auto _ : state) {
        result = eventConfig.ParseJsonFile();
    }
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, ParseJsonFileNoArgs)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: ExtractFaultInfo
 * @tc.desc: Test functions to ExtractFaultInfo benchmark test
 * @tc.desc: FaultInfo ExtractFaultInfo(const cJSON *faultItem);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, ExtractFaultInfo)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    const char *jsonStr = R"({"faultName":"TEST_FAULT","type":"FAULT",)"
                          R"("title":"test_title","msg":"test_msg",)"
                          R"("uri":"www.test.com"})";
    cJSON *faultItem = cJSON_Parse(jsonStr);
    ASSERT_NE(faultItem, nullptr);
    FaultInfo faultInfo;
    for (auto _ : state) {
        faultInfo = eventConfig.ExtractFaultInfo(faultItem);
    }
    EXPECT_EQ(faultInfo.faultName, "TEST_FAULT");
    cJSON_Delete(faultItem);
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, ExtractFaultInfo)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetFaultsInfoByDomain
 * @tc.desc: Test functions to GetFaultsInfoByDomain benchmark test
 * @tc.desc: std::vector<FaultInfo> GetFaultsInfoByDomain(const std::string &domain);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, GetFaultsInfoByDomain)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    DomainFaultsMap peripheralFaultsMap;
    eventConfig.ParseJsonFile(LOCAL_JSON_FILE_PATH, peripheralFaultsMap);
    eventConfig.peripheralFaultsMap_ = peripheralFaultsMap;
    vector<FaultInfo> result;
    for (auto _ : state) {
        result = eventConfig.GetFaultsInfoByDomain("USB");
    }
    EXPECT_FALSE(result.empty());
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, GetFaultsInfoByDomain)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetFaultsInfoByDomainNonExist
 * @tc.desc: Test functions to GetFaultsInfoByDomain with non-exist domain benchmark test
 * @tc.desc: std::vector<FaultInfo> GetFaultsInfoByDomain(const std::string &domain);
 * @tc.desc: Negative test: non-exist domain
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, GetFaultsInfoByDomainNonExist)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    DomainFaultsMap peripheralFaultsMap;
    eventConfig.ParseJsonFile(LOCAL_JSON_FILE_PATH, peripheralFaultsMap);
    eventConfig.peripheralFaultsMap_ = peripheralFaultsMap;
    vector<FaultInfo> result;
    for (auto _ : state) {
        result = eventConfig.GetFaultsInfoByDomain("NON_EXIST_DOMAIN");
    }
    EXPECT_TRUE(result.empty());
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, GetFaultsInfoByDomainNonExist)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetFaultInfo
 * @tc.desc: Test functions to GetFaultInfo benchmark test
 * @tc.desc: FaultInfo GetFaultInfo(const std::string &domain, const std::string &faultName) const;
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, GetFaultInfo)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    DomainFaultsMap peripheralFaultsMap;
    eventConfig.ParseJsonFile(LOCAL_JSON_FILE_PATH, peripheralFaultsMap);
    eventConfig.peripheralFaultsMap_ = peripheralFaultsMap;
    FaultInfo result;
    for (auto _ : state) {
        result = eventConfig.GetFaultInfo("USB", "TRANSFOR_FAULT");
    }
    EXPECT_EQ(result.faultName, "TRANSFOR_FAULT");
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, GetFaultInfo)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetFaultInfoNonExist
 * @tc.desc: Test functions to GetFaultInfo with non-exist fault benchmark test
 * @tc.desc: FaultInfo GetFaultInfo(const std::string &domain, const std::string &faultName) const;
 * @tc.desc: Negative test: non-exist faultName
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, GetFaultInfoNonExist)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    DomainFaultsMap peripheralFaultsMap;
    eventConfig.ParseJsonFile(LOCAL_JSON_FILE_PATH, peripheralFaultsMap);
    eventConfig.peripheralFaultsMap_ = peripheralFaultsMap;
    FaultInfo result;
    for (auto _ : state) {
        result = eventConfig.GetFaultInfo("USB", "NON_EXIST_FAULT");
    }
    EXPECT_TRUE(result.faultName.empty());
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, GetFaultInfoNonExist)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: FillFaultsMap
 * @tc.desc: Test functions to FillFaultsMap benchmark test
 * @tc.desc: DomainFaultsMap FillFaultsMap(const DomainFaultsMap &ccmMap, const DomainFaultsMap &localMap);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, FillFaultsMap)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    DomainFaultsMap ccmMap;
    DomainFaultsMap localMap;
    eventConfig.ParseJsonFile(LOCAL_JSON_FILE_PATH, localMap);
    eventConfig.ParseJsonFile(CCM_JSON_FILE_PATH, ccmMap);
    DomainFaultsMap result;
    for (auto _ : state) {
        result = eventConfig.FillFaultsMap(ccmMap, localMap);
    }
    EXPECT_FALSE(result.empty());
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, FillFaultsMap)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: DeleteJsonObj
 * @tc.desc: Test functions to DeleteJsonObj benchmark test
 * @tc.desc: void DeleteJsonObj(cJSON *obj);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EventConfigBenchmarkTest, DeleteJsonObj)(benchmark::State &state)
{
    EventConfig &eventConfig = EventConfig::GetInstance();
    for (auto _ : state) {
        cJSON *obj = cJSON_CreateObject();
        if (obj) {
            cJSON_AddStringToObject(obj, "key", "value");
        }
        eventConfig.DeleteJsonObj(obj);
    }
}
BENCHMARK_REGISTER_F(EventConfigBenchmarkTest, DeleteJsonObj)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();
} // namespace ExternalDeviceManager
} // namespace OHOS
BENCHMARK_MAIN();
