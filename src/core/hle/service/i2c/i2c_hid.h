// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::I2C {

class I2C_HID final : public ServiceFramework<I2C_HID> {
public:
    I2C_HID();
    ~I2C_HID();
};

} // namespace Service::I2C
