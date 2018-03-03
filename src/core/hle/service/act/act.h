// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/common_types.h"

namespace Service {

class Interface;

namespace ACT {

enum class BlkId : u32 {
    PersistentID = 0x5,
    TransferableIDBase = 0x6,
    MiiData = 0x7,
    NNID = 0x8,
    Birthday = 0xA,
    CountryName = 0xB,
    PrincipalID = 0xC,
    Unknown = 0xE,
    Unknown2 = 0xF,
    InfoStruct = 0x11,
    Unknown3 = 0x12,
    Gender = 0x13,
    Unknown4 = 0x14,
    NNID2 = 0x15,
    Unknown5 = 0x16,
    Unknown6 = 0x17,
    UTCOffset = 0x19,
    Unknown7 = 0x1A,
    U16MiiName = 0x1B,
    NNID3 = 0x1C,
    Unknown8 = 0x1D,
    TimeZoneLocation = 0x1E,
    Unknown9 = 0x1F,
    Unknown10 = 0x20,
    Unknown11 = 0x24,
    MiiImageURL = 0x25,
    PrincipleID = 0x26,
    Unknown12 = 0x27,
    Unknown13 = 0x28,
    Unknown14 = 0x2B,
    Age = 0x2C,
    Unknown15 = 0x2D,
    Unknown16 = 0x2E,
    CountryInfo = 0x2F
};

void Initialize(Service::Interface* self);
void GetErrorCode(Service::Interface* self);
void GetAccountDataBlock(Service::Interface* self);

/// Initializes all ACT services
void Init();

} // namespace ACT
} // namespace Service
