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

#include "scsi_peripheral_api.h"
#include <cerrno>
#include <iproxy_broker.h>
#include <memory.h>
#include <mutex>
#include <scsi/sg.h>
#include <securec.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>

#include "edm_errors.h"
#include "hilog_wrapper.h"
#include "ipc_error_code.h"
#include "scsi_peripheral_types.h"
#include "v1_0/scsi_peripheral_ddk_service.h"

using namespace OHOS;
using namespace OHOS::ExternalDeviceManager;
namespace {
OHOS::sptr<OHOS::HDI::Usb::ScsiDdk::V1_0::IScsiPeripheralDdk> g_ddk = nullptr;
static OHOS::sptr<IRemoteObject::DeathRecipient> recipient_ = nullptr;
std::mutex g_mutex;

constexpr uint8_t ONE_BYTE = 1;
constexpr uint8_t TWO_BYTE = 2;
constexpr uint8_t THREE_BYTE = 3;
constexpr uint8_t FOUR_BYTE = 4;
constexpr uint8_t FIVE_BYTE = 5;
constexpr uint8_t SIX_BYTE = 6;
constexpr uint8_t SEVEN_BYTE = 7;
constexpr uint8_t EIGHT_BYTE = 8;
constexpr uint8_t FIFTEEN_BYTE = 15;
constexpr uint8_t EIGHT_BIT = 8;
constexpr uint8_t SIXTEEN_BIT = 16;
constexpr uint8_t TWENTY_FOUR_BIT = 24;
constexpr uint8_t THIRTY_TWO_BIT = 32;
constexpr uint8_t FORTY_BIT = 40;
constexpr uint8_t FORTY_EIGHT_BIT = 48;
constexpr uint8_t FIFTY_SIX_BIT = 56;
constexpr uint8_t DESCRIPTOR_TYPE_INFORMATION = 0x00;
constexpr uint8_t DESCRIPTOR_TYPE_COMMAND_SPECIFIC_INFORMATION = 0x01;
constexpr uint8_t DESCRIPTOR_TYPE_SENSE_KEY_SPECIFIC = 0x02;
constexpr uint8_t ADDITIONAL_LENGTH_TEN = 0x0A;
constexpr uint8_t ADDITIONAL_LENGTH_SIX = 0x06;
constexpr uint8_t VALID_BIT = 0x80;
constexpr uint8_t MAST_RESPONSE_CODE = 0x7F;
constexpr uint8_t RESPONSE_CODE_70H = 0x70;
constexpr uint8_t RESPONSE_CODE_71H = 0x71;
constexpr uint8_t RESPONSE_CODE_72H = 0x72;
constexpr uint8_t RESPONSE_CODE_73H = 0x73;
constexpr uint32_t MASK_SENSE_KEY_SPECIFIC = 0x007FFFFF;
} // namespace

struct ScsiPeripheral_Device {
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralDevice impl;
    int memMapFd = -1;

    ScsiPeripheral_Device()
    {
        impl.devFd = -1;
        impl.memMapFd = -1;
        impl.lbLength = 0;
    }
} __attribute__ ((aligned(8)));

ScsiPeripheral_Device *NewScsiPeripheralDevice()
{
    return new ScsiPeripheral_Device;
}

void DeleteScsiPeripheralDevice(ScsiPeripheral_Device **dev)
{
    if (*dev != nullptr) {
        delete *dev;
        *dev = nullptr;
    }
}

void SetDdk(OHOS::sptr<OHOS::HDI::Usb::ScsiDdk::V1_0::IScsiPeripheralDdk> &ddk)
{
    g_ddk = ddk;
}

static int32_t TransToDdkErrCode(int32_t ret)
{
    if (ret == HDF_SUCCESS) {
        return SCSIPERIPHERAL_DDK_SUCCESS;
    }
    if (ret >= OH_IPC_ERROR_CODE_BASE && ret <= OH_IPC_ERROR_CODE_MAX) {
        return SCSIPERIPHERAL_DDK_SERVICE_ERROR;
    }
    return ret;
}

static inline uint32_t GetUint24(unsigned char *buf, int start)
{
    return (
        (static_cast<uint32_t>(buf[start]) << SIXTEEN_BIT) |
        (static_cast<uint32_t>(buf[start + ONE_BYTE]) << EIGHT_BIT) |
        (static_cast<uint32_t>(buf[start + TWO_BYTE]))
    );
}

