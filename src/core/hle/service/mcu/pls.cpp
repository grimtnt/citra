// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/hle/service/mcu/pls.h"

namespace Service {
namespace MCU {

PLS::PLS(std::shared_ptr<Module> mcu) : Module::Interface(std::move(mcu), "mcu::PLS", 1) {
    static const FunctionInfo functions[] = {
        {0x00010000, nullptr, "GetDatetime"},  {0x00020000, nullptr, "GetSeconds"},
        {0x00030000, nullptr, "GetMinutes"},   {0x00040000, nullptr, "GetHour"},
        {0x00050000, nullptr, "GetDayOfWeek"}, {0x00060000, nullptr, "GetDay"},
        {0x00070000, nullptr, "GetMonth"},     {0x00080000, nullptr, "GetYear"},
    };
    RegisterHandlers(functions);
}

} // namespace MCU
} // namespace Service
