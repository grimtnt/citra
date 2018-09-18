// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/spi/spi_nor.h"

namespace Service::SPI {

SPI_NOR::SPI_NOR(std::shared_ptr<Module> spi) : Module::Interface{std::move(spi), "SPI::NOR"} {}

} // namespace Service::SPI
