/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import { beforeAll, describe, it, expect } from "@ohos/hypium"
import usbDdkNdkTest from 'libusb_ddk_ndk_test.so'
import deviceManager from "@ohos.driver.deviceManager";


export default function usbDdkNdkJdTest() {
  let dev: number = 0

  describe('usbDdkNdkJdTest', () => {

    beforeAll(async () => {
      try {
        let devices: Array<deviceManager.Device> = deviceManager.queryDevices(deviceManager.BusType.USB);
        for (let item of devices) {
          let device: deviceManager.USBDevice = item as deviceManager.USBDevice;
          dev = device.deviceId;
          console.info(`Device id is ${device.deviceId}`)
        }
      } catch (error) {
        console.error(`Failed to query device. Code is ${error.code}, message is ${error.message}`);
      }
    })

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0100
     * @tc.name       : testUsbInit001
     * @tc.desc       : OH_Usb_Init Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbInit001', 0, async (done: Function) => {
      let data = usbDdkNdkTest.usbInit();
      console.info("Test USB data Result = " + data);
      expect(data).assertEqual(0);
      done()
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0200
     * @tc.name       : testUsbRelease001
     * @tc.desc       : OH_Usb_Release Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbRelease001', 0, async (done: Function) => {
      let data = usbDdkNdkTest.usbRelease();
      console.info("Test USB data Result = " + data);
      expect(data).assertEqual(1);
      done()
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0300
     * @tc.name       : testUsbGetDeviceDescriptor001
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbGetDeviceDescriptor001', 0, async (done: Function) => {
      console.info("Test USB dev = " + dev);
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetDeviceDescriptorOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0400
     * @tc.name       : testUsbGetDeviceDescriptor002
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbGetDeviceDescriptor002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetDeviceDescriptorTwo(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0500
     * @tc.name       : testUsbGetConfigDescriptor001
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbGetConfigDescriptor001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetConfigDescriptorOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0600
     * @tc.name       : testUsbGetConfigDescriptor002
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbGetConfigDescriptor002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetConfigDescriptorTwo();
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-1);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0700
     * @tc.name       : testUsbGetConfigDescriptor003
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbGetConfigDescriptor003', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetConfigDescriptorThree(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0800
     * @tc.name       : testUsbFreeConfigDescriptor001
     * @tc.desc       : OH_Usb_FreeConfigDescriptor Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbFreeConfigDescriptor001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbFreeConfigDescriptor(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(1);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_0900
     * @tc.name       : testUsbClaimInterface001
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbClaimInterface001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbClaimInterfaceOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1000
     * @tc.name       : testUsbClaimInterface002
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbClaimInterface002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbClaimInterfaceTwo();
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-1);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1100
     * @tc.name       : testUsbClaimInterface003
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbClaimInterface003', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbClaimInterfaceThree(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1200
     * @tc.name       : testUsbReleaseInterface001
     * @tc.desc       : OH_Usb_ReleaseInterface Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbReleaseInterface001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbReleaseInterface(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1300
     * @tc.name       : testUsbSelectInterfaceSetting001
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSelectInterfaceSetting001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSelectInterfaceSettingOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1400
     * @tc.name       : testUsbSelectInterfaceSetting002
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbSelectInterfaceSetting002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSelectInterfaceSettingTwo(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-4);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1500
     * @tc.name       : testUsbGetCurrentInterfaceSetting001
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbGetCurrentInterfaceSetting001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetCurrentInterfaceSettingOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1600
     * @tc.name       : testUsbGetCurrentInterfaceSetting002
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbGetCurrentInterfaceSetting002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetCurrentInterfaceSettingTwo(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-4);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1700
     * @tc.name       : testUsbGetCurrentInterfaceSetting003
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbGetCurrentInterfaceSetting003', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbGetCurrentInterfaceSettingThree(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1800
     * @tc.name       : testUsbSendControlReadRequest001
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendControlReadRequest001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlReadRequestOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_1900
     * @tc.name       : testUsbSendControlReadRequest002
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbSendControlReadRequest002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlReadRequestTwo(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-4);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2000
     * @tc.name       : testUsbSendControlReadRequest003
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbSendControlReadRequest003', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlReadRequestThree(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2100
     * @tc.name       : testUsbSendControlReadRequest004
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbSendControlReadRequest004', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlReadRequestFour(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2200
     * @tc.name       : testUsbSendControlReadRequest005
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbSendControlReadRequest005', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlReadRequestFive(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2300
     * @tc.name       : testUsbSendControlWriteRequest001
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendControlWriteRequest001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlWriteRequestOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2400
     * @tc.name       : testUsbSendControlWriteRequest002
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbSendControlWriteRequest002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlWriteRequestTwo(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-4);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2500
     * @tc.name       : testUsbSendControlWriteRequest003
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 2
     */
    it('testUsbSendControlWriteRequest003', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlWriteRequestThree(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2600
     * @tc.name       : testUsbSendControlWriteRequest004
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendControlWriteRequest004', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendControlWriteRequestFour(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2700
     * @tc.name       : testUsbSendPipeRequest001
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequest001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2800
     * @tc.name       : testUsbSendPipeRequest002
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequest002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestTwo(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-4);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_2900
     * @tc.name       : testUsbSendPipeRequest003
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequest003', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestThree(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3000
     * @tc.name       : testUsbSendPipeRequest004
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequest004', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestFour(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3100
     * @tc.name       : testUsbCreateDeviceMemMap001
     * @tc.desc       : OH_Usb_CreateDeviceMemMap Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbCreateDeviceMemMap001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbCreateDeviceMemMapOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3200
     * @tc.name       : testUsbCreateDeviceMemMap002
     * @tc.desc       : OH_Usb_CreateDeviceMemMap Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbCreateDeviceMemMap002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbCreateDeviceMemMapTow(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3300
     * @tc.name       : testUsbDestroyDeviceMemMap001
     * @tc.desc       : OH_Usb_DestroyDeviceMemMap Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbDestroyDeviceMemMap001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbDestroyDeviceMemMap(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(1);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3400
     * @tc.name       : testUsbSendPipeRequestWithAshmem001
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequestWithAshmem001', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestWithAshmemOne(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(0);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3500
     * @tc.name       : testUsbSendPipeRequestWithAshmem002
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequestWithAshmem002', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestWithAshmemTwo(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3600
     * @tc.name       : testUsbSendPipeRequestWithAshmem003
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequestWithAshmem003', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestWithAshmemThree(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-2);
        done()
      }
    });

    /**
     * @tc.number     : SUB_USB_DriverExt_DDKETS_3700
     * @tc.name       : testUsbSendPipeRequestWithAshmem004
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.size       : MediumTest
     * @tc.type       : Function
     * @tc.level      : Level 0
     */
    it('testUsbSendPipeRequestWithAshmem004', 0, async (done: Function) => {
      if (dev == 0) {
        console.info("Test USB device not connected");
        expect(dev).assertEqual(0);
        done()
      } else {
        let data = usbDdkNdkTest.usbSendPipeRequestWithAshmemFour(dev);
        console.info("Test USB data Result = " + data);
        expect(data).assertEqual(-4);
        done()
      }
    });

  })

}

