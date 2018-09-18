// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_nfc.h"

namespace Service::I2C {

I2C_NFC::I2C_NFC() : ServiceFramework{"i2c::NFC"} {}
I2C_NFC::~I2C_NFC() = default;

} // namespace Service::I2C
