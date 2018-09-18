// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/soc/soc.h"

namespace Service::SOC {

class SOC_U final : public Module::Interface {
public:
    explicit SOC_U(std::shared_ptr<Module> soc);
};

} // namespace Service::SOC
