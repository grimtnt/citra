// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mcu/gpu.h"

namespace Service {
namespace MCU {

GPU::GPU(std::shared_ptr<Module> mcu) : Module::Interface(std::move(mcu), "mcu::GPU", 1) {
    static const FunctionInfo functions[] = {
        {0x00010000, nullptr, "GetLcdPowerState"},
        {0x00020080, nullptr, "SetLcdPowerState"},
        {0x00030000, nullptr, "GetGpuLcdInterfaceState"},
        {0x00040040, nullptr, "SetGpuLcdInterfaceState"},
        {0x00050040, nullptr, "SetTopScreenFlicker"},
        {0x00060080, nullptr, "GetTopScreenFlicker"},
        {0x00070040, nullptr, "SetBottomScreenFlicker"},
        {0x00080080, nullptr, "GetBottomScreenFlicker"},
        {0x00090000, nullptr, "GetMcuFwVerHigh"},
        {0x000A0000, nullptr, "GetMcuFwVerLow"},
        {0x000B0040, nullptr, "Set3dLedState"},
        {0x000C0000, nullptr, "Get3dLedState"},
        {0x000D0000, nullptr, "GetMcuGpuEventHandle"},
        {0x000E0000, nullptr, "GetMcuGpuEventReason"},
    };
    RegisterHandlers(functions);
}

} // namespace MCU
} // namespace Service
