// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mp/mp.h"
#include "core/hle/service/mp/mp_u.h"

namespace Service::MP {

void InstallInterfaces(SM::ServiceManager& service_manager) {
    std::make_shared<MP_U>()->InstallAsService(service_manager);
}

} // namespace Service::MP
