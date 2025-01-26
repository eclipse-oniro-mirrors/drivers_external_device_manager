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

import { beforeAll, describe, it, expect } from "@ohos/hypium"
import serialDdkNdkTest from 'libserial_ddk_ndk_test.so'
import deviceManager from "@ohos.driver.deviceManager";


export default function serialDdkNdkJsTest() {
  let deviceId: number = -1;

  describe('serialDdkNdkJsTest', () => {

    const TAG: string = "[serialDdkNdkJsTest]";
    const USB_SERIAL_DDK_INVALID_PARAMETER: number = 401;
    const USB_SERIAL_DDK_SUCCESS: number = 31600000;
    const USB_SERIAL_DDK_INVALID_OPERATION: number = 31600001;
    const USB_SERIAL_DDK_INIT_ERROR: number = 31600002;
    const USB_SERIAL_DDK_SERVICE_ERROR: number = 31600003;
    const USB_SERIAL_DDK_MEMORY_ERROR: number = 31600004;
    const USB_SERIAL_DDK_IO_ERROR: number = 31600005;
    const USB_SERIAL_DDK_DEVICE_NOT_FOUND: number = 31600006;

    beforeAll(async () => {
      try {
        let devices: Array<deviceManager.Device> = deviceManager.queryDevices(deviceManager.BusType.USB);
        for (let item of devices) {
          let device: deviceManager.USBDevice = item as deviceManager.USBDevice;
          console.info(`Device id is ${device.deviceId}`)
          if (!serialDdkNdkTest.isUsbSerialDevice(device.deviceId)) {
            console.info(`This id ${device.deviceId} is not a usb serial device`)
            continue;
          }
          deviceId = device.deviceId;
          console.info(`Usb serial device id is ${device.deviceId}`)
        }
      } catch (error) {
        console.error(`Failed to query device. Code is ${error.code}, message is ${error.message}`);
      }
    })

    /**
     * @tc.name       : testUsbSerialInit001
     * @tc.desc       : OH_UsbSerial_Init Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialInit001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialInit001---------------------------');
      try {
        const ret = serialDdkNdkTest.usbSerialInit();
        console.info("Test Result testUsbSerialInit001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
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
    it('testUsbSerialRelease001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRelease001---------------------------');
      try {
        const ret = serialDdkNdkTest.usbSerialReleaseOne();
        console.info("Test Result testUsbSerialRelease001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRelease001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialRelease002
     * @tc.desc       : OH_UsbSerial_Release Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialRelease002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRelease002---------------------------');
      try {
        const ret = serialDdkNdkTest.usbSerialReleaseTwo();
        console.info("Test Result testUsbSerialRelease002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRelease002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialOpen001
     * @tc.desc       : OH_UsbSerial_Open Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialOpen001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialOpen001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialOpenOne(deviceId);
        console.info("Test Result testUsbSerialOpen001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialOpen001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialOpen002
     * @tc.desc       : OH_UsbSerial_Open Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialOpen002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialOpen002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialOpenTwo(deviceId);
        console.info("Test Result testUsbSerialOpen002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialOpen002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialOpen003
     * @tc.desc       : OH_UsbSerial_Open Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialOpen003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialOpen003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialOpenThree(deviceId);
        console.info("Test Result testUsbSerialOpen003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialOpen003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialOpen004
     * @tc.desc       : OH_UsbSerial_Open Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialOpen004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialOpen004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialOpenFour(deviceId);
        console.info("Test Result testUsbSerialOpen004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialOpen004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialOpen005
     * @tc.desc       : OH_UsbSerial_Open Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialOpen005', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialOpen005---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialOpenFive(deviceId);
        console.info("Test Result testUsbSerialOpen005 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_DEVICE_NOT_FOUND);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialOpen005 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialWrite001
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialWrite001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialWrite001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialWriteOne(deviceId);
        console.info("Test Result testUsbSerialWrite001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialWrite001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialWrite002
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialWrite002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialWrite002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialWriteTwo(deviceId);
        console.info("Test Result testUsbSerialWrite002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialWrite002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialWrite003
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialWrite003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialWrite003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialWriteThree(deviceId);
        console.info("Test Result testUsbSerialWrite003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialWrite003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialWrite004
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialWrite004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialWrite004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialWriteFour(deviceId);
        console.info("Test Result testUsbSerialWrite004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialWrite004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialClose001
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialClose001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialClose001---------------------------');
      try {
        const ret = serialDdkNdkTest.usbSerialCloseOne();
        console.info("Test Result testUsbSerialClose001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialClose001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialClose002
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialClose002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialClose002---------------------------');
      try {
        const ret = serialDdkNdkTest.usbSerialCloseTwo();
        console.info("Test Result testUsbSerialClose002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialClose002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialClose003
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialClose003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialClose003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = serialDdkNdkTest.usbSerialCloseThree(deviceId);
        console.info("Test Result testUsbSerialClose003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialClose003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialClose004
     * @tc.desc       : OH_UsbSerial_Write Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialClose004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialClose004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        const ret = serialDdkNdkTest.usbSerialCloseFour(deviceId);
        console.info("Test Result testUsbSerialClose004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialClose004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialRead001
     * @tc.desc       : OH_UsbSerial_Read Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialRead001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRead001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialReadOne(deviceId);
        console.info("Test Result testUsbSerialRead001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRead001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialRead002
     * @tc.desc       : OH_UsbSerial_Read Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialRead002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRead002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialReadTwo(deviceId);
        console.info("Test Result testUsbSerialRead002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRead002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialRead003
     * @tc.desc       : OH_UsbSerial_Read Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialRead003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRead003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialReadThree(deviceId);
        console.info("Test Result testUsbSerialRead003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRead003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialRead004
     * @tc.desc       : OH_UsbSerial_Read Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialRead004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialRead004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialReadFour(deviceId);
        console.info("Test Result testUsbSerialRead004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialRead004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetBaudRate001
     * @tc.desc       : OH_UsbSerial_SetBaudRate Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetBaudRate001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetBaudRate001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetBaudRateOne(deviceId);
        console.info("Test Result testUsbSerialSetBaudRate001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetBaudRate001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetBaudRate002
     * @tc.desc       : OH_UsbSerial_SetBaudRate Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetBaudRate002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetBaudRate002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetBaudRateTwo(deviceId);
        console.info("Test Result testUsbSerialSetBaudRate002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetBaudRate002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetBaudRate003
     * @tc.desc       : OH_UsbSerial_SetBaudRate Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetBaudRate003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetBaudRate003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetBaudRateThree(deviceId);
        console.info("Test Result testUsbSerialSetBaudRate003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetBaudRate003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetBaudRate004
     * @tc.desc       : OH_UsbSerial_SetBaudRate Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetBaudRate004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetBaudRate004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetBaudRateFour(deviceId);
        console.info("Test Result testUsbSerialSetBaudRate004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetBaudRate004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetParams001
     * @tc.desc       : OH_UsbSerial_SetParams Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetParams001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetParams001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetParamsOne(deviceId);
        console.info("Test Result testUsbSerialSetParams001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetParams001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetParams002
     * @tc.desc       : OH_UsbSerial_SetParams Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetParams002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetParams002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetParamsTwo(deviceId);
        console.info("Test Result testUsbSerialSetParams002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetParams002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetParams003
     * @tc.desc       : OH_UsbSerial_SetParams Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetParams003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetParams003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetParamsThree(deviceId);
        console.info("Test Result testUsbSerialSetParams003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetParams003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetParams004
     * @tc.desc       : OH_UsbSerial_SetParams Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetParams004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetParams004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetParamsFour(deviceId);
        console.info("Test Result testUsbSerialSetParams004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetParams004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetTimeout001
     * @tc.desc       : OH_UsbSerial_SetTimeout Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetTimeout001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetTimeout001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetTimeoutOne(deviceId);
        console.info("Test Result testUsbSerialSetTimeout001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetTimeout001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetTimeout002
     * @tc.desc       : OH_UsbSerial_SetTimeout Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetTimeout002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetTimeout002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetTimeoutTwo(deviceId);
        console.info("Test Result testUsbSerialSetTimeout002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetTimeout002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetTimeout003
     * @tc.desc       : OH_UsbSerial_SetTimeout Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetTimeout003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetTimeout003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetTimeoutThree(deviceId);
        console.info("Test Result testUsbSerialSetTimeout003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetTimeout003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetTimeout004
     * @tc.desc       : OH_UsbSerial_SetTimeout Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetTimeout004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetTimeout004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetTimeoutFour(deviceId);
        console.info("Test Result testUsbSerialSetTimeout004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetTimeout004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetTimeout005
     * @tc.desc       : OH_UsbSerial_SetTimeout Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetTimeout005', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetTimeout005---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetTimeoutFive(deviceId);
        console.info("Test Result testUsbSerialSetTimeout005 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetTimeout005 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetFlowControl001
     * @tc.desc       : OH_UsbSerial_SetFlowControl Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetFlowControl001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetFlowControl001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetFlowControlOne(deviceId);
        console.info("Test Result testUsbSerialSetFlowControl001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetFlowControl001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetFlowControl002
     * @tc.desc       : OH_UsbSerial_SetFlowControl Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetFlowControl002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetFlowControl002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetFlowControlTwo(deviceId);
        console.info("Test Result testUsbSerialSetFlowControl002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetFlowControl002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetFlowControl003
     * @tc.desc       : OH_UsbSerial_SetFlowControl Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetFlowControl003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetFlowControl003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetFlowControlThree(deviceId);
        console.info("Test Result testUsbSerialSetFlowControl003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetFlowControl003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialSetFlowControl004
     * @tc.desc       : OH_UsbSerial_SetFlowControl Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialSetFlowControl004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialSetFlowControl004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialSetFlowControlFour(deviceId);
        console.info("Test Result testUsbSerialSetFlowControl004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialSetFlowControl004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlush001
     * @tc.desc       : OH_UsbSerial_Flush Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlush001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlush001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushOne(deviceId);
        console.info("Test Result testUsbSerialFlush001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlush001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlush002
     * @tc.desc       : OH_UsbSerial_Flush Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlush002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlush002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushTwo(deviceId);
        console.info("Test Result testUsbSerialFlush002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlush002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlush003
     * @tc.desc       : OH_UsbSerial_Flush Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlush003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlush003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushThree(deviceId);
        console.info("Test Result testUsbSerialFlush003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlush003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlush004
     * @tc.desc       : OH_UsbSerial_Flush Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlush004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlush004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushFour(deviceId);
        console.info("Test Result testUsbSerialFlush004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlush004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushInput001
     * @tc.desc       : OH_UsbSerial_FlushInput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushInput001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushInput001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushInputOne(deviceId);
        console.info("Test Result testUsbSerialFlushInput001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushInput001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushInput002
     * @tc.desc       : OH_UsbSerial_FlushInput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushInput002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushInput002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushInputTwo(deviceId);
        console.info("Test Result testUsbSerialFlushInput002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushInput002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushInput003
     * @tc.desc       : OH_UsbSerial_FlushInput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushInput003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushInput003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushInputThree(deviceId);
        console.info("Test Result testUsbSerialFlushInput003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushInput003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushInput004
     * @tc.desc       : OH_UsbSerial_FlushInput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushInput004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushInput004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushInputFour(deviceId);
        console.info("Test Result testUsbSerialFlushInput004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushInput004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushOutput001
     * @tc.desc       : OH_UsbSerial_FlushOutput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushOutput001', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushOutput001---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushOutputOne(deviceId);
        console.info("Test Result testUsbSerialFlushOutput001 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INIT_ERROR);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushOutput001 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushOutput002
     * @tc.desc       : OH_UsbSerial_FlushOutput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushOutput002', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushOutput002---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushOutputTwo(deviceId);
        console.info("Test Result testUsbSerialFlushOutput002 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_PARAMETER);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushOutput002 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushOutput003
     * @tc.desc       : OH_UsbSerial_FlushOutput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushOutput003', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushOutput003---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushOutputThree(deviceId);
        console.info("Test Result testUsbSerialFlushOutput003 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_INVALID_OPERATION);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushOutput003 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });

    /**
     * @tc.name       : testUsbSerialFlushOutput004
     * @tc.desc       : OH_UsbSerial_FlushOutput Interface testing
     * @tc.type       : Function
     */
    it('testUsbSerialFlushOutput004', 0, async (done: Function) => {
      console.info('----------------------testUsbSerialFlushOutput004---------------------------');
      try {
        if (deviceId < 0) {
          console.info("Test USB SERIAL device not connected");
          expect(true).assertTrue();
          done();
          return;
        }
        console.info("Test USB SERIAL deviceId = " + deviceId);
        const ret = serialDdkNdkTest.usbSerialFlushOutputFour(deviceId);
        console.info("Test Result testUsbSerialFlushOutput004 : " + ret);
        expect(ret).assertEqual(USB_SERIAL_DDK_SUCCESS);
        done();
      } catch (err) {
        console.error(TAG, `testUsbSerialFlushOutput004 failed, message is ${err.message}`);
        expect(false).assertTrue();
        done();
      }
    });
  })
}