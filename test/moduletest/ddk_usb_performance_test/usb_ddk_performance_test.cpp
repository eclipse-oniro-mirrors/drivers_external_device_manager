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

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include "usb_ddk_api.h"
#include "usb_ddk_types.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace ExternalDeviceManager {
namespace Testing {

class UsbDdkPerformanceTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGI(MODULE_USB_DDK, "UsbDdkPerformanceTest SetUp");
        int32_t ret = OH_Usb_Init();
        EDM_LOGI(MODULE_USB_DDK, "OH_Usb_Init result: %{public}d", ret);
    }
    
    void TearDown() override
    {
        EDM_LOGI(MODULE_USB_DDK, "UsbDdkPerformanceTest TearDown");
        OH_Usb_Release();
    }
};

HWTEST_F(UsbDdkPerformanceTest, ControlTransferPerformanceTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[64] = {0};
    uint32_t timeout = 5000;
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0100;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0012;
    
    const int testCount = 100;
    std::vector<long long> durations;
    
    for (int i = 0; i < testCount; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        durations.push_back(duration);
        
        if (i == 0) {
            EDM_LOGI(MODULE_USB_DDK, "First call result: %{public}d, duration: %{public}lld us", ret, duration);
        }
    }
    
    long long totalDuration = 0;
    for (auto duration : durations) {
        totalDuration += duration;
    }
    long long avgDuration = totalDuration / testCount;
    
    long long minDuration = durations[0];
    long long maxDuration = durations[0];
    for (auto duration : durations) {
        if (duration < minDuration) minDuration = duration;
        if (duration > maxDuration) maxDuration = duration;
    }
    
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer Performance Test:");
    EDM_LOGI(MODULE_USB_DDK, "  Test count: %{public}d", testCount);
    EDM_LOGI(MODULE_USB_DDK, "  Average duration: %{public}lld us", avgDuration);
    EDM_LOGI(MODULE_USB_DDK, "  Min duration: %{public}lld us", minDuration);
    EDM_LOGI(MODULE_USB_DDK, "  Max duration: %{public}lld us", maxDuration);
    
    EXPECT_GT(avgDuration, 0);
}

HWTEST_F(UsbDdkPerformanceTest, GetNonRootHubsPerformanceTest001, testing::ext::TestSize.Level1)
{
    Usb_NonRootHubArray hubArray;
    hubArray.nonRootHubIds = new uint64_t[128];
    
    const int testCount = 50;
    std::vector<long long> durations;
    
    for (int i = 0; i < testCount; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        int32_t ret = OH_Usb_GetNonRootHubs(&hubArray);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        durations.push_back(duration);
        
        if (i == 0) {
            EDM_LOGI(MODULE_USB_DDK, "First call result: %{public}d, num: %{public}u, duration: %{public}lld us",
                     ret, hubArray.num, duration);
        }
    }
    
    long long totalDuration = 0;
    for (auto duration : durations) {
        totalDuration += duration;
    }
    long long avgDuration = totalDuration / testCount;
    
    long long minDuration = durations[0];
    long long maxDuration = durations[0];
    for (auto duration : durations) {
        if (duration < minDuration) minDuration = duration;
        if (duration > maxDuration) maxDuration = duration;
    }
    
    EDM_LOGI(MODULE_USB_DDK, "GetNonRootHubs Performance Test:");
    EDM_LOGI(MODULE_USB_DDK, "  Test count: %{public}d", testCount);
    EDM_LOGI(MODULE_USB_DDK, "  Average duration: %{public}lld us", avgDuration);
    EDM_LOGI(MODULE_USB_DDK, "  Min duration: %{public}lld us", minDuration);
    EDM_LOGI(MODULE_USB_DDK, "  Max duration: %{public}lld us", maxDuration);
    
    delete[] hubArray.nonRootHubIds;
    EXPECT_GT(avgDuration, 0);
}

HWTEST_F(UsbDdkPerformanceTest, ControlTransferLargeDataPerformanceTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[256] = {0};
    uint32_t timeout = 5000;
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0200;
    setup.wIndex = 0x0000;
    setup.wLength = 0x00FF;
    
    const int testCount = 50;
    std::vector<long long> durations;
    
    for (int i = 0; i < testCount; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        durations.push_back(duration);
    }
    
    long long totalDuration = 0;
    for (auto duration : durations) {
        totalDuration += duration;
    }
    long long avgDuration = totalDuration / testCount;
    
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer Large Data Performance Test:");
    EDM_LOGI(MODULE_USB_DDK, "  Test count: %{public}d", testCount);
    EDM_LOGI(MODULE_USB_DDK, "  Average duration: %{public}lld us", avgDuration);
    
    EXPECT_GT(avgDuration, 0);
}

