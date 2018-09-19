// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/service.h"

namespace Kernel {
class HLERequestContext;
} // namespace Kernel

namespace Service::ERR {

/// Interface to "err:f" service
class ERR_F final : public ServiceFramework<ERR_F> {
public:
    ERR_F();
    ~ERR_F();

private:
    /* ThrowFatalError function
     * Inputs:
     *  1-32: FatalErrInfo
     * Outputs:
     *     1: Result code
     */
    void ThrowFatalError(Kernel::HLERequestContext& ctx);

    /* SetUserString function
     * Inputs:
     *  1: Size
     *  2: String
     * Outputs:
     *  1: Result code
     */
    void SetUserString(Kernel::HLERequestContext& ctx);
};

void InstallInterfaces();

} // namespace Service::ERR