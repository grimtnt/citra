// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::I2C {

class I2C_NFC final : public ServiceFramework<I2C_NFC> {
public:
    I2C_NFC();
    ~I2C_NFC();
};

} // namespace Service::I2C
