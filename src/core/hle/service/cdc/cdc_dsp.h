// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::CDC {

class CDC_DSP final : public ServiceFramework<CDC_DSP> {
public:
    CDC_DSP();
    ~CDC_DSP();
};

} // namespace Service::CDC
