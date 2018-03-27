// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mcu/cdc.h"

namespace Service {
namespace MCU {

CDC::CDC(std::shared_ptr<Module> mcu) : Module::Interface(std::move(mcu), "mcu::CDC", 1) {}

} // namespace MCU
} // namespace Service