static inline uint32_t GetUint32(unsigned char *buf, int start)
{
    return (
        (static_cast<uint32_t>(buf[start]) << TWENTY_FOUR_BIT) |
        (static_cast<uint32_t>(buf[start + ONE_BYTE]) << SIXTEEN_BIT) |
        (static_cast<uint32_t>(buf[start + TWO_BYTE]) << EIGHT_BIT) |
        (static_cast<uint32_t>(buf[start + THREE_BYTE]))
    );
}

static inline uint64_t GetUint64(unsigned char *buf, int start)
{
    return (
        (static_cast<uint64_t>(buf[start]) << FIFTY_SIX_BIT) |
        (static_cast<uint64_t>(buf[start + ONE_BYTE]) << FORTY_EIGHT_BIT) |
        (static_cast<uint64_t>(buf[start + TWO_BYTE]) << FORTY_BIT) |
        (static_cast<uint64_t>(buf[start + THREE_BYTE]) << THIRTY_TWO_BIT) |
        (static_cast<uint64_t>(buf[start + FOUR_BYTE]) << TWENTY_FOUR_BIT) |
        (static_cast<uint64_t>(buf[start + FIVE_BYTE]) << SIXTEEN_BIT) |
        (static_cast<uint64_t>(buf[start + SIX_BYTE]) << EIGHT_BIT) |
        (static_cast<uint64_t>(buf[start + SEVEN_BYTE]))
    );
}

static bool CopyDataToArray(const std::vector<uint8_t> &data, char *arr, uint32_t arrLen)
{
    if (arr == nullptr || data.size() > arrLen) {
        return false;
    }

    size_t i = 0;
    while (i < data.size()) {
        arr[i] = data[i];
        ++i;
    }
    arr[arrLen - 1] = '\0';

    return true;
}

static int32_t CopyResponse(const OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse &srcResp,
    ScsiPeripheral_Response *desResp)
{
    errno_t err = memcpy_s(desResp->senseData, sizeof(desResp->senseData), srcResp.senseData.data(),
        srcResp.senseData.size());
    if (err != EOK) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "memcpy_s failed, err=%{public}d, srcSize=%{public}zu, desSize=%{public}zu",
            err, srcResp.senseData.size(), sizeof(desResp->senseData));
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }

    desResp->status = static_cast<ScsiPeripheral_Status>(srcResp.status);
    desResp->maskedStatus = srcResp.maskedStatus;
    desResp->msgStatus = srcResp.msgStatus;
    desResp->sbLenWr = srcResp.sbLenWr;
    desResp->hostStatus = srcResp.hostStatus;
    desResp->driverStatus = srcResp.driverStatus;
    desResp->resId = srcResp.resId;
    desResp->duration = srcResp.duration;

    return SCSIPERIPHERAL_DDK_SUCCESS;
}

static void ToHdi(ScsiPeripheral_IORequest *request, OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralIORequest &hdiRequest)
{
    hdiRequest.lbAddress = request->lbAddress;
    hdiRequest.transferLength = request->transferLength;
    hdiRequest.control = request->control;
    hdiRequest.byte1 = request->byte1;
    hdiRequest.byte6 = request->byte6;
    hdiRequest.memMapSize = request->data->size;
    hdiRequest.timeout = request->timeout;
}

static int32_t ParseDescriptorFormatSense(uint8_t *senseData, uint8_t senseDataLen,
    ScsiPeripheral_BasicSenseInfo *senseInfo)
{
    if (senseDataLen < SCSIPERIPHERAL_MIN_DESCRIPTOR_FORMAT_SENSE) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "sense data len is invalid");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    uint8_t additionalSenseLen = senseData[EIGHT_BYTE - 1];
    if (senseDataLen > additionalSenseLen + EIGHT_BYTE) {
        senseDataLen = additionalSenseLen + EIGHT_BYTE;
    }

    uint8_t idx;
    uint8_t descLen;

    for (idx = EIGHT_BYTE; idx < senseDataLen; idx += descLen + TWO_BYTE) {
        uint8_t descType = senseData[idx];
        descLen = senseData[idx + 1];

        if (idx + descLen + TWO_BYTE > senseDataLen) {
            break;
        }

        switch (descType) {
            case DESCRIPTOR_TYPE_INFORMATION:
                if (descLen == ADDITIONAL_LENGTH_TEN) {
                    senseInfo->valid = senseData[idx + TWO_BYTE] & VALID_BIT;
                    senseInfo->information = GetUint64(senseData, idx + FOUR_BYTE);
                }
                break;
            case DESCRIPTOR_TYPE_COMMAND_SPECIFIC_INFORMATION:
                if (descLen == ADDITIONAL_LENGTH_TEN) {
                    senseInfo->commandSpecific = GetUint64(senseData, idx + FOUR_BYTE);
                }
                break;
            case DESCRIPTOR_TYPE_SENSE_KEY_SPECIFIC:
                if (descLen == ADDITIONAL_LENGTH_SIX) {
                    senseInfo->sksv = senseData[idx + FOUR_BYTE] & VALID_BIT;
                    senseInfo->senseKeySpecific = GetUint24(senseData, idx + FOUR_BYTE) & MASK_SENSE_KEY_SPECIFIC;
                }
                break;
            default:
                break;
        }
    }

    return SCSIPERIPHERAL_DDK_SUCCESS;
}

