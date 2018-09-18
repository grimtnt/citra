// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::CDC {

class CDC_CHK final : public ServiceFramework<CDC_CHK> {
public:
    CDC_CHK();
    ~CDC_CHK();
};

} // namespace Service::CDC
