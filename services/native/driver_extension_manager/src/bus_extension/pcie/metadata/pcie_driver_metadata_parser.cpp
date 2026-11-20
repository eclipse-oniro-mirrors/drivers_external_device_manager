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

#include "bus_extension/pcie/metadata/pcie_driver_metadata_parser.h"
#include "hilog_wrapper.h"
#include "unique_fd.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace OHOS {
namespace ExternalDeviceManager {

namespace {

constexpr size_t ELF32_HEADER_SIZE = 52;
constexpr size_t ELF64_HEADER_SIZE = 64;
constexpr size_t ELF32_SECTION_HEADER_SIZE = 40;
constexpr size_t ELF64_SECTION_HEADER_SIZE = 64;
constexpr size_t ELF_CLASS_INDEX = 4;
constexpr size_t ELF32_SECTION_HEADER_OFFSET_OFFSET = 32;
constexpr size_t ELF32_SECTION_HEADER_ENTRY_SIZE_OFFSET = 46;
constexpr size_t ELF32_SECTION_HEADER_COUNT_OFFSET = 48;
constexpr size_t ELF32_SECTION_NAME_TABLE_INDEX_OFFSET = 50;
constexpr size_t ELF32_SECTION_OFFSET_FIELD_OFFSET = 16;
constexpr size_t ELF32_SECTION_SIZE_FIELD_OFFSET = 20;
constexpr size_t ELF64_SECTION_HEADER_OFFSET_OFFSET = 40;
constexpr size_t ELF64_SECTION_HEADER_ENTRY_SIZE_OFFSET = 58;
constexpr size_t ELF64_SECTION_HEADER_COUNT_OFFSET = 60;
constexpr size_t ELF64_SECTION_NAME_TABLE_INDEX_OFFSET = 62;
constexpr size_t ELF64_SECTION_OFFSET_FIELD_OFFSET = 24;
constexpr size_t ELF64_SECTION_SIZE_FIELD_OFFSET = 32;
constexpr size_t ELF_MAGIC_FIRST_INDEX = 0;
constexpr size_t ELF_MAGIC_E_INDEX = 1;
constexpr size_t ELF_EI_DATA_INDEX = 5;
constexpr size_t ELF_EI_VERSION_INDEX = 6;
constexpr size_t ELF_MAGIC_L_INDEX = 2;
constexpr size_t ELF_MAGIC_INDEX = 3;
constexpr uint8_t ELF_CLASS_32 = 1;
constexpr uint8_t ELF_CLASS_64 = 2;
constexpr uint8_t ELF_DATA_LITTLE_ENDIAN = 1;
constexpr uint8_t ELF_DATA_BIG_ENDIAN = 2;
constexpr uint8_t ELF_CURRENT_VERSION = 1;
constexpr uint8_t ELF_BYTE_BIT_COUNT = 8;
constexpr char MODINFO_SECTION_NAME[] = ".modinfo";
constexpr size_t MAX_SECTION_NAME_TABLE_SIZE = 1024 * 1024;
constexpr size_t MAX_MODINFO_SECTION_SIZE = 4 * 1024 * 1024;

struct ElfSectionLayout {
    uint64_t sectionHeaderOffset = 0;
    size_t sectionHeaderSize = 0;
    size_t sectionHeaderEntrySize = 0;
    size_t sectionHeaderCount = 0;
    size_t sectionNameTableIndex = 0;
    size_t sectionOffsetFieldOffset = 0;
    size_t sectionSizeFieldOffset = 0;
    bool is64Bit = false;
};

struct ElfDataView {
    int fileFd = -1;
    const std::string *content = nullptr;
    uint64_t size = 0;
    bool needSwap = false;
    bool readFailed = false;

