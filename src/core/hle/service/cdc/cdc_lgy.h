// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::CDC {

class CDC_LGY final : public ServiceFramework<CDC_LGY> {
public:
    CDC_LGY();
    ~CDC_LGY();
};

} // namespace Service::CDC
