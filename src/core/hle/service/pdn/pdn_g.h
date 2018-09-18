// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/service.h"

namespace Service::PDN {

class PDN_G final : public ServiceFramework<PDN_G> {
public:
    PDN_G();
    ~PDN_G();
};

} // namespace Service::PDN
