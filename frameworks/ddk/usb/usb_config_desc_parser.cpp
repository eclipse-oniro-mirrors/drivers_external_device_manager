/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "usb_config_desc_parser.h"

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "securec.h"
#include "usb_ddk_types.h"

namespace OHOS {
namespace ExternalDeviceManager {
struct UsbDescriptorHeader {
    uint8_t bLength;
    uint8_t bDescriptorType;
} __attribute__((packed));

enum UsbDdkDescriptorType {
    USB_DDK_CONFIG_DESCRIPTOR_TYPE,
    USB_DDK_INTERFACE_DESCRIPTOR_TYPE,
    USB_DDK_ENDPOINT_DESCRIPTOR_TYPE,
    USB_DDK_AUDIO_ENDPOINT_DESCRIPTOR_TYPE,
};

constexpr int32_t USB_MAXENDPOINTS = 32;
constexpr int32_t USB_MAXALTSETTING = 128;
constexpr int32_t DESC_HEADER_LENGTH = 2;
constexpr int32_t USB_MAXINTERFACES = 32;
constexpr int32_t USB_DDK_DT_CONFIG_SIZE = 0x09;
constexpr int32_t USB_DDK_DT_INTERFACE_SIZE = 0x09;
constexpr int32_t USB_DDK_DT_ENDPOINT_SIZE = 0x07;
constexpr int32_t USB_DDK_DT_CONFIG = 0x02;
constexpr int32_t USB_DDK_DT_INTERFACE = 0x04;
constexpr int32_t USB_DDK_DT_ENDPOINT = 0x05;

static uint16_t Le16ToHost(uint16_t number)
{
    uint8_t *addr = reinterpret_cast<uint8_t *>(&number);
    uint16_t result = static_cast<uint16_t>(addr[1] << 8) | addr[0];
    return result;
}

static int32_t GetDescriptorLength(UsbDdkDescriptorType descriptorType)
{
    switch (descriptorType) {
        case USB_DDK_CONFIG_DESCRIPTOR_TYPE:
            return USB_DDK_DT_CONFIG_SIZE;
        case USB_DDK_INTERFACE_DESCRIPTOR_TYPE:
            return USB_DDK_DT_INTERFACE_SIZE;
        case USB_DDK_ENDPOINT_DESCRIPTOR_TYPE:
            return USB_DDK_DT_ENDPOINT_SIZE;
        default:
            EDM_LOGE(MODULE_USB_DDK, "invalid descriptorType:%{public}d", descriptorType);
    }
    return UINT32_MAX;
}

static int32_t ParseDescriptor(
    UsbDdkDescriptorType descriptorType, uint8_t *dest, uint32_t destLen, const uint8_t *source, int32_t sourceLen)
{
    if (source == nullptr || dest == nullptr) {
        EDM_LOGE(
            MODULE_USB_DDK, "invalid param, source:%{public}d, dest:%{public}d", source == nullptr, dest == nullptr);
        return -EDM_ERR_INVALID_PARAM;
    }

    int32_t descriptorLen = GetDescriptorLength(descriptorType);
    if (descriptorLen == UINT32_MAX) {
        return -EDM_ERR_INVALID_PARAM;
    }

    if (sourceLen < descriptorLen) {
        EDM_LOGE(MODULE_USB_DDK, "invalid sourceLen:%{public}u, descriptorType:%{public}d", sourceLen, descriptorType);
        return -EDM_ERR_INVALID_PARAM;
    }

    int32_t ret = memcpy_s(dest, destLen, source, descriptorLen);
    if (ret != EOK) {
        EDM_LOGE(MODULE_USB_DDK, "memcpy_s failed, ret = %{public}d", ret);
        return -EDM_ERR_INVALID_PARAM;
    }

    switch (descriptorType) {
        case USB_DDK_CONFIG_DESCRIPTOR_TYPE: {
            struct UsbConfigDescriptor *desc = (struct UsbConfigDescriptor *)dest;
            desc->wTotalLength = Le16ToHost(desc->wTotalLength);
            break;
        }
        case USB_DDK_INTERFACE_DESCRIPTOR_TYPE:
            break;
        case USB_DDK_ENDPOINT_DESCRIPTOR_TYPE: {
            UsbEndpointDescriptor *desc = (UsbEndpointDescriptor *)dest;
            desc->wMaxPacketSize = Le16ToHost(desc->wMaxPacketSize);
            break;
        }
        default:
            EDM_LOGE(MODULE_USB_DDK, "invalid descriptorType:%{public}d", descriptorType);
            return -EDM_ERR_INVALID_PARAM;
    }
    return EDM_OK;
}

static int32_t FindNextDescriptor(const uint8_t *buffer, int32_t size)
{
    UsbDescriptorHeader *header = nullptr;
    const uint8_t *buffer0 = buffer;

    while (size >= sizeof(UsbDescriptorHeader)) {
        header = (UsbDescriptorHeader *)buffer;
        if (header->bDescriptorType == USB_DDK_DT_INTERFACE || header->bDescriptorType == USB_DDK_DT_ENDPOINT) {
            break;
        }
        buffer += header->bLength;
        size -= header->bLength;
    }

    return buffer - buffer0;
}

static int32_t FillExtraDescriptor(
    const unsigned char **extra, uint32_t *extraLength, const uint8_t *buffer, int32_t bufferLen)
{
    if (bufferLen == 0) {
        EDM_LOGE(MODULE_USB_DDK, "invalid param");
        return -EDM_ERR_INVALID_PARAM;
    }

    uint32_t extraLenTmp = *extraLength + bufferLen;
    unsigned char *extraTmp = new unsigned char[extraLenTmp];
    if (extraTmp == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "new failed");
        return -EDM_EER_MALLOC_FAIL;
    }
    memset_s((void *)extraTmp, extraLenTmp, 0, extraLenTmp);

