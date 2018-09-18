// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::I2C {

class I2C_MCU final : public ServiceFramework<I2C_MCU> {
public:
    I2C_MCU();
    ~I2C_MCU();
};

} // namespace Service::I2C
