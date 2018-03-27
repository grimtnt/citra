// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mcu/snd.h"

namespace Service {
namespace MCU {

SND::SND(std::shared_ptr<Module> mcu) : Module::Interface(std::move(mcu), "mcu::SND", 1) {
    static const FunctionInfo functions[] = {
        {0x00010080, nullptr, "GetSoundVolume"}, {0x00030000, nullptr, "GetRegister25h"},
    };
    RegisterHandlers(functions);
}

} // namespace MCU
} // namespace Service