HWTEST_F(UsbDdkPerformanceTest, ControlTransferTimeoutPerformanceTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[64] = {0};
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0100;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0012;
    
    std::vector<uint32_t> timeouts = {100, 500, 1000, 2000, 5000};
    
    for (auto timeout : timeouts) {
        auto start = std::chrono::high_resolution_clock::now();
        int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        EDM_LOGI(MODULE_USB_DDK, "Timeout: %{public}u ms, Duration: %{public}lld us, Result: %{public}d",
                 timeout, duration, ret);
    }
}

HWTEST_F(UsbDdkPerformanceTest, ControlTransferDifferentSizesPerformanceTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    uint32_t timeout = 5000;
    
    std::vector<uint16_t> dataSizes = {8, 16, 32, 64, 128, 255};
    
    for (auto size : dataSizes) {
        UsbControlRequestSetup setup = {0};
        uint8_t* data = new uint8_t[size];
        
        setup.bmRequestType = 0x80;
        setup.bRequest = 0x06;
        setup.wValue = 0x0100;
        setup.wIndex = 0x0000;
        setup.wLength = size;
        
        auto start = std::chrono::high_resolution_clock::now();
        int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        EDM_LOGI(MODULE_USB_DDK, "Data size: %{public}u bytes, Duration: %{public}lld us, Result: %{public}d",
                 size, duration, ret);
        
        delete[] data;
    }
}

HWTEST_F(UsbDdkPerformanceTest, GetNonRootHubsRepeatedCallsTest001, testing::ext::TestSize.Level1)
{
    Usb_NonRootHubArray hubArray;
    hubArray.nonRootHubIds = new uint64_t[128];
    
    const int testCount = 100;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < testCount; i++) {
        int32_t ret = OH_Usb_GetNonRootHubs(&hubArray);
        if (ret != USB_DDK_SUCCESS) {
            EDM_LOGW(MODULE_USB_DDK, "GetNonRootHubs failed at iteration %{public}d: %{public}d", i, ret);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    long long avgDuration = (totalDuration * 1000) / testCount;
    
    EDM_LOGI(MODULE_USB_DDK, "GetNonRootHubs Repeated Calls Performance:");
    EDM_LOGI(MODULE_USB_DDK, "  Total test count: %{public}d", testCount);
    EDM_LOGI(MODULE_USB_DDK, "  Total duration: %{public}lld ms", totalDuration);
    EDM_LOGI(MODULE_USB_DDK, "  Average duration per call: %{public}lld us", avgDuration);
    
    delete[] hubArray.nonRootHubIds;
}

HWTEST_F(UsbDdkPerformanceTest, ControlTransferBenchmarkTest001, testing::ext::TestSize.Level1)
{
    uint64_t deviceId = 0;
    UsbControlRequestSetup setup = {0};
    uint8_t data[64] = {0};
    uint32_t timeout = 5000;
    
    setup.bmRequestType = 0x80;
    setup.bRequest = 0x06;
    setup.wValue = 0x0100;
    setup.wIndex = 0x0000;
    setup.wLength = 0x0012;
    
    const int warmupCount = 10;
    const int benchmarkCount = 1000;
    
    for (int i = 0; i < warmupCount; i++) {
        OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    int successCount = 0;
    int failCount = 0;
    
    for (int i = 0; i < benchmarkCount; i++) {
        int32_t ret = OH_Usb_ControlTransfer(deviceId, &setup, data, timeout);
        if (ret >= 0) {
            successCount++;
        } else {
            failCount++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    double throughput = (benchmarkCount * 1000.0) / totalDuration;
    long long avgLatency = (totalDuration * 1000) / benchmarkCount;
    
    EDM_LOGI(MODULE_USB_DDK, "ControlTransfer Benchmark Test:");
    EDM_LOGI(MODULE_USB_DDK, "  Benchmark count: %{public}d", benchmarkCount);
    EDM_LOGI(MODULE_USB_DDK, "  Success count: %{public}d", successCount);
    EDM_LOGI(MODULE_USB_DDK, "  Fail count: %{public}d", failCount);
    EDM_LOGI(MODULE_USB_DDK, "  Total duration: %{public}lld ms", totalDuration);
    EDM_LOGI(MODULE_USB_DDK, "  Throughput: %{public}.2f calls/sec", throughput);
    EDM_LOGI(MODULE_USB_DDK, "  Average latency: %{public}lld us", avgLatency);
    
    EXPECT_GT(throughput, 0);
}

} // namespace Testing
} // namespace ExternalDeviceManager
} // namespace OHOS