    if (*extra != nullptr && *extraLength != 0) {
        if (memcpy_s(extraTmp, extraLenTmp, *extra, *extraLength) != EOK) {
            EDM_LOGE(MODULE_USB_DDK, "copy extra failed");
            delete[] extraTmp;
            return -EDM_ERR_INVALID_PARAM;
        }
    }

    if (memcpy_s(extraTmp + *extraLength, extraLenTmp - *extraLength, buffer, bufferLen) != EOK) {
        EDM_LOGE(MODULE_USB_DDK, "copy buffer failed");
        delete[] extraTmp;
        return -EDM_ERR_INVALID_PARAM;
    }

    if (*extra != nullptr) {
        delete[] (*extra);
        *extra = extraTmp;
        *extraLength = extraLenTmp;
    }

    return EDM_OK;
}

static int32_t ParseEndpoint(UsbDdkEndpointDescriptor *endPoint, const uint8_t *buffer, int32_t size)
{
    const uint8_t *buffer0 = buffer;
    const UsbDescriptorHeader *header = nullptr;
    int32_t len;
    int32_t ret;

    if (size < DESC_HEADER_LENGTH) {
        EDM_LOGE(MODULE_USB_DDK, "size = %{public}d is short endPoint descriptor", size);
        return -EDM_ERR_IO;
    }

    header = (const UsbDescriptorHeader *)buffer;
    if ((header->bDescriptorType != USB_DDK_DT_ENDPOINT) || (header->bLength > size)) {
        EDM_LOGE(MODULE_USB_DDK, "unexpected descriptor, type = 0x%{public}x, length = %{public}hhu",
            header->bDescriptorType, header->bLength);
        return buffer - buffer0;
    } else if (header->bLength < USB_DDK_DT_ENDPOINT_SIZE) {
        EDM_LOGE(MODULE_USB_DDK, "invalid endpoint length = %{public}hhu", header->bLength);
        return -EDM_ERR_IO;
    }

    ParseDescriptor(USB_DDK_ENDPOINT_DESCRIPTOR_TYPE, (uint8_t *)endPoint, sizeof(UsbEndpointDescriptor), buffer, size);

    buffer += header->bLength;
    size -= header->bLength;

    len = FindNextDescriptor(buffer, size);
    if (!len) {
        return buffer - buffer0;
    }
    ret = FillExtraDescriptor(&endPoint->extra, &endPoint->extraLength, buffer, len);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "FillExtraDescriptor failed");
        return ret;
    }
    return buffer + len - buffer0;
}

