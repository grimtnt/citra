// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/i2c/i2c_cam.h"

namespace Service::I2C {

I2C_CAM::I2C_CAM() : ServiceFramework{"i2c::CAM"} {}
I2C_CAM::~I2C_CAM() = default;

} // namespace Service::I2C
