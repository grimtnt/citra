// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/pdn/pdn_g.h"

namespace Service::PDN {

PDN_G::PDN_G() : ServiceFramework{"pdn:g"} {}
PDN_G::~PDN_G() = default;

} // namespace Service::PDN
