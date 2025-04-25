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

#ifndef USB_IMPL_MOCK_H
#define USB_IMPL_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "v2_0/iusb_host_interface.h"
#include "v2_0/usb_types.h"

using OHOS::HDI::Usb::V2_0::IUsbdBulkCallback;
using OHOS::HDI::Usb::V2_0::IUsbdSubscriber;
using OHOS::HDI::Usb::V2_0::IUsbHostInterface;
using OHOS::HDI::Usb::V2_0::PortInfo;
using OHOS::HDI::Usb::V2_0::UsbCtrlTransfer;
using OHOS::HDI::Usb::V2_0::UsbDev;
using OHOS::HDI::Usb::V2_0::USBDeviceInfo;
using OHOS::HDI::Usb::V2_0::UsbPipe;
using OHOS::HDI::Usb::V2_0::UsbCtrlTransferParams;
using OHOS::HDI::Usb::V2_0::USBTransferInfo;
using OHOS::HDI::Usb::V2_0::IUsbdTransferCallback;
namespace OHOS {
namespace USB {
constexpr int32_t DEV_ADDR_OK = 2;
constexpr int32_t DEV_ADDR_OK_2 = 3;
constexpr int32_t DEV_ADDR_OK_ERR_DESC = 4;
constexpr int32_t DEV_ADDR_OK_NULL_DESC = 5;

constexpr int32_t BUS_NUM_OK = 6;
constexpr int32_t BUS_NUM_OK_2 = 7;
constexpr int32_t DEV_ADDR_INVALID = 255;
constexpr int32_t BUS_NUM_INVALID = 255;
constexpr int32_t DEFAULT_PORT_ID = 1;
constexpr int32_t RETVAL_INVALID = -1;
constexpr int32_t BUS_NUM_ERR = 8;
constexpr int32_t DEV_ADDR_ERR = 8;
constexpr int32_t DEV_ADDR_INTERFACE_ERR = 9;

class UsbHostImplMock : public IUsbHostInterface {
public:
    UsbHostImplMock();
    ~UsbHostImplMock();
    int32_t SubscriberDeviceEvent(const USBDeviceInfo &info);
    int32_t BindUsbdHostSubscriber(const sptr<IUsbdSubscriber> &subscriber) override;
    int32_t UnbindUsbdHostSubscriber(const sptr<IUsbdSubscriber> &subscriber) override;
    int32_t GetDeviceDescriptor(const UsbDev& dev, std::vector<uint8_t>& descriptor) override;
    int32_t GetRawDescriptor(const UsbDev &dev, std::vector<uint8_t> &decriptor) override;
    int32_t GetStringDescriptor(const UsbDev &dev, uint8_t descId, std::vector<uint8_t> &decriptor) override;
    int32_t GetConfig(const UsbDev &dev, uint8_t &configIndex) override;
    int32_t OpenDevice(const UsbDev &dev) override;
    MOCK_METHOD1(CloseDevice, int32_t(const UsbDev &dev));
    MOCK_METHOD3(GetConfigDescriptor, int32_t(const UsbDev &dev, uint8_t descId, std::vector<uint8_t> &decriptor));
    MOCK_METHOD2(GetFileDescriptor, int32_t(const UsbDev &dev, int32_t &fd));
    MOCK_METHOD2(SetConfig, int32_t(const UsbDev &dev, uint8_t configIndex));
    MOCK_METHOD3(ClaimInterface, int32_t(const UsbDev &dev, uint8_t interfaceid, uint8_t force));
    MOCK_METHOD2(ReleaseInterface, int32_t(const UsbDev &dev, uint8_t interfaceid));
    MOCK_METHOD3(SetInterface, int32_t(const UsbDev &dev, uint8_t interfaceid, uint8_t altIndex));
    MOCK_METHOD4(
        BulkTransferRead, int32_t(const UsbDev &dev, const UsbPipe &pipe, int32_t timeout, std::vector<uint8_t> &data));
    MOCK_METHOD4(BulkTransferWrite,
        int32_t(const UsbDev &dev, const UsbPipe &pipe, int32_t timeout, const std::vector<uint8_t> &data));
    MOCK_METHOD3(
        ControlTransferRead, int32_t(const UsbDev &dev, const UsbCtrlTransfer &ctrl, std::vector<uint8_t> &data));
    MOCK_METHOD3(ControlTransferWrite,
        int32_t(const UsbDev &dev, const UsbCtrlTransfer &ctrl, const std::vector<uint8_t> &data));
    MOCK_METHOD4(InterruptTransferRead,
        int32_t(const UsbDev &dev, const UsbPipe &pipe, int32_t timeout, std::vector<uint8_t> &data));
    MOCK_METHOD4(InterruptTransferWrite,
        int32_t(const UsbDev &dev, const UsbPipe &pipe, int32_t timeout, const std::vector<uint8_t> &data));
    MOCK_METHOD4(
        IsoTransferRead, int32_t(const UsbDev &dev, const UsbPipe &pipe, int32_t timeout, std::vector<uint8_t> &data));
    MOCK_METHOD4(IsoTransferWrite,
        int32_t(const UsbDev &dev, const UsbPipe &pipe, int32_t timeout, const std::vector<uint8_t> &data));
    MOCK_METHOD4(RequestQueue,
        int32_t(const UsbDev &dev, const UsbPipe &pipe, const std::vector<uint8_t> &clientData,
            const std::vector<uint8_t> &buffer));
    MOCK_METHOD4(RequestWait,
        int32_t(const UsbDev &dev, std::vector<uint8_t> &clientData, std::vector<uint8_t> &buffer, int32_t timeout));
    MOCK_METHOD2(RequestCancel, int32_t(const UsbDev &dev, const UsbPipe &pipe));
    MOCK_METHOD1(GetCurrentFunctions, int32_t(int32_t &funcs));
    MOCK_METHOD1(SetCurrentFunctions, int32_t(int32_t funcs));
    MOCK_METHOD3(RegBulkCallback, int32_t(const UsbDev &dev, const UsbPipe &pipe, const sptr<IUsbdBulkCallback> &cb));
    MOCK_METHOD2(UnRegBulkCallback, int32_t(const UsbDev &dev, const UsbPipe &pipe));
    MOCK_METHOD3(BulkRead, int32_t(const UsbDev &dev, const UsbPipe &pipe, const sptr<Ashmem> &ashmem));
    MOCK_METHOD3(BulkWrite, int32_t(const UsbDev &dev, const UsbPipe &pipe, const sptr<Ashmem> &ashmem));
    MOCK_METHOD2(BulkCancel, int32_t(const UsbDev &dev, const UsbPipe &pipe));
    MOCK_METHOD3(ManageInterface, int32_t(const UsbDev &dev, uint8_t interfaceid, bool disable));
    MOCK_METHOD3(GetInterfaceActiveStatus, int32_t(const UsbDev &dev, uint8_t interfaceid, bool &unactivated));
    MOCK_METHOD2(GetDeviceSpeed, int32_t(const UsbDev &dev, uint8_t &speed));
    MOCK_METHOD1(ResetDevice, int32_t(const UsbDev &dev));
    MOCK_METHOD2(ClearHalt, int32_t(const UsbDev &dev, const UsbPipe &pipe));
    MOCK_METHOD2(GetDeviceFileDescriptor, int32_t(const UsbDev &dev, int32_t &fd));
    MOCK_METHOD5(BulkTransferReadwithLength, int32_t(const UsbDev &dev,
        const UsbPipe &pipe, int32_t timeout, int32_t length, std::vector<uint8_t> &data));
    MOCK_METHOD3(ControlTransferReadwithLength,
        int32_t(const UsbDev &dev, const UsbCtrlTransferParams &ctrlParams, std::vector<uint8_t> &data));
    MOCK_METHOD4(UsbSubmitTransfer, int32_t(const UsbDev &dev, const USBTransferInfo &info,
        const sptr<IUsbdTransferCallback> &cb, const sptr<Ashmem> &ashmem));
    MOCK_METHOD2(UsbCancelTransfer, int32_t(const UsbDev &dev, const int32_t endpoint));
private:
    sptr<IUsbdSubscriber> subscriber_ = nullptr;
};
} // namespace USB
} // namespace OHOS
#endif
