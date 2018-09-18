// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/pdn/pdn_s.h"

namespace Service::PDN {

PDN_S::PDN_S() : ServiceFramework("pdn:s") {}
PDN_S::~PDN_S() = default;

} // namespace Service::PDN
