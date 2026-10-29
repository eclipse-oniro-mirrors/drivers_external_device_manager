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

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <unordered_map>
#include <unordered_set>
#include "edm_errors.h"
#include "ext_object.h"
#include "hilog_wrapper.h"
#include "idriver_change_callback.h"
#include "driver_ext_mgr_callback_stub.h"
#define private public
#include "driver_pkg_manager.h"
#include "etx_device_mgr.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;

constexpr int32_t ITERATION_FREQUENCY = 100;
constexpr int32_t REPETITION_FREQUENCY = 3;
constexpr uint32_t TEST_BUS_DEVICE_ID_BASE = 1000;
constexpr uint32_t TEST_DEVICE_COUNT = 10;

class MockDriverChangeCallback : public IDriverChangeCallback {
public:
    void OnDriverMatched(const std::shared_ptr<DriverInfo> &driverInfo) override {}
    void OnDriverRemoved(const std::shared_ptr<DriverInfo> &driverInfo) override {}
};

class MockDriverExtMgrCallback : public DriverExtMgrCallbackStub {
public:
    ErrCode OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override
    {
        return ERR_OK;
    }
    ErrCode OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override
    {
        return ERR_OK;
    }
    ErrCode OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override
    {
        return ERR_OK;
    }
};

class EtxDeviceMgrBenchmarkTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State &state);
    void TearDown(const ::benchmark::State &state);

    static void RegisterTestDevices();
    static void ClearTestDevices();

    static vector<shared_ptr<DeviceInfo>> testDevices_;
};

vector<shared_ptr<DeviceInfo>> EtxDeviceMgrBenchmarkTest::testDevices_;

void EtxDeviceMgrBenchmarkTest::RegisterTestDevices()
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    testDevices_.clear();
    for (uint32_t i = 0; i < TEST_DEVICE_COUNT; ++i) {
        auto devInfo = std::make_shared<DeviceInfo>(TEST_BUS_DEVICE_ID_BASE + i, BusType::BUS_TYPE_TEST,
            "benchmark_test_device");
        testDevices_.push_back(devInfo);
        mgr.RegisterDevice(devInfo);
    }
}

void EtxDeviceMgrBenchmarkTest::ClearTestDevices()
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    for (auto &devInfo : testDevices_) {
        mgr.UnRegisterDevice(devInfo);
    }
    testDevices_.clear();
    unordered_map<BusType, unordered_map<uint64_t, shared_ptr<Device>>> emptyMap;
    mgr.deviceMap_ = emptyMap;
    unordered_map<string, unordered_set<uint64_t>> emptyBundleMap;
    mgr.bundleMatchMap_ = emptyBundleMap;
}

void EtxDeviceMgrBenchmarkTest::SetUp(const ::benchmark::State &state)
{
    DriverPkgManager::GetInstance().Init();
    ExtDeviceManager::GetInstance().Init();
    RegisterTestDevices();
}

void EtxDeviceMgrBenchmarkTest::TearDown(const ::benchmark::State &state)
{
    ClearTestDevices();
}

