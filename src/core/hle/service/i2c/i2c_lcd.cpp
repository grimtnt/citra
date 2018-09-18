// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_lcd.h"

namespace Service::I2C {

I2C_LCD::I2C_LCD() : ServiceFramework{"i2c::LCD"} {}
I2C_LCD::~I2C_LCD() = default;

} // namespace Service::I2C
