// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/gpio/gpio_nfc.h"

namespace Service::GPIO {

GPIO_NFC::GPIO_NFC(std::shared_ptr<Module> gpio) : Module::Interface{std::move(gpio), "gpio:NFC"} {}

} // namespace Service::GPIO
