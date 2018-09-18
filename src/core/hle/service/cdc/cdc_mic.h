// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::CDC {

class CDC_MIC final : public ServiceFramework<CDC_MIC> {
public:
    CDC_MIC();
    ~CDC_MIC();
};

} // namespace Service::CDC
