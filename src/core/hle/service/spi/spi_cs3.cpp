// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/spi/spi_cs3.h"

namespace Service::SPI {

SPI_CS3::SPI_CS3(std::shared_ptr<Module> spi) : Module::Interface{std::move(spi), "SPI::CS3"} {}

} // namespace Service::SPI
