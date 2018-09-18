// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_ir.h"

namespace Service::I2C {

I2C_IR::I2C_IR() : ServiceFramework{"i2c::IR"} {}
I2C_IR::~I2C_IR() = default;

} // namespace Service::I2C
