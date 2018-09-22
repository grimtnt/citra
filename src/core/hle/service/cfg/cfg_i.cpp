// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/cfg/cfg_i.h"

namespace Service::CFG {

CFG_I::CFG_I(std::shared_ptr<Module> cfg) : Module::Interface{std::move(cfg), "cfg:i", 23} {
    static const FunctionInfo functions[] = {
        // cfg common
        {0x00010082, &CFG_I::GetConfigInfoBlk2, "GetConfigInfoBlk2"},
        {0x00020000, &CFG_I::D<&CFG_I::SecureInfoGetRegion, 0x0002>, "SecureInfoGetRegion"},
        {0x00030040, &CFG_I::GenHashConsoleUnique, "GenHashConsoleUnique"},
        {0x00040000, &CFG_I::GetRegionCanadaUSA, "GetRegionCanadaUSA"},
        {0x00050000, &CFG_I::GetSystemModel, "GetSystemModel"},
        {0x00060000, &CFG_I::GetModelNintendo2DS, "GetModelNintendo2DS"},
        {0x00070040, nullptr, "WriteToFirstByteCfgSavegame"},
        {0x00080080, nullptr, "GoThroughTable"},
        {0x00090040, &CFG_I::GetCountryCodeString, "GetCountryCodeString"},
        {0x000A0040, &CFG_I::GetCountryCodeID, "GetCountryCodeID"},
        {0x000B0000, &CFG_I::IsFangateSupported, "IsFangateSupported"},
        // cfg:i
        {0x04010082, &CFG_I::D<&CFG_I::GetConfigInfoBlk8, 0x0401>, "GetConfigInfoBlk8"},
        {0x04020082, &CFG_I::D<&CFG_I::SetConfigInfoBlk4, 0x0402>, "SetConfigInfoBlk4"},
        {0x04030000, &CFG_I::D<&CFG_I::UpdateConfigNANDSavegame, 0x0403>,
         "UpdateConfigNANDSavegame"},
        {0x04040042, &CFG_I::D<&CFG_I::GetLocalFriendCodeSeedData, 0x0404>,
         "GetLocalFriendCodeSeedData"},
        {0x04050000, &CFG_I::D<&CFG_I::GetLocalFriendCodeSeed, 0x0405>, "GetLocalFriendCodeSeed"},
        {0x04060000, &CFG_I::D<&CFG_I::SecureInfoGetRegion, 0x0406>, "SecureInfoGetRegion"},
        {0x04070000, nullptr, "SecureInfoGetByte101"},
        {0x04080042, nullptr, "SecureInfoGetSerialNo"},
        {0x04090000, nullptr, "UpdateConfigBlk00040003"},
        {0x08010082, &CFG_I::D<&CFG_I::GetConfigInfoBlk8, 0x0801>, "GetConfigInfoBlk8"},
        {0x08020082, &CFG_I::D<&CFG_I::SetConfigInfoBlk4, 0x0802>, "SetConfigInfoBlk4"},
        {0x08030000, &CFG_I::D<&CFG_I::UpdateConfigNANDSavegame, 0x0803>,
         "UpdateConfigNANDSavegame"},
        {0x080400C2, &CFG_I::CreateConfigInfoBlk, "CreateConfigInfoBlk"},
        {0x08050000, &CFG_I::DeleteConfigNANDSavefile, "DeleteConfigNANDSavefile"},
        {0x08060000, &CFG_I::FormatConfig, "FormatConfig"},
        {0x08080000, nullptr, "UpdateConfigBlk1"},
        {0x08090000, nullptr, "UpdateConfigBlk2"},
        {0x080A0000, nullptr, "UpdateConfigBlk3"},
        {0x080B0082, &CFG_I::SetGetLocalFriendCodeSeedData, "SetGetLocalFriendCodeSeedData"},
        {0x080C0042, &CFG_I::SetLocalFriendCodeSeedSignature, "SetLocalFriendCodeSeedSignature"},
        {0x080D0000, &CFG_I::DeleteCreateNANDLocalFriendCodeSeed,
         "DeleteCreateNANDLocalFriendCodeSeed"},
        {0x080E0000, nullptr, "VerifySigLocalFriendCodeSeed"},
        {0x080F0042, &CFG_I::D<&CFG_I::GetLocalFriendCodeSeedData, 0x080F>,
         "GetLocalFriendCodeSeedData"},
        {0x08100000, &CFG_I::D<&CFG_I::GetLocalFriendCodeSeed, 0x0810>, "GetLocalFriendCodeSeed"},
        {0x08110084, nullptr, "SetSecureInfo"},
        {0x08120000, nullptr, "DeleteCreateNANDSecureInfo"},
        {0x08130000, nullptr, "VerifySigSecureInfo"},
        {0x08140042, nullptr, "SecureInfoGetData"},
        {0x08150042, nullptr, "SecureInfoGetSignature"},
        {0x08160000, &CFG_I::D<&CFG_I::SecureInfoGetRegion, 0x0816>, "SecureInfoGetRegion"},
        {0x08170000, nullptr, "SecureInfoGetByte101"},
        {0x08180042, nullptr, "SecureInfoGetSerialNo"},
    };
    RegisterHandlers(functions);
}

} // namespace Service::CFG
