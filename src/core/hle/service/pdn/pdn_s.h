// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::PDN {

class PDN_S final : public ServiceFramework<PDN_S> {
public:
    PDN_S();
    ~PDN_S();
};

} // namespace Service::PDN
