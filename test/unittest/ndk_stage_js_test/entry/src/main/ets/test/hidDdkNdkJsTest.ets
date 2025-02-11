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

import { describe, expect, it } from '@ohos/hypium'
import hidDdkNdkTest from 'libhid_ddk_ndk_test.so'
import deviceManager from "@ohos.driver.deviceManager";

export default function hidDdkNdkJsTest() {
  let deviceId: number = -1;

  describe('hidDdkNdkJsTest', () => {
    const TAG: string = "[hidDdkNdkJsTest]";
    const HID_DDK_SUCCESS: number = 0;
    const HID_DDK_INVALID_PARAMETER: number = 401;
    const HID_DDK_TIMEOUT: number = 27300004;
    const HID_DDK_INIT_ERROR: number = 27300005;
    const HID_DDK_SERVICE_ERROR: number = 27300006;
    const HID_DDK_MEMORY_ERROR: number  = 27300007;
    const HID_DDK_IO_ERROR: number = 27300008;
    const HID_DDK_DEVICE_NOT_FOUND: number = 27300009;

    beforeAll(async () => {
      try {
        console.info(`before queryDevices`)
        let devices: Array<deviceManager.Device> = deviceManager.queryDevices(deviceManager.BusType.USB);
        console.info(`after queryDevices`)
        for (let item of devices) {
          console.info(`enter for`)
          let device: deviceManager.USBDevice = item as deviceManager.USBDevice;
          if ((device.vendorId == 0x2109 && device.productId == 0x888b) ||
            (device.vendorId == 0xbda && device.productId == 0x8153) ||
            (!hidDdkNdkTest.IsHidDevice(device.deviceId))) {
            console.warning(`Device vid = ${device.vendorId} pid = ${device.productId}, is not hid device`);
            continue;
          }
          deviceId = device.deviceId;
          console.info(`Device id is ${device.deviceId}`)
        }
      } catch (error) {
        console.error(`Failed to query device. Code is ${error.code}, message is ${error.message}`);
      }
    })

    /**
     * @tc.name       : testHidCreateDevice001
     * @tc.desc       : OH_Hid_CreateDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidCreateDevice001', 0, async (done: Function) => {
      console.info('----------------------testHidCreateDevice001---------------------------');
      try {
        const ret = hidDdkNdkTest.hidCreateDevice();
        console.info("Test result testHidCreateDevice001 : " + ret);
        expect(ret >= HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidCreateDevice001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidCreateDevice002
     * @tc.desc       : OH_Hid_CreateDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidCreateDevice002', 0, async (done: Function) => {
      console.info('----------------------testHidCreateDevice002---------------------------');
      try {
        const ret = hidDdkNdkTest.hidCreateDeviceTwo();
        console.info("Test result testHidCreateDevice002 : " + ret);
        expect(ret == HID_DDK_INVALID_PARAMETER).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidCreateDevice002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidCreateDevice003
     * @tc.desc       : OH_Hid_CreateDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidCreateDevice003', 0, async (done: Function) => {
      console.info('----------------------testHidCreateDevice003---------------------------');
      try {
        const ret = hidDdkNdkTest.hidCreateDeviceThree();
        console.info("Test result testHidCreateDevice003 : " + ret);
        expect(ret == HID_DDK_INVALID_PARAMETER).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidCreateDevice003 failed, message is ${err.message}`);
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
        const ret = hidDdkNdkTest.hidEmitEventOne();
        console.info("Test result testHidEmitEvent001 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidEmitEvent002
     * @tc.desc       : OH_Hid_EmitEvent Interface testing
     * @tc.type       : Function
     */
    it('testHidEmitEvent002', 0, async (done: Function) => {
      console.info('----------------------testHidEmitEvent002---------------------------');
      try {
        const ret = hidDdkNdkTest.hidEmitEventTwo();
        console.info("Test result testHidEmitEvent002 : " + ret);
        expect(ret !== HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidEmitEvent003
     * @tc.desc       : OH_Hid_EmitEvent Interface testing
     * @tc.type       : Function
     */
    it('testHidEmitEvent003', 0, async (done: Function) => {
      console.info('----------------------testHidEmitEvent003---------------------------');
      try {
        const ret = hidDdkNdkTest.hidEmitEventThree();
        console.info("Test result testHidEmitEvent003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidEmitEvent003 failed, message is ${err.message}`);
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
        const ret = hidDdkNdkTest.hidDestroyDeviceOne();
        console.info("Test result testHidDestroyDevice001 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidDestroyDevice002
     * @tc.desc       : OH_Hid_DestroyDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidDestroyDevice002', 0, async (done: Function) => {
      console.info('----------------------testHidDestroyDevice002---------------------------');
      try {
        const ret = hidDdkNdkTest.hidDestroyDeviceTwo();
        console.info("Test result testHidDestroyDevice002 : " + ret);
        expect(ret != HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidDestroyDevice003
     * @tc.desc       : OH_Hid_DestroyDevice Interface testing
     * @tc.type       : Function
     */
    it('testHidDestroyDevice003', 0, async (done: Function) => {
      console.info('----------------------testHidDestroyDevice003---------------------------');
      try {
        const ret = hidDdkNdkTest.hidDestroyDeviceThree();
        console.info("Test result testHidDestroyDevice003 : " + ret);
        expect(ret !== HID_DDK_SUCCESS).assertTrue();
        done();
      } catch (err) {
        console.error(TAG, `testHidDestroyDevice003 failed, message is ${err.message}`);
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
        const ret = hidDdkNdkTest.hidInitOne();
        console.info("Test Result testHidInit001 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
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
        const ret = hidDdkNdkTest.hidReleaseOne();
        console.info("Test Result testHidRelease001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidRelease001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidRelease002
     * @tc.desc       : OH_Hid_Release Interface testing
     * @tc.type       : Function
     */
    it('testHidRelease002', 0, async (done: Function) => {
      console.info('----------------------testHidRelease002---------------------------');
      try {
        const ret = hidDdkNdkTest.hidReleaseTwo();
        console.info("Test Result testHidRelease002 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidRelease002 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidOpenOne(deviceId);
        console.info("Test Result testHidOpen001 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidOpen001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidOpen002
     * @tc.desc       : OH_Hid_Open Interface testing
     * @tc.type       : Function
     */
    it('testHidOpen002', 0, async (done: Function) => {
      console.info('----------------------testHidOpen002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidOpenTwo(deviceId);
        console.info("Test Result testHidOpen002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidOpen002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidOpen003
     * @tc.desc       : OH_Hid_Open Interface testing
     * @tc.type       : Function
     */
    it('testHidOpen003', 0, async (done: Function) => {
      console.info('----------------------testHidOpen003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidOpenThree(deviceId);
        console.info("Test Result testHidOpen003 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidOpen003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidOpen004
     * @tc.desc       : OH_Hid_Open Interface testing
     * @tc.type       : Function
     */
    it('testHidOpen004', 0, async (done: Function) => {
      console.info('----------------------testHidOpen004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidOpenFour(deviceId);
        console.info("Test Result testHidOpen004 : " + ret);
        expect(ret).assertEqual(HID_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testHidOpen004 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidCloseOne(deviceId);
        console.info("Test Result testHidClose001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidClose001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidClose002
     * @tc.desc       : OH_Hid_Close Interface testing
     * @tc.type       : Function
     */
    it('testHidClose002', 0, async (done: Function) => {
      console.info('----------------------testHidClose002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidCloseTwo(deviceId);
        console.info("Test Result testHidClose002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidClose002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidClose003
     * @tc.desc       : OH_Hid_Close Interface testing
     * @tc.type       : Function
     */
    it('testHidClose003', 0, async (done: Function) => {
      console.info('----------------------testHidClose003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidCloseThree(deviceId);
        console.info("Test Result testHidClose003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidClose003 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidWriteOne(deviceId);
        console.info("Test Result testHidWrite001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidWrite001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidWrite002
     * @tc.desc       : OH_Hid_Write Interface testing
     * @tc.type       : Function
     */
    it('testHidWrite002', 0, async (done: Function) => {
      console.info('----------------------testHidWrite002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidWriteTwo(deviceId);
        console.info("Test Result testHidWrite002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidWrite002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidWrite003
     * @tc.desc       : OH_Hid_Write Interface testing
     * @tc.type       : Function
     */
    it('testHidWrite003', 0, async (done: Function) => {
      console.info('----------------------testHidWrite003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidWriteThree(deviceId);
        console.info("Test Result testHidWrite003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidWrite003 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadTimeoutOne(deviceId);
        console.info("Test Result testHidReadTimeout001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidReadTimeout001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidReadTimeout002
     * @tc.desc       : OH_Hid_ReadTimeout Interface testing
     * @tc.type       : Function
     */
    it('testHidReadTimeout002', 0, async (done: Function) => {
      console.info('----------------------testHidReadTimeout002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadTimeoutTwo(deviceId);
        console.info("Test Result testHidReadTimeout002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidReadTimeout002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidReadTimeout003
     * @tc.desc       : OH_Hid_ReadTimeout Interface testing
     * @tc.type       : Function
     */
    it('testHidReadTimeout003', 0, async (done: Function) => {
      console.info('----------------------testHidReadTimeout003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadTimeoutThree(deviceId);
        console.info("Test Result testHidReadTimeout003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidReadTimeout003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidReadTimeout004
     * @tc.desc       : OH_Hid_ReadTimeout Interface testing
     * @tc.type       : Function
     */
    it('testHidReadTimeout004', 0, async (done: Function) => {
      console.info('----------------------testHidReadTimeout004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadTimeoutFour(deviceId);
        console.info("Test Result testHidReadTimeout004 : " + ret);
        expect(ret).assertEqual(HID_DDK_TIMEOUT);
        done();
      } catch (err) {
        console.error(TAG, `testHidReadTimeout004 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadOne(deviceId);
        console.info("Test Result testHidRead001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidRead001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidRead002
     * @tc.desc       : OH_Hid_Read Interface testing
     * @tc.type       : Function
     */
    it('testHidRead002', 0, async (done: Function) => {
      console.info('----------------------testHidRead002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadTwo(deviceId);
        console.info("Test Result testHidRead002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidRead002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidRead003
     * @tc.desc       : OH_Hid_Read Interface testing
     * @tc.type       : Function
     */
    it('testHidRead003', 0, async (done: Function) => {
      console.info('----------------------testHidRead003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadThree(deviceId);
        console.info("Test Result testHidRead003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidRead003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidRead004
     * @tc.desc       : OH_Hid_Read Interface testing
     * @tc.type       : Function
     */
    it('testHidRead004', 0, async (done: Function) => {
      console.info('----------------------testHidRead004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidReadFour(deviceId);
        console.info("Test Result testHidRead004 : " + ret);
        expect(ret).assertEqual(HID_DDK_TIMEOUT);
        done();
      } catch (err) {
        console.error(TAG, `testHidRead004 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSetNonBlockingOne(deviceId);
        console.info("Test Result testHidSetNonBlocking001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidSetNonBlocking001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSetNonBlocking002
     * @tc.desc       : OH_Hid_SetNonBlocking Interface testing
     * @tc.type       : Function
     */
    it('testHidSetNonBlocking002', 0, async (done: Function) => {
      console.info('----------------------testHidSetNonBlocking002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSetNonBlockingTwo(deviceId);
        console.info("Test Result testHidSetNonBlocking002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidSetNonBlocking002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSetNonBlocking003
     * @tc.desc       : OH_Hid_SetNonBlocking Interface testing
     * @tc.type       : Function
     */
    it('testHidSetNonBlocking003', 0, async (done: Function) => {
      console.info('----------------------testHidSetNonBlocking003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSetNonBlockingThree(deviceId);
        console.info("Test Result testHidSetNonBlocking003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidSetNonBlocking003 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawInfoOne(deviceId);
        console.info("Test Result testHidGetRawInfo001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawInfo001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawInfo002
     * @tc.desc       : OH_Hid_GetRawInfo Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawInfo002', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawInfo002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawInfoTwo(deviceId);
        console.info("Test Result testHidGetRawInfo002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawInfo002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawInfo003
     * @tc.desc       : OH_Hid_GetRawInfo Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawInfo003', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawInfo003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawInfoThree(deviceId);
        console.info("Test Result testHidGetRawInfo003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawInfo003 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawNameOne(deviceId);
        console.info("Test Result testHidGetRawName001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawName001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawName002
     * @tc.desc       : OH_Hid_GetRawName Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawName002', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawName002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawNameTwo(deviceId);
        console.info("Test Result testHidGetRawName002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawName002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawName003
     * @tc.desc       : OH_Hid_GetRawName Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawName003', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawName003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawNameThree(deviceId);
        console.info("Test Result testHidGetRawName003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawName003 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetPhysicalAddressOne(deviceId);
        console.info("Test Result testHidGetPhysicalAddress001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetPhysicalAddress001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetPhysicalAddress002
     * @tc.desc       : OH_Hid_GetPhysicalAddress Interface testing
     * @tc.type       : Function
     */
    it('testHidGetPhysicalAddress002', 0, async (done: Function) => {
      console.info('----------------------testHidGetPhysicalAddress002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetPhysicalAddressTwo(deviceId);
        console.info("Test Result testHidGetPhysicalAddress002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetPhysicalAddress002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetPhysicalAddress003
     * @tc.desc       : OH_Hid_GetPhysicalAddress Interface testing
     * @tc.type       : Function
     */
    it('testHidGetPhysicalAddress003', 0, async (done: Function) => {
      console.info('----------------------testHidGetPhysicalAddress003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetPhysicalAddressThree(deviceId);
        console.info("Test Result testHidGetPhysicalAddress003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetPhysicalAddress003 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawUniqueIdOne(deviceId);
        console.info("Test Result testHidGetRawUniqueId001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawUniqueId001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawUniqueId002
     * @tc.desc       : OH_Hid_GetRawUniqueId Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawUniqueId002', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawUniqueId002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawUniqueIdTwo(deviceId);
        console.info("Test Result testHidGetRawUniqueId002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawUniqueId002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetRawUniqueId003
     * @tc.desc       : OH_Hid_GetRawUniqueId Interface testing
     * @tc.type       : Function
     */
    it('testHidGetRawUniqueId003', 0, async (done: Function) => {
      console.info('----------------------testHidGetRawUniqueId003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetRawUniqueIdThree(deviceId);
        console.info("Test Result testHidGetRawUniqueId003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetRawUniqueId003 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSendReportOne(deviceId);
        console.info("Test Result testHidSendReport001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidSendReport001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSendReport002
     * @tc.desc       : OH_Hid_SendReport Interface testing
     * @tc.type       : Function
     */
    it('testHidSendReport002', 0, async (done: Function) => {
      console.info('----------------------testHidSendReport002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSendReportTwo(deviceId);
        console.info("Test Result testHidSendReport002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidSendReport002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSendReport003
     * @tc.desc       : OH_Hid_SendReport Interface testing
     * @tc.type       : Function
     */
    it('testHidSendReport003', 0, async (done: Function) => {
      console.info('----------------------testHidSendReport003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSendReportThree(deviceId);
        console.info("Test Result testHidSendReport003 : " + ret);
        done();
      } catch (err) {
        console.error(TAG, `testHidSendReport003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSendReport004
     * @tc.desc       : OH_Hid_SendReport Interface testing
     * @tc.type       : Function
     */
    it('testHidSendReport004', 0, async (done: Function) => {
      console.info('----------------------testHidSendReport004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSendReportFour(deviceId);
        console.info("Test Result testHidSendReport004 : " + ret);
        done();
      } catch (err) {
        console.error(TAG, `testHidSendReport004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidSendReport005
     * @tc.desc       : OH_Hid_SendReport Interface testing
     * @tc.type       : Function
     */
    it('testHidSendReport005', 0, async (done: Function) => {
      console.info('----------------------testHidSendReport005---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidSendReportFive(deviceId);
        console.info("Test Result testHidSendReport005 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidSendReport005 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportOne(deviceId);
        console.info("Test Result testHidGetReport001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReport001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReport002
     * @tc.desc       : OH_Hid_GetReport Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReport002', 0, async (done: Function) => {
      console.info('----------------------testHidGetReport002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportTwo(deviceId);
        console.info("Test Result testHidGetReport002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReport002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReport003
     * @tc.desc       : OH_Hid_GetReport Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReport003', 0, async (done: Function) => {
      console.info('----------------------testHidGetReport003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportThree(deviceId);
        console.info("Test Result testHidGetReport003 : " + ret);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReport003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReport004
     * @tc.desc       : OH_Hid_GetReport Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReport004', 0, async (done: Function) => {
      console.info('----------------------testHidGetReport004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportFour(deviceId);
        console.info("Test Result testHidGetReport004 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReport004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReport005
     * @tc.desc       : OH_Hid_GetReport Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReport005', 0, async (done: Function) => {
      console.info('----------------------testHidGetReport005---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportFive(deviceId);
        console.info("Test Result testHidGetReport005 : " + ret);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReport005 failed, message is ${err.message}`);
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
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportDescriptorOne(deviceId);
        console.info("Test Result testHidGetReportDescriptor001 : " + ret);
        expect(ret).assertEqual(HID_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReportDescriptor001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReportDescriptor002
     * @tc.desc       : OH_Hid_GetReportDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReportDescriptor002', 0, async (done: Function) => {
      console.info('----------------------testHidGetReportDescriptor002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportDescriptorTwo(deviceId);
        console.info("Test Result testHidGetReportDescriptor002 : " + ret);
        expect(ret).assertEqual(HID_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReportDescriptor002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testHidGetReportDescriptor003
     * @tc.desc       : OH_Hid_GetReportDescriptor Interface testing
     * @tc.type       : Function
     */
    it('testHidGetReportDescriptor003', 0, async (done: Function) => {
      console.info('----------------------testHidGetReportDescriptor003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test HID device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test HID deviceId = " + deviceId);
        const ret = hidDdkNdkTest.hidGetReportDescriptorThree(deviceId);
        console.info("Test Result testHidGetReportDescriptor003 : " + ret);
        expect(ret).assertEqual(HID_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testHidGetReportDescriptor003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
  })
}
