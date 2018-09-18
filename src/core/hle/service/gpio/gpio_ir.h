// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/gpio/gpio.h"

namespace Service::GPIO {

class GPIO_IR final : public Module::Interface {
public:
    explicit GPIO_IR(std::shared_ptr<Module> gpio);
};

} // namespace Service::GPIO
