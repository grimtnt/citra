// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/kernel/kernel.h"
#include "core/hle/service/service.h"

namespace Service {
namespace NS {

/// Interface to "ns:s" service
class NS_S final : public ServiceFramework<NS_S> {
public:
    NS_S();
    ~NS_S();

    void LaunchTitle(Kernel::HLERequestContext& ctx);
    void ShutdownAsync(Kernel::HLERequestContext& ctx);
    void RebootSystemClean(Kernel::HLERequestContext& ctx);
};

} // namespace NS
} // namespace Service
