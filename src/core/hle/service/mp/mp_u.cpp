// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mp/mp_u.h"

namespace Service::MP {

MP_U::MP_U() : ServiceFramework("mp:u", DefaultMaxSessions) {
    const FunctionInfo functions[] = {
        {0x00010040, nullptr, "GetHandle"},     {0x00020000, nullptr, "Initialize"},
        {0x00030000, nullptr, "Shutdown"},      {0x000400C0, nullptr, "Bind"},
        {0x00050042, nullptr, "SetBeaconData"}, {0x00070042, nullptr, "SetAppData"},
        {0x00110080, nullptr, "RecvDataFrame"}, {0x001400C2, nullptr, "SendDataFrame"},
    };
    RegisterHandlers(functions);
}

MP_U::~MP_U() = default;

} // namespace Service::MP
