// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mcu/nwm.h"

namespace Service {
namespace MCU {

NWM::NWM(std::shared_ptr<Module> mcu) : Module::Interface(std::move(mcu), "mcu::NWM", 1) {}

} // namespace MCU
} // namespace Service
