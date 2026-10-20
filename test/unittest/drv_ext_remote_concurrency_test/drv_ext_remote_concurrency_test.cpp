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
class TestRemoteObject : public IRemoteObject {
public:
    TestRemoteObject() : IRemoteObject(u"IRemoteObject") { }
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
};

class TestDriverExtMgrCallback : public IDriverExtMgrCallback {
public:
    TestDriverExtMgrCallback() = default;
    sptr<IRemoteObject> AsObject() override
    {
        return sptr<TestRemoteObject>::MakeSptr();
    }
    ErrCode OnConnect(uint64_t deviceId, const sptr<IRemoteObject> &drvExtObj, const ErrMsg &errMsg) override
    {
        return EDM_OK;
    }
    ErrCode OnDisconnect(uint64_t deviceId, const ErrMsg &errMsg) override
    {
        return EDM_OK;
    }
    ErrCode OnUnBind(uint64_t deviceId, const ErrMsg &errMsg) override
    {
        return EDM_OK;
    }
};
} // namespace

class DrvExtRemoteConcurrencyTest : public testing::Test {
public:
    void SetUp() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DrvExtRemoteConcurrencyTest SetUp");
        deviceInfo_ = make_shared<DeviceInfo>(1, BusType::BUS_TYPE_USB, "test_device");
        device_ = make_shared<Device>(deviceInfo_);
    }
    void TearDown() override
    {
        EDM_LOGD(MODULE_DEV_MGR, "DrvExtRemoteConcurrencyTest TearDown");
        device_ = nullptr;
        deviceInfo_ = nullptr;
    }

protected:
    shared_ptr<Device> device_;
    shared_ptr<DeviceInfo> deviceInfo_;
};

// Verify the dedicated mutex protecting drvExtRemote_ is present.
HWTEST_F(DrvExtRemoteConcurrencyTest, DrvExtRemoteMutexExistsTest, TestSize.Level1)
{
    std::mutex *mutexAddr = &device_->drvExtRemoteMutex_;
    ASSERT_NE(mutexAddr, nullptr);
    EDM_LOGI(MODULE_DEV_MGR, "DrvExtRemoteMutexExistsTest: drvExtRemoteMutex_ present");
}

