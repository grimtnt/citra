// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included..

#pragma once

#include "core/hle/service/service.h"

namespace Service {
namespace MP {

class MP_U final : public ServiceFramework<MP_U> {
public:
    MP_U();
    ~MP_U();
};

} // namespace MP
} // namespace Service