static int32_t RawParseDescriptor(
    int32_t size, const uint8_t *buffer, UsbDdkDescriptorType bDescriptorType, UsbDdkInterfaceDescriptor &ddkIntfDesc)
{
    int32_t ret =
        ParseDescriptor(bDescriptorType, (uint8_t *)&ddkIntfDesc, sizeof(UsbInterfaceDescriptor), buffer, size);
    if (ret != EDM_OK) {
        EDM_LOGE(MODULE_USB_DDK, "ParseDescriptor failed");
        return ret;
    }
    if ((ddkIntfDesc.interfaceDescriptor.bDescriptorType != USB_DDK_DT_INTERFACE) ||
        (ddkIntfDesc.interfaceDescriptor.bLength > size)) {
        EDM_LOGE(MODULE_USB_DDK, "unexpected descriptor: type = 0x%{public}x, size = %{public}d",
            ddkIntfDesc.interfaceDescriptor.bDescriptorType, size);
        ret = -EDM_NOK;
    } else if ((ddkIntfDesc.interfaceDescriptor.bLength < USB_DDK_DT_INTERFACE_SIZE) ||
        (ddkIntfDesc.interfaceDescriptor.bNumEndpoints > USB_MAXENDPOINTS)) {
        EDM_LOGE(MODULE_USB_DDK, "invalid descriptor: length = %{public}u, numEndpoints = %{public}u",
            ddkIntfDesc.interfaceDescriptor.bLength, ddkIntfDesc.interfaceDescriptor.bNumEndpoints);
        ret = -EDM_ERR_IO;
    }

    return ret;
}

static int32_t ParseInterfaceEndpoint(UsbDdkInterfaceDescriptor &ddkIntfDesc, const uint8_t **buffer, int32_t *size)
{
    UsbDdkEndpointDescriptor *endPoint = nullptr;
    int32_t ret = EDM_OK;

    if (ddkIntfDesc.interfaceDescriptor.bNumEndpoints > 0) {
        endPoint = new UsbDdkEndpointDescriptor[ddkIntfDesc.interfaceDescriptor.bNumEndpoints];
        if (endPoint == nullptr) {
            ret = -EDM_EER_MALLOC_FAIL;
            return ret;
        }
        auto len = ddkIntfDesc.interfaceDescriptor.bNumEndpoints * sizeof(UsbDdkEndpointDescriptor);
        memset_s((void *)endPoint, len, 0, len);

        ddkIntfDesc.endPoint = endPoint;
        for (uint8_t i = 0; i < ddkIntfDesc.interfaceDescriptor.bNumEndpoints; i++) {
            ret = ParseEndpoint(endPoint + i, *buffer, *size);
            if (ret == 0) {
                ddkIntfDesc.interfaceDescriptor.bNumEndpoints = i;
                break;
            } else if (ret < 0) {
                return ret;
            }

            *buffer += ret;
            *size -= ret;
        }
    }
    return ret;
}

static void GetInterfaceNumberDes(
    const UsbDescriptorHeader *header, std::vector<uint8_t> &interfaceNums, std::vector<uint8_t> &alternateSetting)
{
    UsbInterfaceDescriptor *desc = (UsbInterfaceDescriptor *)header;
    if (desc->bLength < USB_DDK_DT_INTERFACE_SIZE) {
        EDM_LOGW(MODULE_USB_DDK, "invalid interface descriptor length %{public}d, skipping", desc->bLength);
        return;
    }

    uint8_t intfNum = desc->bInterfaceNumber;
    size_t currentSize = interfaceNums.size();
    size_t i;
    for (i = 0; i < currentSize; ++i) {
        if (interfaceNums[i] == intfNum) {
            break;
        }
    }
    if (i < currentSize) {
        if (alternateSetting[i] < USB_MAXALTSETTING) {
            ++(alternateSetting[i]);
        }
    } else if (currentSize < USB_MAXINTERFACES) {
        interfaceNums.emplace_back(intfNum);
        alternateSetting.emplace_back(1);
    }
}

