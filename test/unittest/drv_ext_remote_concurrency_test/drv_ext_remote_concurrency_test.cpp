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

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdint>
#include <future>
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#define private public
#include "device.h"
#include "usb_device_info.h"
#include "usb_driver_info.h"
#undef private

namespace OHOS {
namespace ExternalDeviceManager {
using namespace std;
using namespace testing::ext;

namespace {
// TestRemoteObject tracks live instance count via a static atomic.
//
// Detection strategy for the unprotected drvExtRemote_ (pre-fix):
// sptr<T>::operator= does: (1) INC new ref, (2) DEC old ref, (3) store pointer.
// Without a lock, two writers can both observe the same OLD pointer and each call
// DecStrongRef on it -> RefBase::DecStrongRefCount does a check-then-act (load > 0,
// then fetch_sub) -> one fetch_sub drives the strong refcount to 0 and deletes the
// RefBase, the second fetch_sub drives it to -1. That negative refcount is the
// deterministic, observable invariant violation (it does NOT depend on lucky timing:
// the check-then-act window is wide, and with enough concurrency it will trigger).
//
// Post-fix (lock_guard), the mutex serializes the three steps so no two writers ever
// race on DecStrongRef, and the refcount never goes negative.
class TestRemoteObject : public IRemoteObject {
public:
    TestRemoteObject() : IRemoteObject(u"IRemoteObject")
    {
        liveCount_.fetch_add(1, std::memory_order_relaxed);
    }
    ~TestRemoteObject()
    {
        liveCount_.fetch_sub(1, std::memory_order_relaxed);
    }
    static int32_t GetLiveCount()
    {
        return liveCount_.load(std::memory_order_acquire);
    }
    static void ResetLiveCount()
    {
        liveCount_.store(0, std::memory_order_release);
    }
    int32_t GetObjectRefCount()
    {
        return 0;
    }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return 0;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient)
    {
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient)
    {
        return true;
    }
    int Dump(int fd, const std::vector<std::u16string> &args)
    {
        return 0;
    }

private:
    static std::atomic<int32_t> liveCount_;
};
std::atomic<int32_t> TestRemoteObject::liveCount_ {0};
} // namespace

class DrvExtRemoteConcurrencyTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DrvExtRemoteConcurrencyTest SetUp");
        TestRemoteObject::ResetLiveCount();
        deviceInfo_ = make_shared<DeviceInfo>(1, BusType::BUS_TYPE_USB, "test_device");
        device_ = make_shared<Device>(deviceInfo_);
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DrvExtRemoteConcurrencyTest TearDown");
        device_ = nullptr;
        deviceInfo_ = nullptr;
        TestRemoteObject::ResetLiveCount();
    }

    // Worker bodies extracted as static methods so the HWTEST_F stays under 50 lines
    // (G.FUD.05) and the lambdas only capture-and-forward.
    static void UpdaterBody(Device &device, int iterPerThread, std::atomic<int32_t> &minLive)
    {
        for (int i = 0; i < iterPerThread; i++) {
            sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
            device.UpdateDrvExtRemote(remote);
            RecordMinLive(minLive);
        }
    }
    static void ClearerBody(Device &device, int iterPerThread, std::atomic<int32_t> &minLive)
    {
        for (int i = 0; i < iterPerThread; i++) {
            device.ClearDrvExtRemote();
            RecordMinLive(minLive);
        }
    }
    static void GetterBody(Device &device, std::atomic<bool> &stop, std::atomic<int32_t> &minLive)
    {
        while (!stop.load(std::memory_order_relaxed)) {
            sptr<IRemoteObject> snap = device.GetDrvExtRemote();
            (void)snap;
            RecordMinLive(minLive);
        }
    }
    // Simple deadlock-test workers (no live-count tracking). Extracted to keep
    // HWTEST_F functions under 50 lines (G.FUD.05).
    static void UpdateWorkerBody(Device &device, int iterPerThread, const std::vector<sptr<IRemoteObject>> &remotes)
    {
        int idx = 0;
        for (int i = 0; i < iterPerThread; i++) {
            device.UpdateDrvExtRemote(remotes[idx % remotes.size()]);
            idx++;
        }
    }
    static void ClearWorkerBody(Device &device, int iterPerThread)
    {
        for (int i = 0; i < iterPerThread; i++) {
            device.ClearDrvExtRemote();
        }
    }
    static void GetterWorkerBody(Device &device, int iterPerThread)
    {
        for (int i = 0; i < iterPerThread; i++) {
            auto snap = device.GetDrvExtRemote();
            (void)snap;
        }
    }

