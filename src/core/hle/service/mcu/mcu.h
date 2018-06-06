// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include "core/hle/kernel/kernel.h"
#include "core/hle/service/service.h"

namespace Service {
namespace MCU {

class Module final {
public:
    class Interface : public ServiceFramework<Interface> {
    public:
        Interface(std::shared_ptr<Module> mcu, const char* name, u32 max_session);

        /**
         * MCU::GetBatteryLevel service function
         *  Outputs:
         *      1 : Result of function, 0 on success, otherwise error code
         *      2 : Battery level (0-0x64)
         */
        void GetBatteryLevel(Kernel::HLERequestContext& ctx, u16 id);

        /**
         * MCU::GetBatteryChargeState service function
         *  Outputs:
         *      1 : Result of function, 0 on success, otherwise error code
         *      2 : True if the battery is charging, otherwise false
         */
        void GetBatteryChargeState(Kernel::HLERequestContext& ctx);

        /**
         * MCU::Set3DLEDState service function
         *  Inputs:
         *      1 : 3D LED state
         *  Outputs:
         *      1 : Result of function, 0 on success, otherwise error code
         */
        void Set3DLEDState(Kernel::HLERequestContext& ctx);

        /**
         * MCU::GetSoundVolume service function
         *  Outputs:
         *      1 : Result of function, 0 on success, otherwise error code
         *      2 : Sound volume (0-0x3F)
         */
        void GetSoundVolume(Kernel::HLERequestContext& ctx);

        /// A helper function for dispatching service functions that have multiple IDs
        template <void (Interface::*function)(Kernel::HLERequestContext& ctx, u16 id), u16 id>
        void D(Kernel::HLERequestContext& ctx) {
            (this->*function)(ctx, id);
        }

    protected:
        std::shared_ptr<Module> mcu;
    };
};

void InstallInterfaces(SM::ServiceManager& service_manager);

} // namespace MCU
} // namespace Service
