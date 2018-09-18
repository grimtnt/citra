// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/cdc/cdc_mic.h"

namespace Service::CDC {

CDC_MIC::CDC_MIC() : ServiceFramework("cdc:MIC") {}
CDC_MIC::~CDC_MIC() = default;

} // namespace Service::CDC