// Deterministic lock-protection check for GetDrvExtRemote.
//   Post-fix (function takes the lock): the worker blocks while we hold the mutex,
//     `done` stays false during the probe window -> test PASSES.
//   Pre-fix (no lock_guard, the bug): the worker returns immediately, `done`
//     becomes true during the probe window -> test FAILS.
HWTEST_F(DrvExtRemoteConcurrencyTest, GetDrvExtRemoteProtectedByMutexTest, TestSize.Level1)
{
    device_->drvExtRemoteMutex_.lock();
    std::atomic<bool> done {false};
    std::thread worker([&device_, &done]() {
        device_->GetDrvExtRemote();
        done.store(true, std::memory_order_release);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(done.load(std::memory_order_acquire))
        << "GetDrvExtRemote returned while the guard mutex was held -> NOT protected "
           "(pre-fix: no lock_guard, data race on drvExtRemote_)";
    device_->drvExtRemoteMutex_.unlock();
    worker.join();
    ASSERT_TRUE(done.load(std::memory_order_acquire));
    EDM_LOGI(MODULE_DEV_MGR, "GetDrvExtRemoteProtectedByMutexTest: blocked by mutex, protected");
}

HWTEST_F(DrvExtRemoteConcurrencyTest, UpdateDrvExtRemoteProtectedByMutexTest, TestSize.Level1)
{
    device_->drvExtRemoteMutex_.lock();
    std::atomic<bool> done {false};
    sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
    std::thread worker([&device_, &remote, &done]() {
        device_->UpdateDrvExtRemote(remote);
        done.store(true, std::memory_order_release);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(done.load(std::memory_order_acquire))
        << "UpdateDrvExtRemote returned while the guard mutex was held -> NOT protected "
           "(pre-fix: no lock_guard, data race on drvExtRemote_)";
    device_->drvExtRemoteMutex_.unlock();
    worker.join();
    ASSERT_TRUE(done.load(std::memory_order_acquire));
    EDM_LOGI(MODULE_DEV_MGR, "UpdateDrvExtRemoteProtectedByMutexTest: blocked by mutex, protected");
}

HWTEST_F(DrvExtRemoteConcurrencyTest, ClearDrvExtRemoteProtectedByMutexTest, TestSize.Level1)
{
    device_->drvExtRemoteMutex_.lock();
    std::atomic<bool> done {false};
    std::thread worker([&device_, &done]() {
        device_->ClearDrvExtRemote();
        done.store(true, std::memory_order_release);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_FALSE(done.load(std::memory_order_acquire))
        << "ClearDrvExtRemote returned while the guard mutex was held -> NOT protected "
           "(pre-fix: no lock_guard, data race on drvExtRemote_)";
    device_->drvExtRemoteMutex_.unlock();
    worker.join();
    ASSERT_TRUE(done.load(std::memory_order_acquire));
    EDM_LOGI(MODULE_DEV_MGR, "ClearDrvExtRemoteProtectedByMutexTest: blocked by mutex, protected");
}

// High-concurrency stress on Update/Clear/Get. Under TSAN this deterministically
// reports a data race on drvExtRemote_ for the pre-fix code (no lock_guard). It also
// asserts the snapshot returned by Get is always one of the known remotes or null.
HWTEST_F(DrvExtRemoteConcurrencyTest, ConcurrentUpdateClearGetStressTest, TestSize.Level1)
{
    const int iterPerThread = 3000;
    std::vector<sptr<IRemoteObject>> remotes;
    for (int i = 0; i < 6; i++) {
        remotes.push_back(sptr<TestRemoteObject>::MakeSptr());
    }

    std::atomic<bool> stop {false};
    std::atomic<int> badSnapshot {0};

    auto updater = [&device_, iterPerThread, &remotes]() {
        int idx = 0;
        for (int i = 0; i < iterPerThread; i++) {
            device_->UpdateDrvExtRemote(remotes[idx % remotes.size()]);
            idx++;
        }
    };
    auto clearer = [&device_, iterPerThread]() {
        for (int i = 0; i < iterPerThread; i++) {
            device_->ClearDrvExtRemote();
        }
    };
    auto getter = [&device_, &stop, &remotes, &badSnapshot]() {
        while (!stop.load(std::memory_order_relaxed)) {
            sptr<IRemoteObject> snap = device_->GetDrvExtRemote();
            if (snap != nullptr && std::find(remotes.begin(), remotes.end(), snap) == remotes.end()) {
                badSnapshot.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(updater);
    }
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(clearer);
    }
    for (int i = 0; i < 4; i++) {
        threads.emplace_back(getter);
    }

    for (int i = 0; i < 8; i++) {
        threads[i].join();
    }
    stop.store(true, std::memory_order_relaxed);
    for (int i = 8; i < 12; i++) {
        threads[i].join();
    }

    ASSERT_EQ(badSnapshot.load(), 0) << "data race detected: GetDrvExtRemote returned an unknown remote pointer";
    EDM_LOGI(MODULE_DEV_MGR, "ConcurrentUpdateClearGetStressTest: no bad snapshot, bad=%d", badSnapshot.load());
}

// Drive the API paths that acquire deviceMutex_ then drvExtRemoteMutex_
// (OnConnect / OnDisconnect) concurrently. The lock order is always
// deviceMutex_ -> drvExtRemoteMutex_; if an AB-BA ordering existed this would
// deadlock. A bounded wait turns a deadlock into a failure instead of a hang.
HWTEST_F(DrvExtRemoteConcurrencyTest, NoAbBaDeadlockMixedOpsTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    driverInfo->accessAllowed_ = true;
    device_->AddBundleInfo("testBundle", driverInfo);

    const int iterPerThread = 500;
    const int threadCount = 8;

    auto onConnectWorker = [&device_, iterPerThread]() {
        for (int i = 0; i < iterPerThread; i++) {
            sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
            device_->OnConnect(remote, 0);
        }
    };
    auto onDisconnectWorker = [&device_, iterPerThread]() {
        for (int i = 0; i < iterPerThread; i++) {
            device_->OnDisconnect(0);
        }
    };
    auto getterWorker = [&device_, iterPerThread]() {
        for (int i = 0; i < iterPerThread; i++) {
            auto snap = device_->GetDrvExtRemote();
            (void)snap;
        }
    };

    auto runner = [&onConnectWorker, &onDisconnectWorker, &getterWorker, threadCount]() {
        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount / 2; i++) {
            threads.emplace_back(onConnectWorker);
        }
        for (int i = 0; i < threadCount / 4; i++) {
            threads.emplace_back(onDisconnectWorker);
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

// Connect(callback, tokenId) acquires deviceMutex_ then drvExtRemoteMutex_ (via
// GetDrvExtRemote). OnConnect acquires deviceMutex_ then drvExtRemoteMutex_ (via
// UpdateDrvExtRemote + GetDrvExtRemote). Running them concurrently exercises the
// shared lock order and must not deadlock.
HWTEST_F(DrvExtRemoteConcurrencyTest, ConnectCallbackAndOnConnectConcurrentNoDeadlockTest, TestSize.Level1)
{
    shared_ptr<DriverInfo> driverInfo = make_shared<DriverInfo>("testBundle", "testDriver");
    driverInfo->accessAllowed_ = true;
    device_->AddBundleInfo("testBundle", driverInfo);

    const int iterPerThread = 200;

    auto connectWorker = [&device_, iterPerThread]() {
        for (int i = 0; i < iterPerThread; i++) {
            sptr<IDriverExtMgrCallback> callback = sptr<TestDriverExtMgrCallback>::MakeSptr();
            device_->Connect(callback, 100 + i);
        }
    };
    auto onConnectWorker = [&device_, iterPerThread]() {
        for (int i = 0; i < iterPerThread; i++) {
            sptr<IRemoteObject> remote = sptr<TestRemoteObject>::MakeSptr();
            device_->OnConnect(remote, 0);
        }
    };
    auto onDisconnectWorker = [&device_, iterPerThread]() {
        for (int i = 0; i < iterPerThread; i++) {
            device_->OnDisconnect(0);
        }
    };

    auto runner = [&connectWorker, &onConnectWorker, &onDisconnectWorker]() {
        std::vector<std::thread> threads;
        threads.emplace_back(connectWorker);
        threads.emplace_back(onConnectWorker);
        threads.emplace_back(onDisconnectWorker);
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
