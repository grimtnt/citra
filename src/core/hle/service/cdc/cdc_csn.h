// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::CDC {

class CDC_CSN final : public ServiceFramework<CDC_CSN> {
public:
    CDC_CSN();
    ~CDC_CSN();
};

} // namespace Service::CDC
