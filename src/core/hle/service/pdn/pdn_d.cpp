// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/pdn/pdn_d.h"

namespace Service::PDN {

PDN_D::PDN_D() : ServiceFramework{"pdn:d"} {}
PDN_D::~PDN_D() = default;

} // namespace Service::PDN
