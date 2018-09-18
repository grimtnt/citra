// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_mcu.h"

namespace Service::I2C {

I2C_MCU::I2C_MCU() : ServiceFramework("i2c::MCU") {}
I2C_MCU::~I2C_MCU() = default;

} // namespace Service::I2C
