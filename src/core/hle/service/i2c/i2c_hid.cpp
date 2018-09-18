// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_hid.h"

namespace Service::I2C {

I2C_HID::I2C_HID() : ServiceFramework{"i2c::HID"} {}
I2C_HID::~I2C_HID() = default;

} // namespace Service::I2C
