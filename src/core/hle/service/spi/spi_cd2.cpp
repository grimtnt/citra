// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/spi/spi_cd2.h"

namespace Service::SPI {

SPI_CD2::SPI_CD2(std::shared_ptr<Module> spi) : Module::Interface{std::move(spi), "SPI::CD2"} {}

} // namespace Service::SPI