static int32_t ParseFixedFormatSense(uint8_t *senseData, uint8_t senseDataLen, ScsiPeripheral_BasicSenseInfo *senseInfo)
{
    if (senseDataLen < SCSIPERIPHERAL_MIN_FIXED_FORMAT_SENSE) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "sense data len is invalid");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    senseInfo->valid = senseData[0] & VALID_BIT;

    if (senseInfo->valid) {
        senseInfo->information = GetUint32(senseData, THREE_BYTE);
    }
    senseInfo->commandSpecific = GetUint32(senseData, EIGHT_BYTE);
    senseInfo->sksv = senseData[FIFTEEN_BYTE] & VALID_BIT;
    senseInfo->senseKeySpecific = GetUint24(senseData, FIFTEEN_BYTE) & MASK_SENSE_KEY_SPECIFIC;

    return SCSIPERIPHERAL_DDK_SUCCESS;
}

class ScsiPeripheralDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

void ScsiPeripheralDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    EDM_LOGI(MODULE_SCSIPERIPHERAL_DDK, "scsi_ddk remote died");
    if (g_ddk == nullptr) {
        return;
    }
    auto remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Usb::ScsiDdk::V1_0::IScsiPeripheralDdk>(g_ddk);
    remote->RemoveDeathRecipient(recipient_);
    recipient_.clear();
    g_ddk = nullptr;
    EDM_LOGI(MODULE_SCSIPERIPHERAL_DDK, "remove death recipient success");
}

int32_t OH_ScsiPeripheral_Init(void)
{
    auto ddk = OHOS::HDI::Usb::ScsiDdk::V1_0::IScsiPeripheralDdk::Get();
    SetDdk(ddk);
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "get ddk failed");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    recipient_ = new ScsiPeripheralDeathRecipient();
    sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<OHOS::HDI::Usb::ScsiDdk::V1_0::IScsiPeripheralDdk>(g_ddk);
    if (!remote->AddDeathRecipient(recipient_)) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "add DeathRecipient failed");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    return TransToDdkErrCode(g_ddk->Init());
}

int32_t OH_ScsiPeripheral_Release(void)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "ddk is null");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    int32_t ret = g_ddk->Release();
    g_ddk.clear();

    return TransToDdkErrCode(ret);
}

int32_t OH_ScsiPeripheral_Open(uint64_t deviceId, uint8_t interfaceIndex, ScsiPeripheral_Device **dev)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    *dev = NewScsiPeripheralDevice();
    if (*dev == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK,  "malloc failed, errno=%{public}d", errno);
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }

    return TransToDdkErrCode(g_ddk->Open(deviceId, interfaceIndex, (*dev)->impl, (*dev)->memMapFd));
}

int32_t OH_ScsiPeripheral_Close(ScsiPeripheral_Device **dev)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || *dev == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    int32_t ret = TransToDdkErrCode(g_ddk->Close((*dev)->impl));
    if (close((*dev)->memMapFd)) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "close memMapFd failed, errno=%{public}d", errno);
        if (ret == SCSIPERIPHERAL_DDK_SUCCESS) {
            ret = SCSIPERIPHERAL_DDK_IO_ERROR;
        }
    }
    DeleteScsiPeripheralDevice(dev);
    return ret;
}

