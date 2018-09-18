// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_deb.h"

namespace Service::I2C {

I2C_DEB::I2C_DEB() : ServiceFramework{"i2c::DEB"} {}
I2C_DEB::~I2C_DEB() = default;

} // namespace Service::I2C
