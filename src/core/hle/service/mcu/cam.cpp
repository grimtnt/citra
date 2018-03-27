// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mcu/cam.h"

namespace Service {
namespace MCU {

CAM::CAM(std::shared_ptr<Module> mcu) : Module::Interface(std::move(mcu), "mcu::CAM", 1) {
    static const FunctionInfo functions[] = {
        {0x00010040, nullptr, "WriteCameraLedState"}, {0x00020080, nullptr, "ReadCameraLedState"},
    };
    RegisterHandlers(functions);
}

} // namespace MCU
} // namespace Service