/**
 * @tc.name: Init
 * @tc.desc: Test functions to Init benchmark test
 * @tc.desc: int32_t Init();
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, Init)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    int32_t ret;
    for (auto _ : state) {
        ret = mgr.Init();
    }
    EXPECT_EQ(EDM_OK, ret);
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, Init)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: RegisterDevice
 * @tc.desc: Test functions to RegisterDevice benchmark test
 * @tc.desc: int32_t RegisterDevice(shared_ptr<DeviceInfo> devInfo);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, RegisterDevice)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint32_t idx = 0;
    int32_t ret;
    for (auto _ : state) {
        auto devInfo = std::make_shared<DeviceInfo>(TEST_BUS_DEVICE_ID_BASE + TEST_DEVICE_COUNT + idx,
            BusType::BUS_TYPE_TEST, "benchmark_register_device");
        ret = mgr.RegisterDevice(devInfo);
        mgr.UnRegisterDevice(devInfo);
        idx++;
    }
    EXPECT_EQ(EDM_OK, ret);
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, RegisterDevice)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: UnRegisterDevice
 * @tc.desc: Test functions to UnRegisterDevice benchmark test
 * @tc.desc: int32_t UnRegisterDevice(const shared_ptr<DeviceInfo> devInfo);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, UnRegisterDevice)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint32_t idx = 0;
    int32_t ret;
    for (auto _ : state) {
        auto devInfo = std::make_shared<DeviceInfo>(TEST_BUS_DEVICE_ID_BASE + TEST_DEVICE_COUNT + 100 + idx,
            BusType::BUS_TYPE_TEST, "benchmark_unregister_device");
        mgr.RegisterDevice(devInfo);
        ret = mgr.UnRegisterDevice(devInfo);
        idx++;
    }
    EXPECT_EQ(EDM_OK, ret);
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, UnRegisterDevice)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: QueryDevice
 * @tc.desc: Test functions to QueryDevice benchmark test
 * @tc.desc: vector<shared_ptr<DeviceInfo>> QueryDevice(const BusType busType);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, QueryDevice)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    vector<shared_ptr<DeviceInfo>> result;
    for (auto _ : state) {
        result = mgr.QueryDevice(BusType::BUS_TYPE_TEST);
    }
    EXPECT_EQ(testDevices_.size(), result.size());
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, QueryDevice)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: QueryAllDevices
 * @tc.desc: Test functions to QueryAllDevices benchmark test
 * @tc.desc: vector<shared_ptr<Device>> QueryAllDevices();
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, QueryAllDevices)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    vector<shared_ptr<Device>> result;
    for (auto _ : state) {
        result = mgr.QueryAllDevices();
    }
    EXPECT_EQ(testDevices_.size(), result.size());
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, QueryAllDevices)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: QueryDevicesById
 * @tc.desc: Test functions to QueryDevicesById benchmark test
 * @tc.desc: vector<shared_ptr<Device>> QueryDevicesById(const uint64_t deviceId);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, QueryDevicesById)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint64_t deviceId = testDevices_[0]->GetDeviceId();
    vector<shared_ptr<Device>> result;
    for (auto _ : state) {
        result = mgr.QueryDevicesById(deviceId);
    }
    EXPECT_FALSE(result.empty());
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, QueryDevicesById)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: ConnectDevice
 * @tc.desc: Test functions to ConnectDevice benchmark test
 * @tc.desc: int32_t ConnectDevice(uint64_t deviceId, uint32_t callingTokenId,
 *             const sptr<IDriverExtMgrCallback> &connectCallback);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, ConnectDevice)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint64_t deviceId = testDevices_[0]->GetDeviceId();
    sptr<MockDriverExtMgrCallback> callback = new MockDriverExtMgrCallback();
    ASSERT_NE(callback, nullptr);
    int32_t ret;
    for (auto _ : state) {
        ret = mgr.ConnectDevice(deviceId, 0, callback);
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, ConnectDevice)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: DisConnectDevice
 * @tc.desc: Test functions to DisConnectDevice benchmark test
 * @tc.desc: int32_t DisConnectDevice(uint64_t deviceId, uint32_t callingTokenId);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, DisConnectDevice)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint64_t deviceId = testDevices_[0]->GetDeviceId();
    int32_t ret;
    for (auto _ : state) {
        ret = mgr.DisConnectDevice(deviceId, 0);
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, DisConnectDevice)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: ConnectDriverWithDeviceId
 * @tc.desc: Test functions to ConnectDriverWithDeviceId benchmark test
 * @tc.desc: int32_t ConnectDriverWithDeviceId(uint64_t deviceId, uint32_t callingTokenId,
 *             const unordered_set<std::string> &accessibleAppIds,
 *             const sptr<IDriverExtMgrCallback> &connectCallback);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, ConnectDriverWithDeviceId)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint64_t deviceId = testDevices_[0]->GetDeviceId();
    unordered_set<string> accessibleAppIds = {"testAppId1", "testAppId2"};
    sptr<MockDriverExtMgrCallback> callback = new MockDriverExtMgrCallback();
    ASSERT_NE(callback, nullptr);
    int32_t ret;
    for (auto _ : state) {
        ret = mgr.ConnectDriverWithDeviceId(deviceId, 0, accessibleAppIds, callback);
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, ConnectDriverWithDeviceId)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: DisConnectDriverWithDeviceId
 * @tc.desc: Test functions to DisConnectDriverWithDeviceId benchmark test
 * @tc.desc: int32_t DisConnectDriverWithDeviceId(uint64_t deviceId, uint32_t callingTokenId);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, DisConnectDriverWithDeviceId)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint64_t deviceId = testDevices_[0]->GetDeviceId();
    int32_t ret;
    for (auto _ : state) {
        ret = mgr.DisConnectDriverWithDeviceId(deviceId, 0);
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, DisConnectDriverWithDeviceId)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: DeleteBundlesOfBundleInfoMap
 * @tc.desc: Test functions to DeleteBundlesOfBundleInfoMap benchmark test
 * @tc.desc: std::unordered_set<uint64_t> DeleteBundlesOfBundleInfoMap(const std::string &bundleName = "");
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, DeleteBundlesOfBundleInfoMap)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    unordered_set<uint64_t> result;
    for (auto _ : state) {
        result = mgr.DeleteBundlesOfBundleInfoMap("benchmark_test_bundle");
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, DeleteBundlesOfBundleInfoMap)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: SetDriverChangeCallback
 * @tc.desc: Test functions to SetDriverChangeCallback benchmark test
 * @tc.desc: void SetDriverChangeCallback(shared_ptr<IDriverChangeCallback> &driverChangeCallback);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, SetDriverChangeCallback)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    for (auto _ : state) {
        shared_ptr<IDriverChangeCallback> callback = std::make_shared<MockDriverChangeCallback>();
        mgr.SetDriverChangeCallback(callback);
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, SetDriverChangeCallback)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: RemoveDeviceOfDeviceMap
 * @tc.desc: Test functions to RemoveDeviceOfDeviceMap benchmark test
 * @tc.desc: void RemoveDeviceOfDeviceMap(shared_ptr<Device> device);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, RemoveDeviceOfDeviceMap)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    uint32_t idx = 0;
    for (auto _ : state) {
        auto devInfo = std::make_shared<DeviceInfo>(TEST_BUS_DEVICE_ID_BASE + TEST_DEVICE_COUNT + 200 + idx,
            BusType::BUS_TYPE_TEST, "benchmark_remove_device");
        mgr.RegisterDevice(devInfo);
        auto devices = mgr.QueryDevicesById(devInfo->GetDeviceId());
        if (!devices.empty()) {
            mgr.RemoveDeviceOfDeviceMap(devices[0]);
        }
        idx++;
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, RemoveDeviceOfDeviceMap)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: MatchDriverInfos
 * @tc.desc: Test functions to MatchDriverInfos benchmark test
 * @tc.desc: void MatchDriverInfos(std::unordered_set<uint64_t> deviceIds);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, MatchDriverInfos)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    unordered_set<uint64_t> deviceIds;
    for (const auto &devInfo : testDevices_) {
        deviceIds.insert(devInfo->GetDeviceId());
    }
    for (auto _ : state) {
        mgr.MatchDriverInfos(deviceIds);
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, MatchDriverInfos)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();

/**
 * @tc.name: ClearMatchedDrivers
 * @tc.desc: Test functions to ClearMatchedDrivers benchmark test
 * @tc.desc: void ClearMatchedDrivers(const int32_t userId);
 * @tc.desc: Positive test: parameters correctly
 * @tc.type: FUNC
 */
BENCHMARK_F(EtxDeviceMgrBenchmarkTest, ClearMatchedDrivers)(benchmark::State &state)
{
    ExtDeviceManager &mgr = ExtDeviceManager::GetInstance();
    for (auto _ : state) {
        mgr.ClearMatchedDrivers(100);
    }
}
BENCHMARK_REGISTER_F(EtxDeviceMgrBenchmarkTest, ClearMatchedDrivers)->
    Iterations(ITERATION_FREQUENCY)->Repetitions(REPETITION_FREQUENCY)->ReportAggregatesOnly();
} // namespace ExternalDeviceManager
} // namespace OHOS
BENCHMARK_MAIN();
