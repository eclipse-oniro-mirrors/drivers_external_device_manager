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

#include "ext_dev_api_metrics.h"
#include <vector>

#define TIME_1000 1000
#define ERROR_CODE_BOUNDARY 8

namespace OHOS {
namespace ExternalDeviceManager {
enum MetricsErrCode : int32_t {
    METRICS_UEC_OK = 0,
    METRICS_OHEC_COMMON_PERMISSION_NOT_ALLOWED = 201,
    METRICS_OHEC_COMMON_NORMAL_APP_NOT_ALLOWED = 202,
    METRICS_OHEC_COMMON_PARAM_ERROR = 401,
    METRICS_UEC_SERVICE_EXCEPTION = 22900001,
    METRICS_UEC_SERVICE_EXCEPTION_NEW = 26300001,
    METRICS_UEC_SERVICE_NOT_ALLOW_ACCESS = 26300002,
    METRICS_UEC_SERVICE_NOT_BOUND = 26300003,
    METRICS_OTHER_ERROR = 99999999,
};

std::vector<int32_t> g_errorVector = {
    METRICS_UEC_OK,
    METRICS_OHEC_COMMON_PERMISSION_NOT_ALLOWED,
    METRICS_OHEC_COMMON_NORMAL_APP_NOT_ALLOWED,
    METRICS_OHEC_COMMON_PARAM_ERROR,
    METRICS_UEC_SERVICE_EXCEPTION,
    METRICS_UEC_SERVICE_EXCEPTION_NEW,
    METRICS_UEC_SERVICE_NOT_ALLOW_ACCESS,
    METRICS_UEC_SERVICE_NOT_BOUND,
    METRICS_OTHER_ERROR
};

ExtDevApiMetrics::ExtDevApiMetrics(std::string name)
    : metricsName_(name)
{
    errorCode_ = 0;
    gettimeofday(&startTime_, nullptr);
}

ExtDevApiMetrics::~ExtDevApiMetrics()
{
#ifdef EDM_MANAGER_METRICS_ENABLE
    std::string boolMetricsName = metricsName_ + ".Boolean";
    std::string enumMetricsName = metricsName_ + ".Enum";
    std::string timeMetricsName = metricsName_ + ".Time";
    if (errorCode_ == METRICS_UEC_OK) {
        HISTOGRAM_BOOLEAN(boolMetricsName.c_str(), 1);
    } else {
        HISTOGRAM_BOOLEAN(boolMetricsName.c_str(), 0);
    }
    struct timeval endTime;
    gettimeofday(&endTime, nullptr);
    int32_t runTime = (int32_t)((endTime.tv_sec - startTime_.tv_sec) * TIME_1000 +
        (endTime.tv_usec - startTime_.tv_usec) / TIME_1000);
    HISTOGRAM_TIMES(timeMetricsName.c_str(), runTime);
    HISTOGRAM_ENUMERATION(enumMetricsName.c_str(), errorCode_, ERROR_CODE_BOUNDARY);
#endif
}

void ExtDevApiMetrics::SetErrorCode(int32_t error)
{
    for (int i = 0; i < ERROR_CODE_BOUNDARY; i++) {
        if (error == g_errorVector[i]) {
            errorCode_ = i;
            return;
        }
    }
    errorCode_ = ERROR_CODE_BOUNDARY;
}
}
}