int32_t OH_ScsiPeripheral_TestUnitReady(ScsiPeripheral_Device *dev, ScsiPeripheral_TestUnitReadyRequest *request,
    ScsiPeripheral_Response *response)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    auto hdiRequest = reinterpret_cast<OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralTestUnitReadyRequest *>(request);
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));
    int32_t ret = TransToDdkErrCode(g_ddk->TestUnitReady(dev->impl, *hdiRequest, hdiResponse));
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "test unit ready failed");
        return ret;
    }

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_Inquiry(ScsiPeripheral_Device *dev, ScsiPeripheral_InquiryRequest *request,
    ScsiPeripheral_InquiryInfo *inquiryInfo, ScsiPeripheral_Response *response)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || inquiryInfo == nullptr || inquiryInfo->data == nullptr ||
        response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralInquiryRequest hdiInquiryRequest;
    hdiInquiryRequest.pageCode = request->pageCode;
    hdiInquiryRequest.allocationLength = request->allocationLength;
    hdiInquiryRequest.control = request->control;
    hdiInquiryRequest.byte1 = request->byte1;
    hdiInquiryRequest.memMapSize = inquiryInfo->data->size;
    hdiInquiryRequest.timeout = request->timeout;

    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralInquiryInfo hdiInquiryInfo;
    hdiInquiryInfo.idVendor.resize(SCSIPERIPHERAL_VENDOR_ID_LEN);
    hdiInquiryInfo.idProduct.resize(sizeof(inquiryInfo->idProduct));
    hdiInquiryInfo.revProduct.resize(sizeof(inquiryInfo->revProduct));
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));

    int32_t ret = TransToDdkErrCode(g_ddk->Inquiry(dev->impl, hdiInquiryRequest, hdiInquiryInfo, hdiResponse));
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "inquiry failed");
        return ret;
    }

    inquiryInfo->deviceType = hdiInquiryInfo.deviceType;
    if (!CopyDataToArray(hdiInquiryInfo.idVendor, inquiryInfo->idVendor, sizeof(inquiryInfo->idVendor))) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "copy idVendor failed");
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }
    if (!CopyDataToArray(hdiInquiryInfo.idProduct, inquiryInfo->idProduct, sizeof(inquiryInfo->idProduct))) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "copy idProduct failed");
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }
    if (!CopyDataToArray(hdiInquiryInfo.revProduct, inquiryInfo->revProduct, sizeof(inquiryInfo->revProduct))) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "copy revProduct failed");
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }
    inquiryInfo->data->transferredLength = static_cast<uint32_t>(hdiResponse.transferredLength);

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_ReadCapacity10(ScsiPeripheral_Device *dev, ScsiPeripheral_ReadCapacityRequest *request,
    ScsiPeripheral_CapacityInfo *capacityInfo, ScsiPeripheral_Response *response)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || capacityInfo == nullptr || response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    auto hdiRequest = reinterpret_cast<OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralReadCapacityRequest *>(request);
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralCapacityInfo hdiCapacityInfo;
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));

    int32_t ret = TransToDdkErrCode(g_ddk->ReadCapacity10(dev->impl, *hdiRequest, hdiCapacityInfo,
        hdiResponse));
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "readcapacity10 failed");
        return ret;
    }

    capacityInfo->lbAddress = hdiCapacityInfo.lbAddress;
    capacityInfo->lbLength = hdiCapacityInfo.lbLength;

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_RequestSense(ScsiPeripheral_Device *dev, ScsiPeripheral_RequestSenseRequest *request,
    ScsiPeripheral_Response *response)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    auto hdiRequest = reinterpret_cast<OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralRequestSenseRequest *>(request);
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));
    int32_t ret = TransToDdkErrCode(g_ddk->RequestSense(dev->impl, *hdiRequest, hdiResponse));
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "request sense failed");
        return ret;
    }

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_Read10(ScsiPeripheral_Device *dev, ScsiPeripheral_IORequest *request,
    ScsiPeripheral_Response *response)
{
    EDM_LOGD(MODULE_SCSIPERIPHERAL_DDK, "Read10 start");
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || request->data ==nullptr || response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralIORequest hdiIORequest;
    ToHdi(request, hdiIORequest);
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));

    int32_t ret = TransToDdkErrCode(g_ddk->Read10(dev->impl, hdiIORequest, hdiResponse));
    if (ret !=  SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "read10 failed");
        return ret;
    }

    request->data->transferredLength = static_cast<uint32_t>(hdiResponse.transferredLength);

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_Write10(ScsiPeripheral_Device *dev, ScsiPeripheral_IORequest *request,
    ScsiPeripheral_Response *response)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || request->data ==nullptr || response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralIORequest hdiIORequest;
    ToHdi(request, hdiIORequest);
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));

    int32_t ret = TransToDdkErrCode(g_ddk->Write10(dev->impl, hdiIORequest, hdiResponse));
    if (ret !=  SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "write10 failed");
        return ret;
    }

    request->data->transferredLength = static_cast<uint32_t>(hdiResponse.transferredLength);

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_Verify10(ScsiPeripheral_Device *dev, ScsiPeripheral_VerifyRequest *request,
    ScsiPeripheral_Response *response)
{
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralVerifyRequest hdiVerifyRequest;
    hdiVerifyRequest.lbAddress = request->lbAddress;
    hdiVerifyRequest.verificationLength = request->verificationLength;
    hdiVerifyRequest.control = request->control;
    hdiVerifyRequest.byte1 = request->byte1;
    hdiVerifyRequest.byte6 = request->byte6;
    hdiVerifyRequest.timeout = request->timeout;
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));

    int32_t ret = TransToDdkErrCode(g_ddk->Verify10(dev->impl, hdiVerifyRequest, hdiResponse));
    if (ret != SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "verify10 failed");
        return ret;
    }

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_SendRequestByCdb(ScsiPeripheral_Device *dev, ScsiPeripheral_Request *request,
    ScsiPeripheral_Response *response)
{
    EDM_LOGD(MODULE_SCSIPERIPHERAL_DDK, "SendRequestByCDB start");
    if (g_ddk == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "invalid obj");
        return SCSIPERIPHERAL_DDK_INIT_ERROR;
    }
    if (dev == nullptr || request == nullptr || request->data ==nullptr || response == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }
    if (request->cdbLength == 0) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "cdb length is 0");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralRequest hdiRequest;
    hdiRequest.commandDescriptorBlock.assign(request->commandDescriptorBlock,
        request->commandDescriptorBlock + request->cdbLength);
    hdiRequest.dataTransferDirection = request->dataTransferDirection;
    hdiRequest.memMapSize = request->data->size;
    hdiRequest.timeout = request->timeout;
    OHOS::HDI::Usb::ScsiDdk::V1_0::ScsiPeripheralResponse hdiResponse;
    hdiResponse.senseData.resize(sizeof(response->senseData));

    int32_t ret = TransToDdkErrCode(g_ddk->SendRequestByCDB(dev->impl, hdiRequest, hdiResponse));
    if (ret !=  SCSIPERIPHERAL_DDK_SUCCESS) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "send request by cdb failed");
        return ret;
    }

    request->data->transferredLength = static_cast<uint32_t>(hdiResponse.transferredLength);

    return CopyResponse(hdiResponse, response);
}

