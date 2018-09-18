// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_qtm.h"

namespace Service::I2C {

I2C_QTM::I2C_QTM() : ServiceFramework{"i2c::QTM"} {}
I2C_QTM::~I2C_QTM() = default;

} // namespace Service::I2C
