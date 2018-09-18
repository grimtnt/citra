// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_eep.h"

namespace Service::I2C {

I2C_EEP::I2C_EEP() : ServiceFramework{"i2c::EEP"} {}
I2C_EEP::~I2C_EEP() = default;

} // namespace Service::I2C