int32_t OH_ScsiPeripheral_CreateDeviceMemMap(ScsiPeripheral_Device *dev, size_t size,
    ScsiPeripheral_DeviceMemMap **devMmap)
{
    if (dev == nullptr || devMmap == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }
    if (dev->memMapFd < 0) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "memMapFd is invalid");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    ftruncate(dev->memMapFd, size);
    auto buffer = static_cast<uint8_t *>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev->memMapFd, 0));
    if (buffer == MAP_FAILED) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "mmap failed, errno=%{public}d", errno);
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }

    ScsiPeripheral_DeviceMemMap *memMap = new ScsiPeripheral_DeviceMemMap({buffer, size, 0, size, 0});
    if (memMap == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "alloc dev mem failed, errno=%{public}d", errno);
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }

    *devMmap = memMap;
    return SCSIPERIPHERAL_DDK_SUCCESS;
}

int32_t OH_ScsiPeripheral_DestroyDeviceMemMap(ScsiPeripheral_DeviceMemMap *devMmap)
{
    if (devMmap == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "devMmap is nullptr");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    if (munmap(devMmap->address, devMmap->size) != 0) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "munmap failed, errno=%{public}d", errno);
        return SCSIPERIPHERAL_DDK_MEMORY_ERROR;
    }
    delete devMmap;
    devMmap = nullptr;
    return SCSIPERIPHERAL_DDK_SUCCESS;
}

int32_t OH_ScsiPeripheral_ParseBasicSenseInfo(uint8_t *senseData, uint8_t senseDataLen,
    ScsiPeripheral_BasicSenseInfo *senseInfo)
{
    if (senseData == nullptr || senseInfo == nullptr) {
        EDM_LOGE(MODULE_SCSIPERIPHERAL_DDK, "param is null");
        return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
    }

    senseInfo->responseCode = senseData[0] & MAST_RESPONSE_CODE;

    if (senseInfo->responseCode == RESPONSE_CODE_70H || senseInfo->responseCode == RESPONSE_CODE_71H) {
        return ParseFixedFormatSense(senseData, senseDataLen, senseInfo);
    } else if (senseInfo->responseCode == RESPONSE_CODE_72H || senseInfo->responseCode == RESPONSE_CODE_73H) {
        return ParseDescriptorFormatSense(senseData, senseDataLen, senseInfo);
    }

    return SCSIPERIPHERAL_DDK_INVALID_PARAMETER;
}
