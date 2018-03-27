// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mcu/hwc.h"

namespace Service {
namespace MCU {

HWC::HWC(std::shared_ptr<Module> mcu) : Module::Interface(std::move(mcu), "mcu::HWC", 1) {
    static const FunctionInfo functions[] = {
        {0x00010082, nullptr, "ReadRegister"},
        {0x00020082, nullptr, "WriteRegister"},
        {0x00030042, nullptr, "GetInfoRegisters"},
        {0x00040000, nullptr, "GetBatteryVoltage"},
        {0x00050000, &HWC::GetBatteryLevel, "GetBatteryLevel"},
        {0x00060040, nullptr, "SetPowerLEDPattern"},
        {0x00070040, nullptr, "SetWifiLEDState"},
        {0x00080040, nullptr, "SetCameraLEDPattern"},
        {0x00090040, &HWC::Set3DLEDState, "Set3DLEDState"},
        {0x000A0640, nullptr, "SetInfoLEDPattern"},
        {0x000B0000, &HWC::GetSoundVolume, "GetSoundVolume"},
        {0x000C0040, nullptr, "SetTopScreenFlicker"},
        {0x000D0040, nullptr, "SetBottomScreenFlicker"},
        {0x000F00C0, nullptr, "GetRtcTime"},
        {0x00100000, nullptr, "GetMcuFwVerHigh"},
        {0x00110000, nullptr, "GetMcuFwVerLow"},
    };
    RegisterHandlers(functions);
}

} // namespace MCU
} // namespace Service
