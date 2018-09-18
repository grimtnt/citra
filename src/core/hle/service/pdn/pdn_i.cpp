// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/pdn/pdn_i.h"

namespace Service::PDN {

PDN_I::PDN_I() : ServiceFramework{"pdn:i"} {}
PDN_I::~PDN_I() = default;

} // namespace Service::PDN