// return number of interfaces
// intf contains all interface numbers
// alts contains the number of alternate settings on the corresponding interface
static void GetInterfaceNumber(
    const uint8_t *buffer, int32_t size, std::vector<uint8_t> &interfaceNums, std::vector<uint8_t> &alternateSetting)
{
    UsbDescriptorHeader *header = nullptr;
    const uint8_t *buffer2;
    int32_t size2;

    for ((buffer2 = buffer, size2 = size); size2 > 0; (buffer2 += header->bLength, size2 -= header->bLength)) {
        if (size2 < sizeof(UsbDescriptorHeader)) {
            EDM_LOGW(MODULE_USB_DDK, "descriptor has %{public}d excess bytes", size2);
            break;
        }
        header = (UsbDescriptorHeader *)buffer2;
        if ((header->bLength > size2) || (header->bLength < sizeof(UsbDescriptorHeader))) {
            EDM_LOGW(MODULE_USB_DDK, "invalid descriptor length %{public}hhu, skipping remainder", header->bLength);
            break;
        }

        if (header->bDescriptorType == USB_DDK_DT_INTERFACE) {
            GetInterfaceNumberDes(header, interfaceNums, alternateSetting);
        }
    }
}

static int32_t ParseInterface(UsbDdkInterface &usbInterface, const uint8_t *buffer, int32_t size)
{
    const uint8_t *buffer0 = buffer;
    int32_t interfaceNumber = -1; // initial value of interfaceNumber is -1
    const UsbInterfaceDescriptor *ifDesc = nullptr;

    if (usbInterface.numAltsetting > USB_MAXALTSETTING) {
        EDM_LOGE(MODULE_USB_DDK, "usbInterface is null or numAltsetting is invalid");
        return -EDM_ERR_OUT_OF_RANGE;
    }

    while (size >= USB_DDK_DT_INTERFACE_SIZE) {
        UsbDdkInterfaceDescriptor &ddkIntfDesc = usbInterface.altsetting[usbInterface.numAltsetting];
        int32_t ret = RawParseDescriptor(size, buffer, USB_DDK_INTERFACE_DESCRIPTOR_TYPE, ddkIntfDesc);
        if (ret == -EDM_NOK) {
            return buffer - buffer0;
        } else if (ret == -EDM_ERR_IO) {
            EDM_LOGE(MODULE_USB_DDK, "RawParseDescriptor failed");
            return ret;
        }

        usbInterface.numAltsetting++;
        ddkIntfDesc.extra = nullptr;
        ddkIntfDesc.extraLength = 0;
        ddkIntfDesc.endPoint = nullptr;
        if (interfaceNumber == -1) {
            interfaceNumber = ddkIntfDesc.interfaceDescriptor.bInterfaceNumber;
        }

        buffer += ddkIntfDesc.interfaceDescriptor.bLength;
        size -= ddkIntfDesc.interfaceDescriptor.bLength;
        int32_t len = FindNextDescriptor(buffer, size);
        if (len != 0) {
            if (FillExtraDescriptor(&ddkIntfDesc.extra, &ddkIntfDesc.extraLength, buffer, len) != EDM_OK) {
                EDM_LOGE(MODULE_USB_DDK, "FillExtraDescriptor failed");
                return -EDM_NOK;
            }
            buffer += len;
            size -= len;
        }

        ret = ParseInterfaceEndpoint(ddkIntfDesc, &buffer, &size);
        if (ret < EDM_OK) {
            EDM_LOGE(MODULE_USB_DDK, "ParseInterfaceEndpoint, ret less than zero");
            return ret;
        }

        ifDesc = (const UsbInterfaceDescriptor *)buffer;
        bool tempFlag = (size < USB_DDK_DT_INTERFACE_SIZE) || (ifDesc->bDescriptorType != USB_DDK_DT_INTERFACE) ||
            (ifDesc->bInterfaceNumber != interfaceNumber);
        if (tempFlag == true) {
            return buffer - buffer0;
        }
    }

    return buffer - buffer0;
}

static void ClearEndpoint(UsbDdkEndpointDescriptor *endPoint)
{
    if ((endPoint != nullptr) && (endPoint->extra != nullptr)) {
        delete[] (endPoint->extra);
        endPoint->extra = nullptr;
    }
}