    bool Read(uint64_t offset, size_t length, std::string &buffer)
    {
        if (offset > size || length > size - offset) {
            return false;
        }
        if (content != nullptr) {
            buffer.assign(*content, static_cast<size_t>(offset), length);
            return true;
        }
        buffer.resize(length);
        size_t bytesRead = 0;
        while (bytesRead < length) {
            ssize_t result =
                pread(fileFd, &buffer[bytesRead], length - bytesRead, static_cast<off_t>(offset + bytesRead));
            if (result < 0 && errno == EINTR) {
                continue;
            }
            if (result <= 0) {
                readFailed = true;
                buffer.clear();
                return false;
            }
            bytesRead += static_cast<size_t>(result);
        }
        return true;
    }
};

std::string Trim(const std::string &input)
{
    size_t beginIndex = 0;
    size_t endIndex = input.size();
    while (beginIndex < endIndex && std::isspace(static_cast<unsigned char>(input[beginIndex]))) {
        ++beginIndex;
    }
    while (endIndex > beginIndex && std::isspace(static_cast<unsigned char>(input[endIndex - 1]))) {
        --endIndex;
    }
    return input.substr(beginIndex, endIndex - beginIndex);
}

std::vector<std::string> SplitCsv(const std::string &commaSeparatedText)
{
    std::vector<std::string> entries;
    std::istringstream inputStream(commaSeparatedText);
    std::string entry;
    while (std::getline(inputStream, entry, ',')) {
        if (!entry.empty()) {
            entries.push_back(entry);
        }
    }
    return entries;
}

/* ── ELF .modinfo section extraction ─────────────────────────────────
 *
 * Locates the .modinfo section via ELF section headers instead of
 * scanning the entire file.  Handles both 32-bit and 64-bit ELF,
 * little-endian and big-endian.  Returns the raw section content.
 */
template <typename UnsignedInteger>
UnsignedInteger ReadElfUnsignedInteger(const uint8_t *data, bool needSwap)
{
    UnsignedInteger value = 0;
    for (size_t byteIndex = 0; byteIndex < sizeof(UnsignedInteger); ++byteIndex) {
        size_t dataIndex = needSwap ? byteIndex : sizeof(UnsignedInteger) - byteIndex - 1;
        value = static_cast<UnsignedInteger>((value << ELF_BYTE_BIT_COUNT) | data[dataIndex]);
    }
    return value;
}

uint16_t ReadElfUint16(const uint8_t *data, bool needSwap)
{
    return ReadElfUnsignedInteger<uint16_t>(data, needSwap);
}

uint32_t ReadElfUint32(const uint8_t *data, bool needSwap)
{
    return ReadElfUnsignedInteger<uint32_t>(data, needSwap);
}

uint64_t ReadElfUint64(const uint8_t *data, bool needSwap)
{
    return ReadElfUnsignedInteger<uint64_t>(data, needSwap);
}

// Validate the ELF header length, magic, class and byte order without checking the entire file.
bool ValidateElfHeader(const std::string &elfContent, uint8_t &elfClass, bool &needSwap)
{
    if (elfContent.size() < ELF32_HEADER_SIZE) {
        return false;
    }
    const auto *elfData = reinterpret_cast<const uint8_t *>(elfContent.data());
    if (elfData[ELF_MAGIC_FIRST_INDEX] != 0x7f || elfData[ELF_MAGIC_E_INDEX] != 'E' ||
        elfData[ELF_MAGIC_L_INDEX] != 'L' || elfData[ELF_MAGIC_INDEX] != 'F') {
        return false;
    }
    elfClass = elfData[ELF_CLASS_INDEX];
    if (elfClass != ELF_CLASS_32 && elfClass != ELF_CLASS_64) {
        return false;
    }
    if (elfData[ELF_EI_DATA_INDEX] != ELF_DATA_LITTLE_ENDIAN && elfData[ELF_EI_DATA_INDEX] != ELF_DATA_BIG_ENDIAN) {
        return false;
    }
    if (elfData[ELF_EI_VERSION_INDEX] != ELF_CURRENT_VERSION) {
        return false;
    }
    needSwap = elfData[ELF_EI_DATA_INDEX] == ELF_DATA_BIG_ENDIAN;
    return true;
}

// Read the ELF section table layout and validate its bounds before locating .modinfo.
bool ReadElfSectionLayout(
    const uint8_t *elfData, uint64_t elfSize, uint8_t elfClass, bool needSwap, ElfSectionLayout &sectionLayout)
{
    size_t sectionHeaderEntrySize;
    sectionLayout.is64Bit = elfClass == ELF_CLASS_64;
    if (elfClass == ELF_CLASS_64) {
        if (elfSize < ELF64_HEADER_SIZE) {
            return false;
        }
        sectionLayout.sectionHeaderOffset = ReadElfUint64(elfData + ELF64_SECTION_HEADER_OFFSET_OFFSET, needSwap);
        sectionHeaderEntrySize = ReadElfUint16(elfData + ELF64_SECTION_HEADER_ENTRY_SIZE_OFFSET, needSwap);
        sectionLayout.sectionHeaderCount = ReadElfUint16(elfData + ELF64_SECTION_HEADER_COUNT_OFFSET, needSwap);
        sectionLayout.sectionNameTableIndex = ReadElfUint16(elfData + ELF64_SECTION_NAME_TABLE_INDEX_OFFSET, needSwap);
        sectionLayout.sectionHeaderSize = ELF64_SECTION_HEADER_SIZE;
        sectionLayout.sectionOffsetFieldOffset = ELF64_SECTION_OFFSET_FIELD_OFFSET;
        sectionLayout.sectionSizeFieldOffset = ELF64_SECTION_SIZE_FIELD_OFFSET;
    } else {
        if (elfSize < ELF32_HEADER_SIZE) {
            return false;
        }
        sectionLayout.sectionHeaderOffset = ReadElfUint32(elfData + ELF32_SECTION_HEADER_OFFSET_OFFSET, needSwap);
        sectionHeaderEntrySize = ReadElfUint16(elfData + ELF32_SECTION_HEADER_ENTRY_SIZE_OFFSET, needSwap);
        sectionLayout.sectionHeaderCount = ReadElfUint16(elfData + ELF32_SECTION_HEADER_COUNT_OFFSET, needSwap);
        sectionLayout.sectionNameTableIndex = ReadElfUint16(elfData + ELF32_SECTION_NAME_TABLE_INDEX_OFFSET, needSwap);
        sectionLayout.sectionHeaderSize = ELF32_SECTION_HEADER_SIZE;
        sectionLayout.sectionOffsetFieldOffset = ELF32_SECTION_OFFSET_FIELD_OFFSET;
        sectionLayout.sectionSizeFieldOffset = ELF32_SECTION_SIZE_FIELD_OFFSET;
    }
    sectionLayout.sectionHeaderEntrySize = sectionHeaderEntrySize;
    if (sectionHeaderEntrySize == 0 || sectionLayout.sectionHeaderCount == 0 ||
        sectionLayout.sectionNameTableIndex >= sectionLayout.sectionHeaderCount ||
        sectionHeaderEntrySize < sectionLayout.sectionHeaderSize) {
        return false;
    }
    if (sectionLayout.sectionHeaderOffset >= elfSize) {
        return false;
    }
    return sectionLayout.sectionHeaderCount <= (elfSize - sectionLayout.sectionHeaderOffset) / sectionHeaderEntrySize;
}

// Read a section offset or size using the ELF class and byte order without reading section contents.
uint64_t ReadElfSectionValue(const uint8_t *data, const ElfSectionLayout &sectionLayout, bool needSwap)
{
    if (sectionLayout.is64Bit) {
        return ReadElfUint64(data, needSwap);
    }
    return ReadElfUint32(data, needSwap);
}

// Read the section name table using its header offset and size to identify .modinfo later.
bool ReadSectionNameTable(ElfDataView &elfView, const ElfSectionLayout &sectionLayout, std::string &stringTable)
{
    uint64_t stringTableHeaderOffset =
        sectionLayout.sectionHeaderOffset + sectionLayout.sectionNameTableIndex * sectionLayout.sectionHeaderEntrySize;
    std::string sectionHeader;
    if (!elfView.Read(stringTableHeaderOffset, sectionLayout.sectionHeaderSize, sectionHeader)) {
        return false;
    }
    const auto *headerData = reinterpret_cast<const uint8_t *>(sectionHeader.data());
    uint64_t sectionOffset =
        ReadElfSectionValue(headerData + sectionLayout.sectionOffsetFieldOffset, sectionLayout, elfView.needSwap);
    uint64_t sectionSize =
        ReadElfSectionValue(headerData + sectionLayout.sectionSizeFieldOffset, sectionLayout, elfView.needSwap);
    return sectionSize > 0 && sectionSize <= MAX_SECTION_NAME_TABLE_SIZE &&
        elfView.Read(sectionOffset, static_cast<size_t>(sectionSize), stringTable);
}

bool IsSectionNameTerminated(const char *sectionName, size_t maxNameLength)
{
    return std::memchr(sectionName, '\0', maxNameLength) != nullptr;
}

// Find .modinfo in the ELF section table and copy its raw contents to modinfoContent without parsing fields.
bool FindModinfoSection(ElfDataView &elfView, const ElfSectionLayout &sectionLayout, const std::string &stringTable,
    std::string &modinfoContent)
{
    std::string sectionHeader;
    for (size_t sectionIndex = 0; sectionIndex < sectionLayout.sectionHeaderCount; ++sectionIndex) {
        uint64_t headerOffset = sectionLayout.sectionHeaderOffset + sectionIndex * sectionLayout.sectionHeaderEntrySize;
        if (!elfView.Read(headerOffset, sectionLayout.sectionHeaderSize, sectionHeader)) {
            return false;
        }
        const auto *headerData = reinterpret_cast<const uint8_t *>(sectionHeader.data());
        uint32_t sectionNameIndex = ReadElfUint32(headerData, elfView.needSwap);
        if (sectionNameIndex >= stringTable.size()) {
            continue;
        }
        uint64_t sectionOffset =
            ReadElfSectionValue(headerData + sectionLayout.sectionOffsetFieldOffset, sectionLayout, elfView.needSwap);
        uint64_t sectionSize =
            ReadElfSectionValue(headerData + sectionLayout.sectionSizeFieldOffset, sectionLayout, elfView.needSwap);
        if (sectionSize == 0 || sectionOffset >= elfView.size || sectionSize > elfView.size - sectionOffset) {
            continue;
        }
        const char *sectionName = stringTable.data() + sectionNameIndex;
        size_t maxNameLength = stringTable.size() - sectionNameIndex;
        if (!IsSectionNameTerminated(sectionName, maxNameLength)) {
            continue;
        }
        if (std::strcmp(sectionName, MODINFO_SECTION_NAME) == 0) {
            return sectionSize <= MAX_MODINFO_SECTION_SIZE &&
                elfView.Read(sectionOffset, static_cast<size_t>(sectionSize), modinfoContent);
        }
    }
    return false;
}

// Read the ELF header, section headers, name table and .modinfo by offset, skipping other section contents.
bool ExtractModinfoSection(ElfDataView &elfView, std::string &modinfoContent)
{
    std::string elfHeader;
    if (!elfView.Read(0, static_cast<size_t>(std::min<uint64_t>(elfView.size, ELF64_HEADER_SIZE)), elfHeader)) {
        return false;
    }
    uint8_t elfClass;
    if (!ValidateElfHeader(elfHeader, elfClass, elfView.needSwap) ||
        (elfClass == ELF_CLASS_64 && elfHeader.size() < ELF64_HEADER_SIZE)) {
        return false;
    }
    const auto *elfData = reinterpret_cast<const uint8_t *>(elfHeader.data());
    ElfSectionLayout sectionLayout;
    if (!ReadElfSectionLayout(elfData, elfView.size, elfClass, elfView.needSwap, sectionLayout)) {
        return false;
    }
    std::string stringTable;
    if (!ReadSectionNameTable(elfView, sectionLayout, stringTable)) {
        return false;
    }
    return FindModinfoSection(elfView, sectionLayout, stringTable, modinfoContent);
}

} // namespace

PcieDriverMetadataParser::PcieDriverMetadataParser(FileReader fileReader)
    : fileReader_(std::move(fileReader)), useInjectedFileReader_(true)
{
}

PcieDriverFailureReason PcieDriverMetadataParser::ReadModinfo(
    const std::string &driverPath, std::string &modinfoContent)
{
    ElfDataView elfView;
    std::string elfContent;
    UniqueFd fileFd;
    if (useInjectedFileReader_) {
        // Calls the file reader injected at construction, primarily for tests to supply ELF content.
        if (!fileReader_ || !fileReader_(driverPath, elfContent)) {
            return PcieDriverFailureReason::FILE_NOT_READABLE;
        }
        elfView.content = &elfContent;
        elfView.size = elfContent.size();
    } else {
        fileFd = UniqueFd(open(driverPath.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK));
        struct stat fileStatus;
        if (fileFd.Get() < 0 || fstat(fileFd.Get(), &fileStatus) != 0 || !S_ISREG(fileStatus.st_mode) ||
            fileStatus.st_size < 0) {
            return PcieDriverFailureReason::FILE_NOT_READABLE;
        }
        elfView.fileFd = fileFd.Get();
        elfView.size = static_cast<uint64_t>(fileStatus.st_size);
    }
    if (!ExtractModinfoSection(elfView, modinfoContent)) {
        return elfView.readFailed ? PcieDriverFailureReason::FILE_NOT_READABLE :
                                    PcieDriverFailureReason::UNSUPPORTED_FORMAT;
    }
    return PcieDriverFailureReason::NONE;
}

bool PcieDriverMetadataParser::SplitModinfo(const std::string &modinfoContent, PcieDriverMetadata &metadata)
{
    bool hasEntries = false;
    std::string currentEntry;
    for (char character : modinfoContent) {
        if (character == '\0' || character == '\n' || character == '\r') {
            if (!currentEntry.empty()) {
                TryParseLine(currentEntry, metadata);
                hasEntries = true;
                currentEntry.clear();
            }
        } else if (std::isprint(static_cast<unsigned char>(character)) || character == '\t') {
            currentEntry.push_back(character);
        } else {
            /* non-printable byte (e.g. ARM64 instructions between ELF sections):
               treat as splitter to avoid corrupting the next modinfo entry */
            if (!currentEntry.empty()) {
                TryParseLine(currentEntry, metadata);
                hasEntries = true;
                currentEntry.clear();
            }
        }
    }
    if (!currentEntry.empty()) {
        TryParseLine(currentEntry, metadata);
        hasEntries = true;
    }
    return hasEntries;
}

PcieDriverMetadataResult PcieDriverMetadataParser::Parse(const PcieDriverRecord &driver)
{
    PcieDriverMetadata metadata;

    if (!driver.isValid) {
        EDM_LOGE(MODULE_SERVICE, "Parse: INVALID_RECORD");
        return BuildFailure(driver, PcieDriverFailureReason::INVALID_RECORD, metadata);
    }

    std::string modinfoContent;
    auto readFailure = ReadModinfo(driver.driverPath, modinfoContent);
    if (readFailure != PcieDriverFailureReason::NONE) {
        EDM_LOGE(MODULE_SERVICE, "Parse: metadata read failed reason=%{public}d", static_cast<int32_t>(readFailure));
        return BuildFailure(driver, readFailure, metadata);
    }

    if (!SplitModinfo(modinfoContent, metadata)) {
        EDM_LOGE(MODULE_SERVICE, "Parse: MODINFO_EMPTY");
        return BuildFailure(driver, PcieDriverFailureReason::UNSUPPORTED_FORMAT, metadata);
    }

    // aliases may come from modprobe.d — not fatal if empty here
    if (metadata.kabi.empty() && metadata.vermagic.empty()) {
        EDM_LOGE(MODULE_SERVICE, "Parse: NO_DRIVER_KABI_OR_VERMAGIC aliases=%{public}zu", metadata.aliases.size());
        return BuildFailure(driver, PcieDriverFailureReason::NO_DRIVER_KABI_METADATA, metadata);
    }

    EDM_LOGI(MODULE_SERVICE, "Parse: OK aliases=%{public}zu vermagic=%{public}s",
        metadata.aliases.size(), metadata.vermagic.c_str());
    return BuildSuccess(driver, metadata);
}

bool PcieDriverMetadataParser::TryParseLine(const std::string &modinfoEntry, PcieDriverMetadata &metadata)
{
    auto equalSignPosition = modinfoEntry.find('=');
    if (equalSignPosition == std::string::npos)
        return false;

    std::string key = Trim(modinfoEntry.substr(0, equalSignPosition));
    std::string val = Trim(modinfoEntry.substr(equalSignPosition + 1));

    if (key == "alias") {
        metadata.aliases.push_back(val);
    } else if (key == "kabi") {
        if (metadata.kabi.empty())
            metadata.kabi = val;
    } else if (key == "vermagic") {
        if (metadata.vermagic.empty())
            metadata.vermagic = val;
    } else if (key == "module_name" || key == "name") {
        metadata.moduleName = val;
    } else if (key == "license") {
        metadata.license = val;
    } else if (key == "depends") {
        auto deps = SplitCsv(val);
        metadata.depends.insert(metadata.depends.end(), deps.begin(), deps.end());
    }
    return true;
}

PcieDriverMetadataResult PcieDriverMetadataParser::BuildSuccess(
    const PcieDriverRecord &driver, const PcieDriverMetadata &metadata)
{
    PcieDriverMetadataResult metadataResult;
    metadataResult.metadata = metadata;
    metadataResult.metadata.driverPath = driver.driverPath;
    // Mark parsing as successful, metadata as usable and the failure reason as NONE.
    metadataResult.metadata.isParseable = true;
    metadataResult.metadata.failReason = PcieDriverFailureReason::NONE;
    metadataResult.parseSucceeded = true;
    return metadataResult;
}

PcieDriverMetadataResult PcieDriverMetadataParser::BuildFailure(
    const PcieDriverRecord &driver, PcieDriverFailureReason reason, const PcieDriverMetadata &partialMetadata)
{
    PcieDriverMetadataResult metadataResult;
    metadataResult.metadata = partialMetadata;
    metadataResult.metadata.driverPath = driver.driverPath;
    metadataResult.metadata.isParseable = false;
    metadataResult.metadata.failReason = reason;
    metadataResult.parseSucceeded = false;
    metadataResult.failReason = reason;
    return metadataResult;
}

} // namespace ExternalDeviceManager
} // namespace OHOS
