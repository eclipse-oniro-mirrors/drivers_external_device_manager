/*
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

#ifndef OHOS_EDM_PCIE_DRIVER_METADATA_PARSER_H
#define OHOS_EDM_PCIE_DRIVER_METADATA_PARSER_H

#include "bus_extension/pcie/common/pcie_driver_types.h"
#include <functional>
#include <string>

namespace OHOS {
namespace ExternalDeviceManager {

class PcieDriverMetadataParser {
public:
    // Tests supply ELF bytes through this callback instead of reading the driver file.
    using FileReader = std::function<bool(const std::string& driverPath,
                                           std::string& content)>;

    // Production reads only the required ELF sections from the actual driver file.
    PcieDriverMetadataParser() = default;
    // Stores a test file reader; this constructor selects the injected reading path.
    explicit PcieDriverMetadataParser(FileReader fileReader);
    ~PcieDriverMetadataParser() = default;

    PcieDriverMetadataResult Parse(const PcieDriverRecord& driver);

private:
    PcieDriverFailureReason ReadModinfo(const std::string &driverPath, std::string &modinfoContent);
    bool SplitModinfo(const std::string &modinfoContent, PcieDriverMetadata &metadata);
    bool TryParseLine(const std::string& modinfoEntry, PcieDriverMetadata& metadata);
    PcieDriverMetadataResult BuildSuccess(const PcieDriverRecord& driver,
                                       const PcieDriverMetadata& metadata);
    PcieDriverMetadataResult BuildFailure(const PcieDriverRecord& driver,
                                       PcieDriverFailureReason reason,
                                       const PcieDriverMetadata& partialMetadata);

    // Retains the injected reader for subsequent Parse calls.
    FileReader fileReader_;
    // Distinguishes default file access from an explicitly supplied reader, including an empty one.
    bool useInjectedFileReader_ = false;
};

} // namespace ExternalDeviceManager
} // namespace OHOS
#endif
