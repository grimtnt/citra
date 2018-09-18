// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/cdc/cdc_csn.h"

namespace Service::CDC {

CDC_CSN::CDC_CSN() : ServiceFramework("cdc:CSN") {}
CDC_CSN::~CDC_CSN() = default;

} // namespace Service::CDC
