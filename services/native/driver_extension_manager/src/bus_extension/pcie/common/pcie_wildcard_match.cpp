/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "bus_extension/pcie/common/pcie_wildcard_match.h"

namespace OHOS {
namespace ExternalDeviceManager {

bool PcieWildcardMatch(const std::string& wildcardPattern, const std::string& candidateText)
{
    size_t patternIndex = 0;
    size_t textIndex = 0;
    size_t starIndex = std::string::npos;
    size_t matchIndex = 0;

    while (textIndex < candidateText.size()) {
        if (patternIndex < wildcardPattern.size() &&
            (wildcardPattern[patternIndex] == '?' || wildcardPattern[patternIndex] == candidateText[textIndex])) {
            ++patternIndex;
            ++textIndex;
        } else if (patternIndex < wildcardPattern.size() && wildcardPattern[patternIndex] == '*') {
            starIndex = patternIndex;
            matchIndex = textIndex;
            ++patternIndex;
        } else if (starIndex != std::string::npos) {
            patternIndex = starIndex + 1;
            ++matchIndex;
            textIndex = matchIndex;
        } else {
            return false;
        }
    }

    while (patternIndex < wildcardPattern.size() && wildcardPattern[patternIndex] == '*') {
        ++patternIndex;
    }
    return patternIndex == wildcardPattern.size();
}

} // namespace ExternalDeviceManager
} // namespace OHOS
