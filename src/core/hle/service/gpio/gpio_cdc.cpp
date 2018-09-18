// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/gpio/gpio_cdc.h"

namespace Service::GPIO {

GPIO_CDC::GPIO_CDC(std::shared_ptr<Module> gpio) : Module::Interface{std::move(gpio), "gpio:CDC"} {}

} // namespace Service::GPIO
