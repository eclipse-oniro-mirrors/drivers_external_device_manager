/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import { beforeAll, describe, it, xit, expect } from "@ohos/hypium"
import ddkPermissionTest from 'libddk_permission_js_test.so'
import deviceManager from "@ohos.driver.deviceManager";


export default function DdkPermissionJsTest() {
  let deviceId: number = -1;

  describe('DdkPermissionJsTest', () => {

    const TAG: string = "[DdkPermissionJsTest]";
    const DDK_ERR_NOPERM: number = 201;

    beforeAll(async () => {
      try {
        let devices: Array<deviceManager.Device> = deviceManager.queryDevices(deviceManager.BusType.USB);
        for (let item of devices) {
          let device: deviceManager.USBDevice = item as deviceManager.USBDevice;
          deviceId = device.deviceId;
          console.info(`Device id is ${device.deviceId}`)
        }
      } catch (error) {
        console.error(`Failed to query device. Code is ${error.code}, message is ${error.message}`);
      }
    })

    /**
     * @tc.name       : testUsbInit001
     * @tc.desc       : OH_Usb_Init Interface testing
     * @tc.type       : Function
     */
    it('testUsbInit001', 0, async (done: Function) => {
      console.info('----------------------testUsbInit001---------------------------');
      try {
        const ret = ddkPermissionTest.usbInit();
        console.info("Test Result testUsbInit001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbInit001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbReleaseResource001
     * @tc.desc       : OH_Usb_ReleaseResource Interface testing
     * @tc.type       : Function
     */
    it('testUsbReleaseResource001', 0, async (done: Function) => {
      console.info('----------------------testUsbReleaseResource001---------------------------');
      try {
        const ret = ddkPermissionTest.usbReleaseResource();
        console.info("Test Result testUsbReleaseResource001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbReleaseResource001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetDeviceDescriptor001
     * @tc.desc       : OH_Usb_GetDeviceDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetDeviceDescriptor001', 0, async (done: Function) => {
      console.info('----------------------testUsbGetDeviceDescriptor001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbGetDeviceDescriptor(deviceId);
        console.info("Test Result testUsbGetDeviceDescriptor001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetDeviceDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetConfigDescriptor001
     * @tc.desc       : OH_Usb_GetConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetConfigDescriptor001', 0, async (done: Function) => {
      console.info('----------------------testUsbGetConfigDescriptor001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbGetConfigDescriptor(deviceId);
        console.info("Test Result testUsbGetConfigDescriptor001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetConfigDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbFreeConfigDescriptor001
     * @tc.desc       : OH_Usb_FreeConfigDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testUsbFreeConfigDescriptor001', 0, async (done: Function) => {
      console.info('----------------------testUsbFreeConfigDescriptor001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbFreeConfigDescriptor(deviceId);
        console.info("Test Result testUsbFreeConfigDescriptor001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbFreeConfigDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbClaimInterface001
     * @tc.desc       : OH_Usb_ClaimInterface Interface testing
     * @tc.type       : Function
     */
    it('testUsbClaimInterface001', 0, async (done: Function) => {
      console.info('----------------------testUsbClaimInterface001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbClaimInterface(deviceId);
        console.info("Test Result testUsbClaimInterface001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbClaimInterface001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbReleaseInterface001
     * @tc.desc       : OH_Usb_ReleaseInterface Interface testing
     * @tc.type       : Function
     */
    it('testUsbReleaseInterface001', 0, async (done: Function) => {
      console.info('----------------------testUsbReleaseInterface001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbReleaseInterface(deviceId);
        console.info("Test Result testUsbReleaseInterface001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbReleaseInterface001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSelectInterfaceSetting001
     * @tc.desc       : OH_Usb_SelectInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testUsbSelectInterfaceSetting001', 0, async (done: Function) => {
      console.info('----------------------testUsbSelectInterfaceSetting001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSelectInterfaceSetting(deviceId);
        console.info("Test Result testUsbSelectInterfaceSetting001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSelectInterfaceSetting001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetCurrentInterfaceSetting001
     * @tc.desc       : OH_Usb_GetCurrentInterfaceSetting Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetCurrentInterfaceSetting001', 0, async (done: Function) => {
      console.info('----------------------testUsbGetCurrentInterfaceSetting001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbGetCurrentInterfaceSetting(deviceId);
        console.info("Test Result testUsbGetCurrentInterfaceSetting001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetCurrentInterfaceSetting001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlReadRequest001
     * @tc.desc       : OH_Usb_SendControlReadRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlReadRequest001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlReadRequest001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendControlReadRequest(deviceId);
        console.info("Test Result testUsbSendControlReadRequest001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlReadRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendControlWriteRequest001
     * @tc.desc       : OH_Usb_SendControlWriteRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendControlWriteRequest001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendControlWriteRequest001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendControlWriteRequest(deviceId);
        console.info("Test Result testUsbSendControlWriteRequest001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendControlWriteRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });


    /**
     * @tc.name       : testUsbSendPipeRequest001
     * @tc.desc       : OH_Usb_SendPipeRequest Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequest001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequest001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendPipeRequest(deviceId);
        console.info("Test Result testUsbSendPipeRequest001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequest001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbCreateDeviceMemMap001
     * @tc.desc       : OH_Usb_CreateDeviceMemMap Interface testing
     * @tc.type       : Function
     */
    it('testUsbCreateDeviceMemMap001', 0, async (done: Function) => {
      console.info('----------------------testUsbCreateDeviceMemMap001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbCreateDeviceMemMape(deviceId);
        console.info("Test Result testUsbCreateDeviceMemMap001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbCreateDeviceMemMap001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSendPipeRequestWithAshmem001
     * @tc.desc       : OH_Usb_SendPipeRequestWithAshmem Interface testing
     * @tc.type       : Function
     */
    it('testUsbSendPipeRequestWithAshmem001', 0, async (done: Function) => {
      console.info('----------------------testUsbSendPipeRequestWithAshmem001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB deviceId = " + deviceId);
        const ret = ddkPermissionTest.usbSendPipeRequestWithAshmem(deviceId);
        console.info("Test Result testUsbSendPipeRequestWithAshmem001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSendPipeRequestWithAshmem001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbGetDevices001
     * @tc.desc       : OH_Usb_GetDevices Interface testing
     * @tc.type       : Function
     */
    it('testUsbGetDevices001', 0, async (done: Function) => {
      console.info('----------------------testUsbGetDevices001---------------------------');
      try {
        const ret = ddkPermissionTest.usbGetDevices(deviceId);
        console.info("Test Result testUsbGetDevices001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbGetDevices001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testhidCreateDevice001
     * @tc.desc       : OH_Hid_CreateDevice Interface testing
     * @tc.type       : Function
     */
    it('testhidCreateDevice001', 0, async (done: Function) => {
      console.info('----------------------testhidCreateDevice001---------------------------');
      try {
        const ret = ddkPermissionTest.hidCreateDevice();
        console.info("Test Result testhidCreateDevice001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testhidCreateDevice001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidEmitEvent001
     * @tc.desc       : OH_Hid_EmitEvent Interface testing
     * @tc.type       : Function
     */
    it('testHidEmitEvent001', 0, async (done: Function) => {
      console.info('----------------------testHidEmitEvent001---------------------------');
      try {
        const ret = ddkPermissionTest.hidEmitEvent();
        console.info("Test Result testHidEmitEvent001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

     /**
     * @tc.name       : testHidDestroyDevice001
     * @tc.desc       : OH_Hid_DestroyDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidDestroyDevice001', 0, async (done: Function) => {
      console.info('----------------------testHidDestroyDevice001---------------------------');
      try {
        const ret = ddkPermissionTest.hidDestroyDevice();
        console.info("Test Result testHidDestroyDevice001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidInit001
     * @tc.desc       : OH_Hid_Init Interface testing
     * @tc.type       : Function
     */
    it('testHidInit001', 0, async (done: Function) => {
      console.info('----------------------testHidInit001---------------------------');
      try {
        const ret = ddkPermissionTest.hidInit();
        console.info("Test Result testHidInit001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidInit001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidRelease001
     * @tc.desc       : OH_Hid_Release Interface testing
     * @tc.type       : Function
     */
    it('testHidRelease001', 0, async (done: Function) => {
      console.info('----------------------testHidRelease001---------------------------');
      try {
        const ret = ddkPermissionTest.hidRelease();
        console.info("Test Result testHidRelease001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidRelease001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidOpen001
     * @tc.desc       : OH_Hid_Open Interface testing
     * @tc.type       : Function
     */
    it('testHidOpen001', 0, async (done: Function) => {
      console.info('----------------------testHidOpen001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidOpen(deviceId);
        console.info("Test Result testHidOpen001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidOpen001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidClose001
     * @tc.desc       : OH_Hid_Close Interface testing
     * @tc.type       : Function
     */
    it('testHidClose001', 0, async (done: Function) => {
      console.info('----------------------testHidClose001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidClose(deviceId);
        console.info("Test Result testHidClose001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidClose001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidWrite001
     * @tc.desc       : OH_Hid_Write Interface testing
     * @tc.type       : Function
     */
    it('testHidWrite001', 0, async (done: Function) => {
      console.info('----------------------testHidWrite001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidWrite(deviceId);
        console.info("Test Result testHidWrite001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidWrite001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidReadTimeout001
     * @tc.desc       : OH_Hid_ReadTimeout Interface testing
     * @tc.type       : Function
     */
    it('testHidReadTimeout001', 0, async (done: Function) => {
      console.info('----------------------testHidReadTimeout001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidReadTimeout(deviceId);
        console.info("Test Result testHidReadTimeout001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidReadTimeout001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidRead001
     * @tc.desc       : OH_Hid_Read Interface testing
     * @tc.type       : Function
     */
    it('testHidRead001', 0, async (done: Function) => {
      console.info('----------------------testHidRead001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidRead(deviceId);
        console.info("Test Result testHidRead001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidRead001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSetNonBlocking001
     * @tc.desc       : OH_Hid_SetNonBlocking Interface testing
     * @tc.type       : Function
     */
    it('testHidSetNonBlocking001', 0, async (done: Function) => {
      console.info('----------------------testHidSetNonBlocking001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidSetNonBlocking(deviceId);
        console.info("Test Result testHidSetNonBlocking001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidSetNonBlocking001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawInfo001
     * @tc.desc       : OH_Hid_GetRawInfo Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawInfo001', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawInfo001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidGetRawInfo(deviceId);
        console.info("Test Result testHidGetRawInfo001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawInfo001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawName001
     * @tc.desc       : OH_Hid_GetRawName Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawName001', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawName001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidGetRawName(deviceId);
        console.info("Test Result testHidGetRawName001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawName001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetPhysicalAddress001
     * @tc.desc       : OH_Hid_GetPhysicalAddress Interface testing
     * @tc.type       : Function
     */
    it('testHidGetPhysicalAddress001', 0, async (done: Function) => {
      console.info('----------------------testHidGetPhysicalAddress001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidGetPhysicalAddress(deviceId);
        console.info("Test Result testHidGetPhysicalAddress001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetPhysicalAddress001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawUniqueId001
     * @tc.desc       : OH_Hid_GetRawUniqueId Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawUniqueId001', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawUniqueId001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidGetRawUniqueId(deviceId);
        console.info("Test Result testHidGetRawUniqueId001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawUniqueId001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSendReport001
     * @tc.desc       : OH_Hid_SendReport Interface testing
     * @tc.type       : Function
     */
    it('testHidSendReport001', 0, async (done: Function) => {
      console.info('----------------------testHidSendReport001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidSendReport(deviceId);
        console.info("Test Result testHidSendReport001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidSendReport001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReport001
     * @tc.desc       : OH_Hid_GetReport Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReport001', 0, async (done: Function) => {
      console.info('----------------------testHidGetReport001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidGetReport(deviceId);
        console.info("Test Result testHidGetReport001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReport001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReportDescriptor001
     * @tc.desc       : OH_Hid_GetReportDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReportDescriptor001', 0, async (done: Function) => {
      console.info('----------------------testHidGetReportDescriptor001---------------------------');
      try {
        console.info("Test HID deviceId = " + deviceId);
        const ret = ddkPermissionTest.hidGetReportDescriptor(deviceId);
        console.info("Test Result testHidGetReportDescriptor001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReportDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialInit001
     * @tc.desc       : OH_UsbSerial_Init Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialInit001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialInit001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialInit();
        console.info("Test Result testUsbSerialInit001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialInit001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
    
    /**
     * @tc.name       : testUsbSerialRelease001
     * @tc.desc       : OH_UsbSerial_Release Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialRelease001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRelease001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialRelease();
        console.info("Test Result testUsbSerialRelease001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRelease001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
    
    /**
     * @tc.name       : testUsbSerialOpen001
     * @tc.desc       : OH_UsbSerial_Open Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialOpen001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialOpen001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialOpen();
        console.info("Test Result testUsbSerialOpen001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialOpen001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialClose001
     * @tc.desc       : OH_UsbSerial_Close Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialClose001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialClose001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialClose();
        console.info("Test Result testUsbSerialClose001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialClose001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialRead001
     * @tc.desc       : OH_UsbSerial_Read Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialRead001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRead001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialRead();
        console.info("Test Result testUsbSerialRead001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRead001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

  /**
     * @tc.name       : testUsbSerialWrite001
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
   xit('testUsbSerialWrite001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialWrite001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialWrite();
        console.info("Test Result testUsbSerialWrite001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialWrite001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetBaudRate001
     * @tc.desc       : OH_UsbSerial_SetBaudRate Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialSetBaudRate001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetBaudRate001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialSetBaudRate();
        console.info("Test Result testUsbSerialSetBaudRate001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetBaudRate001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetParams001
     * @tc.desc       : OH_UsbSerial_SetParams Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialSetParams001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetParams001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialSetParams();
        console.info("Test Result testUsbSerialSetParams001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetParams001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetTimeout001
     * @tc.desc       : OH_UsbSerial_SetTimeout Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialSetTimeout001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetTimeout001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialSetTimeout();
        console.info("Test Result testUsbSerialSetTimeout001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetTimeout001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetFlowControl001
     * @tc.desc       : OH_UsbSerial_SetFlowControl Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialSetFlowControl001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetFlowControl001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialSetFlowControl();
        console.info("Test Result testUsbSerialSetFlowControl001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetFlowControl001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlush001
     * @tc.desc       : OH_UsbSerial_Flush Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialFlush001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlush001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialFlush();
        console.info("Test Result testUsbSerialFlush001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlush001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushInput001
     * @tc.desc       : OH_UsbSerial_FlushInput Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialFlushInput001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushInput001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialFlushInput();
        console.info("Test Result testUsbSerialFlushInput001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushInput001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushOutput001
     * @tc.desc       : OH_UsbSerial_FlushOutput Interface testing
     * @tc.type       : Function
     */
    xit('testUsbSerialFlushOutput001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushOutput001---------------------------');
      try {
        const ret = ddkPermissionTest.usbSerialFlushOutput();
        console.info("Test Result testUsbSerialFlushOutput001 : " + ret);
        expect(ret).assertEqual(DDK_ERR_NOPERM);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushOutput001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
  })
}