// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/common_funcs.h"
#include "common/common_types.h"
#include "common/swap.h"

namespace Service {

class Interface;

namespace NFC {

enum class TagState : u8 {
    NotInitialized = 0,
    NotScanning = 1,
    Scanning = 2,
    TagInRange = 3,
    TagOutOfRange = 4,
    TagDataLoaded = 5,
};

enum class CommunicationStatus : u8 {
    AttemptInitialize = 1,
    NfcInitialized = 2,
};

struct TagInfo {
    u16 len;
    u8 unknown1;
    u8 unknown2;
    u8 id_data[0x28];
};
static_assert(sizeof(TagInfo) == 0x2C, "TagInfo structure has incorrect size");

struct AmiiboDate {
    u16 year;
    u8 month;
    u8 day;
};
static_assert(sizeof(AmiiboDate) == 0x4, "AmiiboDate structure has incorrect size");

struct AmiiboConfig {
    AmiiboDate last_write_date;
    u16 write_count;
    INSERT_PADDING_BYTES(6);
    u8 unknown1;
    u8 unknown2;
    u16 appdata_size;
    INSERT_PADDING_BYTES(0x30);
};
static_assert(sizeof(AmiiboConfig) == 0x40, "AmiiboConfig structure has incorrect size");

struct Mii {
    u32 mii_id;
    u8 system_id[8];
    u32 creation_date;
    u8 mac_address[6];
    INSERT_PADDING_BYTES(2);
    u16 birthday;
    u16 mii_name[10];
    u16 width_height;
    u8 face;
    u8 makeup;
    u8 hair_style;
    u8 hair_color;
    INSERT_PADDING_BYTES(4);
    u8 eyebrow_style;
    u8 eyebrow_scale;
    u16 eyebrow_pos;
    INSERT_PADDING_BYTES(4);
    u8 allow_copy;
    u8 unknown3[7];
    u16 author_name[10];
    INSERT_PADDING_BYTES(4);
};
static_assert(sizeof(Mii) == 0x60, "Mii structure has incorrect size");

struct AmiiboSettings {
    Mii owner_mii;
    u16_be nickname[11];
    u8 unknown1;
    u8 unknown2;
    AmiiboDate setup_date;
    INSERT_PADDING_BYTES(0x2C);
};
static_assert(sizeof(AmiiboSettings) == 0xA8, "AmiiboSettings structure has incorrect size");

/**
 * NFC::Initialize service function
 *  Inputs:
 *      0 : Header code [0x00010040]
 *      1 : (u8) unknown parameter. Can be either value 0x1 or 0x2
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void Initialize(Interface* self);

/**
 * NFC::Shutdown service function
 *  Inputs:
 *      0 : Header code [0x00020040]
 *      1 : (u8) unknown parameter
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void Shutdown(Interface* self);

/**
 * NFC::StartCommunication service function
 *  Inputs:
 *      0 : Header code [0x00030000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void StartCommunication(Interface* self);

/**
 * NFC::StopCommunication service function
 *  Inputs:
 *      0 : Header code [0x00040000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void StopCommunication(Interface* self);

/**
 * NFC::StartTagScanning service function
 *  Inputs:
 *      0 : Header code [0x00050040]
 *      1 : (u16) unknown. This is normally 0x0
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void StartTagScanning(Interface* self);

/**
 * NFC::StopTagScanning service function
 *  Inputs:
 *      0 : Header code [0x00060000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void StopTagScanning(Interface* self);

/**
 * NFC::LoadAmiiboData service function
 *  Inputs:
 *      0 : Header code [0x00070000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void LoadAmiiboData(Interface* self);

/**
 * NFC::ResetTagScanState service function
 *  Inputs:
 *      0 : Header code [0x00080000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void ResetTagScanState(Interface* self);

/**
 * NFC::GetTagInRangeEvent service function
 *  Inputs:
 *      0 : Header code [0x000B0000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : Copy handle descriptor
 *      3 : Event Handle
 */
void GetTagInRangeEvent(Interface* self);

/**
 * NFC::GetTagOutOfRangeEvent service function
 *  Inputs:
 *      0 : Header code [0x000C0000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : Copy handle descriptor
 *      3 : Event Handle
 */
void GetTagOutOfRangeEvent(Interface* self);

/**
 * NFC::GetTagState service function
 *  Inputs:
 *      0 : Header code [0x000D0000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : (u8) Tag state
 */
void GetTagState(Interface* self);

/**
 * NFC::CommunicationGetStatus service function
 *  Inputs:
 *      0 : Header code [0x000F0000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : (u8) Communication state
 */
void CommunicationGetStatus(Interface* self);

/**
 * NFC::GetTagInfo service function
 *  Inputs:
 *      0 : Header code [0x00110000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2-12 : TagInfo struct
 */
void GetTagInfo(Interface* self);

/**
 * NFC::GetAmiiboSettings service function
 *  Inputs:
 *      0 : Header code [0x00170000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2-43 : AmiiboSettings struct
 */
void GetAmiiboSettings(Interface* self);

/**
 * NFC::GetAmiiboConfig service function
 *  Inputs:
 *      0 : Header code [0x00180000]
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2-17 : AmiiboConfig struct
 */
void GetAmiiboConfig(Interface* self);

/// Initialize all NFC services.
void Init();

/// Shutdown all NFC services.
void Shutdown();

} // namespace NFC
} // namespace Service
