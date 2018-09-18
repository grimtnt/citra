// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/spi/spi_cs2.h"

namespace Service::SPI {

SPI_CS2::SPI_CS2(std::shared_ptr<Module> spi) : Module::Interface{std::move(spi), "SPI::CS2"} {}

} // namespace Service::SPI
