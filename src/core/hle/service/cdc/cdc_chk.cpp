// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/cdc/cdc_chk.h"

namespace Service::CDC {

CDC_CHK::CDC_CHK() : ServiceFramework("cdc:CHK") {}
CDC_CHK::~CDC_CHK() = default;

} // namespace Service::CDC
