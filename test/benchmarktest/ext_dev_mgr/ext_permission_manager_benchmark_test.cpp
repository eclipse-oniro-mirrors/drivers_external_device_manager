/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include <unordered_set>
#include "accesstoken_kit.h"
#include "ext_permission_manager.h"
#include "hilog_wrapper.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace OHOS::Security::AccessToken;

constexpr int32_t ITERATION_FREQUENCY = 100;
constexpr int32_t REPETITION_FREQUENCY = 3;

static void GetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.ACCESS_EXTENSIONAL_DEVICE_DRIVER";

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "BenchmarkTest";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

class ExtPermissionManagerBenchmarkTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State &state);
    void TearDown(const ::benchmark::State &state);
};

void ExtPermissionManagerBenchmarkTest::SetUp(const ::benchmark::State &state)
{
    GetNativeToken();
}

void ExtPermissionManagerBenchmarkTest::TearDown(const ::benchmark::State &state) {}

/**
 * @tc.name: VerifyPermission
 * @tc.desc: Test functions to VerifyPermission benchmark test
 * @tc.desc: static bool VerifyPermission(std::string permissionName);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(ExtPermissionManagerBenchmarkTest, VerifyPermission)(benchmark::State &state)
{
    bool result;
    for (auto _ : state) {
        result = ExtPermissionManager::VerifyPermission("ohos.permission.ACCESS_EXTENSIONAL_DEVICE_DRIVER");
    }
    EXPECT_TRUE(result);
}
BENCHMARK_REGISTER_F(ExtPermissionManagerBenchmarkTest, VerifyPermission)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: VerifyPermissionInvalid
 * @tc.desc: Test functions to VerifyPermission with invalid permission benchmark test
 * @tc.desc: static bool VerifyPermission(std::string permissionName);
 * @tc.desc: Negative test: invalid permission name
 * @tc.type: FUNC
 */
BENCHMARK_F(ExtPermissionManagerBenchmarkTest, VerifyPermissionInvalid)(benchmark::State &state)
{
    bool result;
    for (auto _ : state) {
        result = ExtPermissionManager::VerifyPermission("ohos.permission.INVALID_PERMISSION");
    }
    EXPECT_FALSE(result);
}
BENCHMARK_REGISTER_F(ExtPermissionManagerBenchmarkTest, VerifyPermissionInvalid)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: IsSystemAppOrSa
 * @tc.desc: Test functions to IsSystemAppOrSa benchmark test
 * @tc.desc: static bool IsSystemAppOrSa(void);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(ExtPermissionManagerBenchmarkTest, IsSystemAppOrSa)(benchmark::State &state)
{
    bool result;
    for (auto _ : state) {
        result = ExtPermissionManager::IsSystemAppOrSa();
    }
    EXPECT_TRUE(result);
}
BENCHMARK_REGISTER_F(ExtPermissionManagerBenchmarkTest, IsSystemAppOrSa)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetCallingTokenID
 * @tc.desc: Test functions to GetCallingTokenID benchmark test
 * @tc.desc: static uint32_t GetCallingTokenID(void);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(ExtPermissionManagerBenchmarkTest, GetCallingTokenID)(benchmark::State &state)
{
    uint32_t tokenId;
    for (auto _ : state) {
        tokenId = ExtPermissionManager::GetCallingTokenID();
    }
    EXPECT_NE(0u, tokenId);
}
BENCHMARK_REGISTER_F(ExtPermissionManagerBenchmarkTest, GetCallingTokenID)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetPermissionValues
 * @tc.desc: Test functions to GetPermissionValues benchmark test
 * @tc.desc: static bool GetPermissionValues(const std::string &permissionName,
 *             std::unordered_set<std::string> &permissionValues);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(ExtPermissionManagerBenchmarkTest, GetPermissionValues)(benchmark::State &state)
{
    unordered_set<string> permissionValues;
    bool result;
    for (auto _ : state) {
        permissionValues.clear();
        result = ExtPermissionManager::GetPermissionValues("ohos.permission.ACCESS_DDK_DRIVERS",
            permissionValues);
    }
}
BENCHMARK_REGISTER_F(ExtPermissionManagerBenchmarkTest, GetPermissionValues)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetHapTokenInfo
 * @tc.desc: Test functions to GetHapTokenInfo benchmark test
 * @tc.desc: static bool GetHapTokenInfo(uint32_t tokenId, Security::AccessToken::HapTokenInfo &hapTokenInfo);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(ExtPermissionManagerBenchmarkTest, GetHapTokenInfo)(benchmark::State &state)
{
    uint32_t tokenId = ExtPermissionManager::GetCallingTokenID();
    HapTokenInfo hapTokenInfo;
    bool result;
    for (auto _ : state) {
        result = ExtPermissionManager::GetHapTokenInfo(tokenId, hapTokenInfo);
    }
}
BENCHMARK_REGISTER_F(ExtPermissionManagerBenchmarkTest, GetHapTokenInfo)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: GetHapTokenInfoInvalid
 * @tc.desc: Test functions to GetHapTokenInfo with invalid tokenId benchmark test
 * @tc.desc: static bool GetHapTokenInfo(uint32_t tokenId, Security::AccessToken::HapTokenInfo &hapTokenInfo);
 * @tc.desc: Negative test: invalid tokenId
 * @tc.type: FUNC
 */
BENCHMARK_F(ExtPermissionManagerBenchmarkTest, GetHapTokenInfoInvalid)(benchmark::State &state)
{
    HapTokenInfo hapTokenInfo;
    bool result;
    for (auto _ : state) {
        result = ExtPermissionManager::GetHapTokenInfo(0, hapTokenInfo);
    }
    EXPECT_FALSE(result);
}
BENCHMARK_REGISTER_F(ExtPermissionManagerBenchmarkTest, GetHapTokenInfoInvalid)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

} // namespace ExternalDeviceManager
} // namespace OHOS
BENCHMARK_MAIN();
