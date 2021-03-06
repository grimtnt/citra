// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <tuple>
#include "core/hle/service/service.h"

namespace Service::PS {

struct LocalFriendCodeSeed {
    u8 signature[0x100];
    u8 unknown[0x8];
    u64_le seed;
};

class PS_PS final : public ServiceFramework<PS_PS> {
public:
    PS_PS();
    ~PS_PS() = default;

private:
    void SignRsaSha256(Kernel::HLERequestContext& ctx);
    void VerifyRsaSha256(Kernel::HLERequestContext& ctx);
    void EncryptDecryptAes(Kernel::HLERequestContext& ctx);
    void EncryptSignDecryptVerifyAesCcm(Kernel::HLERequestContext& ctx);
    void GetRomId(Kernel::HLERequestContext& ctx);
    void GetRomId2(Kernel::HLERequestContext& ctx);
    void GetRomMakerCode(Kernel::HLERequestContext& ctx);
    void GetCTRCardAutoStartupBit(Kernel::HLERequestContext& ctx);
    void GetLocalFriendCodeSeed(Kernel::HLERequestContext& ctx);
    void GetDeviceId(Kernel::HLERequestContext& ctx);
    void SeedRNG(Kernel::HLERequestContext& ctx);
    void GenerateRandomBytes(Kernel::HLERequestContext& ctx);
    void InterfaceForPXI_0x04010084(Kernel::HLERequestContext& ctx);
    void InterfaceForPXI_0x04020082(Kernel::HLERequestContext& ctx);
    void InterfaceForPXI_0x04030044(Kernel::HLERequestContext& ctx);
    void InterfaceForPXI_0x04040044(Kernel::HLERequestContext& ctx);
};

/// Initializes the PS_PS Service
void InstallInterfaces(SM::ServiceManager& service_manager);

std::tuple<bool, LocalFriendCodeSeed> GetLocalFriendCodeSeedTuple();

} // namespace Service::PS
