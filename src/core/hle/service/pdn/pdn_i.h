// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::PDN {

class PDN_I final : public ServiceFramework<PDN_I> {
public:
    PDN_I();
    ~PDN_I();
};

} // namespace Service::PDN
