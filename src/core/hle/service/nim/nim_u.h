// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/service.h"

namespace Service::NIM {

class NIM_U final : public ServiceFramework<NIM_U> {
public:
    NIM_U();
    ~NIM_U();

private:
    void CheckForSysUpdateEvent(Kernel::HLERequestContext& ctx);
    void CheckSysUpdateAvailable(Kernel::HLERequestContext& ctx);

    Kernel::SharedPtr<Kernel::Event> nim_system_update_event;
};

} // namespace Service::NIM
