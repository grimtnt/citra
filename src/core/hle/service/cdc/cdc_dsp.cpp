// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/cdc/cdc_dsp.h"

namespace Service::CDC {

CDC_DSP::CDC_DSP() : ServiceFramework{"cdc:DSP"} {}
CDC_DSP::~CDC_DSP() = default;

} // namespace Service::CDC