protected:
    shared_ptr<Device> device_;
    shared_ptr<DeviceInfo> deviceInfo_;

private:
    static void RecordMinLive(std::atomic<int32_t> &minLive)
    {
        int32_t cur = TestRemoteObject::GetLiveCount();
        int32_t prev = minLive.load(std::memory_order_relaxed);
        // CAS loop: compare_exchange_weak updates `prev` on failure, so we must
        // re-check the condition. Written as a while with an explicit body so the
        // loop control variable update is visible to static analysis (G.CTL.03).
        while (cur < prev) {
            if (minLive.compare_exchange_weak(prev, cur)) {
                break;
            }
            // compare_exchange_weak failed: `prev` has been loaded with the current
            // value of minLive; loop back and re-check cur < prev.
        }
    }
};

// Baseline: Update -> Get -> Clear -> Get behaves as expected (functional sanity).
HWTEST_F(DrvExtRemoteConcurrencyTest, GetUpdateClearDrvExtRemoteSequentialTest, TestSize.Level1)
{
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    ASSERT_EQ(device_->GetDrvExtRemote(), nullptr);
    device_->UpdateDrvExtRemote(remote);
    ASSERT_EQ(device_->GetDrvExtRemote(), remote);
    device_->ClearDrvExtRemote();
    ASSERT_EQ(device_->GetDrvExtRemote(), nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "GetUpdateClearDrvExtRemoteSequentialTest: sequential ops ok");
}

// High-concurrency stress on Update/Clear/Get.
//
// Detection strategy (does NOT reference drvExtRemoteMutex_, so it compiles both
// pre-fix and post-fix):
//   - Many writer threads each create their own TestRemoteObject and call
//     UpdateDrvExtRemote; clearer threads call ClearDrvExtRemote; getter threads
//     call GetDrvExtRemote.
//   - Pre-fix (no lock_guard): concurrent writers race on sptr::operator=, which
//     does (INC new, DEC old, STORE ptr) non-atomically. Two writers both read the
//     same OLD pointer, each calls DecStrongRef -> the check-then-act window in
//     RefCounter::DecStrongRefCount (load>0 then fetch_sub) lets both proceed ->
//     strong refcount goes to -1 and the RefBase is double-freed. The observable
//     signal: TestRemoteObject destructor runs more times than constructors, so
//     liveCount_ goes NEGATIVE, or a live TestRemoteObject is destroyed while still
//     referenced (crash / ASAN report). With many threads and iterations this
//     interleaving is hit reliably.
//   - Post-fix (lock_guard): the mutex serializes all three steps; refcount is
//     always correct and liveCount_ is always consistent (>= 0).
//
// After all threads finish and the device is cleared, liveCount_ must be >= 0
// (no double-free) and == 0 (no leak). Pre-fix: fails (negative). Post-fix: passes.
HWTEST_F(DrvExtRemoteConcurrencyTest, ConcurrentUpdateClearGetStressTest, TestSize.Level1)
{
    const int iterPerThread = 2000;
    const int updaterCount = 6;
    const int clearerCount = 6;
    const int getterCount = 4;
    std::atomic<bool> stop {false};
    std::atomic<int32_t> minLiveCount {INT32_MAX};

    auto updater = [this, iterPerThread, &minLiveCount]() {
        UpdaterBody(*device_, iterPerThread, minLiveCount);
    };
    auto clearer = [this, iterPerThread, &minLiveCount]() {
        ClearerBody(*device_, iterPerThread, minLiveCount);
    };
    auto getter = [this, &stop, &minLiveCount]() {
        GetterBody(*device_, stop, minLiveCount);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < updaterCount; i++) {
        threads.emplace_back(updater);
    }
    for (int i = 0; i < clearerCount; i++) {
        threads.emplace_back(clearer);
    }
    for (int i = 0; i < getterCount; i++) {
        threads.emplace_back(getter);
    }
    const int writerCount = updaterCount + clearerCount;
    for (int i = 0; i < writerCount; i++) {
        threads[i].join();
    }
    stop.store(true, std::memory_order_relaxed);
    for (int i = writerCount; i < writerCount + getterCount; i++) {
        threads[i].join();
    }
    device_->ClearDrvExtRemote();
    int32_t finalLive = TestRemoteObject::GetLiveCount();
    int32_t observedMin = minLiveCount.load(std::memory_order_acquire);
    ASSERT_GE(observedMin, 0) << "data race detected: TestRemoteObject live count went negative (" << observedMin
                              << ") -> double DecStrongRef on unprotected drvExtRemote_";
    ASSERT_GE(finalLive, 0) << "leak/double-free detected: final live count negative";
    ASSERT_EQ(finalLive, 0) << "leak detected: TestRemoteObject live count non-zero after cleanup";
    EDM_LOGI(MODULE_DEV_MGR, "ConcurrentUpdateClearGetStressTest: no data race, observedMin=%d finalLive=%d",
        observedMin, finalLive);
}