static void ClearInterface(UsbDdkInterface &usbInterface)
{
    UsbDdkInterfaceDescriptor *infPtr = nullptr;
    uint8_t i;
    uint8_t j;

    if (usbInterface.numAltsetting > USB_MAXALTSETTING) {
        EDM_LOGE(MODULE_USB_DDK, "numAltsetting = %{public}hhu is error", usbInterface.numAltsetting);
        return;
    }

    for (i = 0; i < usbInterface.numAltsetting; i++) {
        infPtr = (UsbDdkInterfaceDescriptor *)(usbInterface.altsetting + i);
        if (infPtr == nullptr) {
            EDM_LOGE(MODULE_USB_DDK, "altsetting is null");
            continue;
        }

        if (infPtr->extra != nullptr) {
            delete[] (infPtr->extra);
            infPtr->extra = nullptr;
        }

        if (infPtr->endPoint != nullptr) {
            for (j = 0; j < infPtr->interfaceDescriptor.bNumEndpoints; j++) {
                ClearEndpoint((UsbDdkEndpointDescriptor *)infPtr->endPoint + j);
            }

            delete[] (infPtr->endPoint);
            infPtr->endPoint = nullptr;
        }
    }

    delete[] usbInterface.altsetting;
}

void RawClearConfiguration(UsbDdkConfigDescriptor &config)
{
    uint8_t i;
    for (i = 0; i < config.configDescriptor.bNumInterfaces; i++) {
        ClearInterface(config.interface[i]);
        delete[] config.interface;
        config.interface = nullptr;
    }

    if (config.extra != nullptr) {
        delete[] (config.extra);
        config.extra = nullptr;
    }
}

static int32_t ParseConfigurationDes(
    UsbDdkConfigDescriptor &config, const uint8_t *buffer, int32_t size, std::vector<uint8_t> &interfaceNums)
{
    int32_t ret, len;
    uint8_t i;
    while (size >= sizeof(UsbDescriptorHeader)) {
        len = FindNextDescriptor(buffer, size);
        if (len != 0) {
            ret = FillExtraDescriptor(&config.extra, &config.extraLength, buffer, len);
            if (ret != EDM_OK) {
                EDM_LOGE(MODULE_USB_DDK, "FillExtraDescriptor failed");
                return ret;
            }
            buffer += len;
            size -= len;
        }

        if (size <= sizeof(UsbDescriptorHeader)) {
            break;
        }
        UsbInterfaceDescriptor *ifDesc = (UsbInterfaceDescriptor *)buffer;
        if (config.configDescriptor.bNumInterfaces >= USB_MAXINTERFACES) {
            EDM_LOGE(MODULE_USB_DDK, "%{public}d: bNumInterfaces overlong.", config.configDescriptor.bNumInterfaces);
            return -EDM_NOK;
        }
        for (i = 0; i < config.configDescriptor.bNumInterfaces; ++i) {
            if (interfaceNums[i] == ifDesc->bInterfaceNumber) {
                break;
            }
        }
        if (i == config.configDescriptor.bNumInterfaces) {
            EDM_LOGE(MODULE_USB_DDK, "%{public}u: bInterfaceNumber not found.", ifDesc->bInterfaceNumber);
            return -EDM_NOK;
        }
        ret = ParseInterface(config.interface[i], buffer, size);
        if (ret < 0) {
            EDM_LOGE(MODULE_USB_DDK, "%{public}u: Parse interface failed.", ifDesc->bInterfaceNumber);
            return ret;
        }

        buffer += ret;
        size -= ret;
    }

    return size;
}

