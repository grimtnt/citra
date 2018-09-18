// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/gpio/gpio_mcu.h"

namespace Service::GPIO {

GPIO_MCU::GPIO_MCU(std::shared_ptr<Module> gpio) : Module::Interface{std::move(gpio), "gpio:MCU"} {}

} // namespace Service::GPIO
