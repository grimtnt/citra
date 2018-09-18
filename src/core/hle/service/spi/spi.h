// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include "core/hle/service/service.h"

namespace Service::SPI {

class Module final {
public:
    Module();
    ~Module();

    class Interface : public ServiceFramework<Interface> {
    public:
        Interface(std::shared_ptr<Module> spi, const char* name);
        ~Interface();

    private:
        std::shared_ptr<Module> spi;
    };
};

void InstallInterfaces(SM::ServiceManager& service_manager);

} // namespace Service::SPI