// On error, return errcode, negative number
// On success, return 0, means all buffer are resolved into the config; return positive number, means buffer size that
// is not resolved
static int32_t ParseConfiguration(UsbDdkConfigDescriptor &config, const uint8_t *buffer, int32_t size)
{
    if (size < USB_DDK_DT_CONFIG_SIZE) {
        EDM_LOGE(MODULE_USB_DDK, "size = %{public}u is short, or config is null!", size);
        return -EDM_ERR_IO;
    }

    ParseDescriptor(
        USB_DDK_CONFIG_DESCRIPTOR_TYPE, (uint8_t *)&config, sizeof(struct UsbConfigDescriptor), buffer, size);
    if ((config.configDescriptor.bDescriptorType != USB_DDK_DT_CONFIG) ||
        (config.configDescriptor.bLength < USB_DDK_DT_CONFIG_SIZE) ||
        (config.configDescriptor.bLength > (uint8_t)size) ||
        (config.configDescriptor.bNumInterfaces > USB_MAXINTERFACES)) {
        EDM_LOGE(MODULE_USB_DDK, "invalid descriptor: type = 0x%{public}x, length = %{public}u",
            config.configDescriptor.bDescriptorType, config.configDescriptor.bLength);
        return -EDM_ERR_IO;
    }

    std::vector<uint8_t> interfaceNums;
    std::vector<uint8_t> alternateSetting;
    GetInterfaceNumber(buffer, size, interfaceNums, alternateSetting);

    size_t intfNum = interfaceNums.size();
    if (intfNum == 0 || intfNum > USB_MAXALTSETTING) {
        EDM_LOGE(MODULE_USB_DDK, "interface num is zero");
        return -EDM_ERR_OUT_OF_RANGE;
    }

    config.configDescriptor.bNumInterfaces = (uint8_t)intfNum;
    config.interface = new UsbDdkInterface[intfNum];
    if (config.interface == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "new UsbDdkInterface failed");
        return -EDM_EER_MALLOC_FAIL;
    }
    memset_s((void *)config.interface, sizeof(UsbDdkInterface) * intfNum, 0, sizeof(UsbDdkInterface) * intfNum);

    for (size_t i = 0; i < intfNum; ++i) {
        uint8_t j = alternateSetting[i];
        if (j > USB_MAXALTSETTING) {
            EDM_LOGE(MODULE_USB_DDK, "too many alternate settings: %{public}hhu", j);
            alternateSetting[i] = USB_MAXALTSETTING;
            j = USB_MAXALTSETTING;
        }
        config.interface[i].altsetting = new UsbDdkInterfaceDescriptor[j];
        if (config.interface[i].altsetting == nullptr) {
            EDM_LOGE(MODULE_USB_DDK, "new UsbDdkInterfaceDescriptor failed");
            return -EDM_EER_MALLOC_FAIL;
        }
        memset_s((void *)config.interface[i].altsetting, sizeof(UsbDdkInterfaceDescriptor) * j, 0,
            sizeof(UsbDdkInterfaceDescriptor) * j);
    }

    buffer += config.configDescriptor.bLength;
    size -= config.configDescriptor.bLength;

    return ParseConfigurationDes(config, buffer, size, interfaceNums);
}

int32_t ParseUsbConfigDescriptor(const std::vector<uint8_t> &configBuffer, UsbDdkConfigDescriptor ** const config)
{
    UsbDdkConfigDescriptor *tmpConfig = new UsbDdkConfigDescriptor();
    if (tmpConfig == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "new failed");
        return -EDM_EER_MALLOC_FAIL;
    }
    memset_s((void *)tmpConfig, sizeof(UsbDdkConfigDescriptor), 0, sizeof(UsbDdkConfigDescriptor));

    int32_t ret = ParseConfiguration(*tmpConfig, configBuffer.data(), configBuffer.size());
    if (ret < 0) {
        EDM_LOGE(MODULE_USB_DDK, "ParseConfiguration failed with error = %{public}d", ret);
        FreeUsbConfigDescriptor(tmpConfig);
        tmpConfig = nullptr;
        return ret;
    } else if (ret > 0) {
        EDM_LOGW(MODULE_USB_DDK, "still %{public}d bytes of descriptor data left", ret);
    }

    *config = tmpConfig;
    return ret;
}

void FreeUsbConfigDescriptor(UsbDdkConfigDescriptor * const config)
{
    if (config == nullptr) {
        EDM_LOGE(MODULE_USB_DDK, "config is nullptr");
        return;
    }

    RawClearConfiguration(*config);
    delete config;
}
} // namespace ExternalDeviceManager
} // namespace OHOS