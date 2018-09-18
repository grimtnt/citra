// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/spi/spi_def.h"

namespace Service::SPI {

SPI_DEF::SPI_DEF(std::shared_ptr<Module> spi) : Module::Interface{std::move(spi), "SPI::DEF"} {}

} // namespace Service::SPI
