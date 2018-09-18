// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::PDN {

class PDN_D final : public ServiceFramework<PDN_D> {
public:
    PDN_D();
    ~PDN_D();
};

} // namespace Service::PDN
