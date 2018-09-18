// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::PDN {

class PDN_C final : public ServiceFramework<PDN_C> {
public:
    PDN_C();
    ~PDN_C();
};

} // namespace Service::PDN