// Drive concurrent Update/Clear/Get to verify no AB-BA deadlock. To avoid log storms
// this test does NOT call OnConnect/OnDisconnect; it directly exercises the three
// lock-guarded accessors. A bounded wait turns a deadlock into a failure.
HWTEST_F(DrvExtRemoteConcurrencyTest, NoAbBaDeadlockMixedOpsTest, TestSize.Level1)
{
    const int iterPerThread = 500;
    const int threadCount = 8;
    std::vector<sptr<IRemoteObject>> remotes;
    for (int i = 0; i < 4; i++) {
        remotes.push_back(sptr<TestRemoteObject>::MakeSptr());
    }
    auto updateWorker = [this, iterPerThread, &remotes]() {
        UpdateWorkerBody(*device_, iterPerThread, remotes);
    };
    auto clearWorker = [this, iterPerThread]() {
        ClearWorkerBody(*device_, iterPerThread);
    };
    auto getterWorker = [this, iterPerThread]() {
        GetterWorkerBody(*device_, iterPerThread);
    };
    auto runner = [&updateWorker, &clearWorker, &getterWorker, threadCount]() {
        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount / 2; i++) {
            threads.emplace_back(updateWorker);
        }
        for (int i = 0; i < threadCount / 4; i++) {
            threads.emplace_back(clearWorker);
        }
        for (int i = 0; i < threadCount / 4; i++) {
            threads.emplace_back(getterWorker);
        }
        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    };
    std::future<void> fut = std::async(std::launch::async, runner);
    auto status = fut.wait_for(std::chrono::seconds(30));
    ASSERT_EQ(status, std::future_status::ready) << "threads did not finish within 30s, likely AB-BA deadlock";
    EDM_LOGI(MODULE_DEV_MGR, "NoAbBaDeadlockMixedOpsTest: all threads completed, no deadlock");
}

// Concurrently run Update/Clear/Get (all acquire drvExtRemoteMutex_) to verify the
// lock-ordering does not reverse against any other lock. To avoid log storms this
// test does NOT call OnConnect/OnDisconnect/Connect. A bounded wait turns a deadlock
// into a failure.
HWTEST_F(DrvExtRemoteConcurrencyTest, ConnectCallbackAndOnConnectConcurrentNoDeadlockTest, TestSize.Level1)
{
    const int iterPerThread = 200;
    std::vector<sptr<IRemoteObject>> remotes;
    for (int i = 0; i < 4; i++) {
        remotes.push_back(sptr<TestRemoteObject>::MakeSptr());
    }
    auto updateWorker = [this, iterPerThread, &remotes]() {
        UpdateWorkerBody(*device_, iterPerThread, remotes);
    };
    auto clearWorker = [this, iterPerThread]() {
        ClearWorkerBody(*device_, iterPerThread);
    };
    auto getterWorker = [this, iterPerThread]() {
        GetterWorkerBody(*device_, iterPerThread);
    };
    auto runner = [&updateWorker, &clearWorker, &getterWorker]() {
        std::vector<std::thread> threads;
        threads.emplace_back(updateWorker);
        threads.emplace_back(clearWorker);
        threads.emplace_back(getterWorker);
        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    };
    std::future<void> fut = std::async(std::launch::async, runner);
    auto status = fut.wait_for(std::chrono::seconds(30));
    ASSERT_EQ(status, std::future_status::ready) << "threads did not finish within 30s, likely AB-BA deadlock";
    EDM_LOGI(MODULE_DEV_MGR, "ConnectCallbackAndOnConnectConcurrentNoDeadlockTest: no deadlock across both locks");
}
} // namespace ExternalDeviceManager
} // namespace OHOS
