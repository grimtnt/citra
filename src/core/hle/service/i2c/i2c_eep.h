// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::I2C {

class I2C_EEP final : public ServiceFramework<I2C_EEP> {
public:
    I2C_EEP();
    ~I2C_EEP();
};

} // namespace Service::I2C
