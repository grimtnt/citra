// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/logging/log.h"
#include "core/hle/shared_page.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/service/cfg/cfg.h"
#include "core/hle/service/nwm/nwm_ext.h"
#include "core/settings.h"

namespace Service {
namespace NWM {

void NWM_EXT::ControlWirelessEnabled(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x0008, 1, 0);
    u8 enabled = rp.Pop<u8>();

    ResultCode result = RESULT_SUCCESS;

    switch (enabled) {
        case 0: {
            if (SharedPage::shared_page.network_state != 7) {
                result = ResultCode(ErrCodes::D_13, ErrorModule::NWM, ErrorSummary::InvalidState,
                                    ErrorLevel::Status);
                break;
            }
            switch (static_cast<CFG::SystemModel>(CFG::GetSystemModelID())) {
            case CFG::SystemModel::NINTENDO_3DS:
            case CFG::SystemModel::NINTENDO_3DS_XL:
            case CFG::SystemModel::NINTENDO_2DS:
                Settings::values.n_wifi_status = 1;
                break;
            case CFG::SystemModel::NEW_NINTENDO_3DS:
            case CFG::SystemModel::NEW_NINTENDO_3DS_XL:
            case CFG::SystemModel::NEW_NINTENDO_2DS_XL:
                Settings::values.n_wifi_status = 2;
                break;
            }
            Settings::values.n_wifi_link_level = 3;
            Settings::values.n_state = 2;
            SharedPage::shared_page.wifi_link_level = 3;
            SharedPage::shared_page.network_state = 2;
            break;
        }

        case 1: {
            if (SharedPage::shared_page.network_state == 7) {
                result = ResultCode(ErrCodes::D_13, ErrorModule::NWM, ErrorSummary::InvalidState,
                                    ErrorLevel::Status);
                break;
            }
            Settings::values.n_wifi_status = 0;
            Settings::values.n_wifi_link_level = 0;
            Settings::values.n_state = 7;
            SharedPage::shared_page.wifi_link_level = 0;
            SharedPage::shared_page.network_state = 7;
            break;
        }

        default: {
            LOG_ERROR(Service_NWM, "Invalid enabled value %u", static_cast<u32>(enabled));
            break;
        }
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(result);
}

NWM_EXT::NWM_EXT() : ServiceFramework("nwm::EXT") {
    static const FunctionInfo functions[] = {
        {0x00080040, &NWM_EXT::ControlWirelessEnabled, "ControlWirelessEnabled"},
    };
    RegisterHandlers(functions);
}
    
NWM_EXT::~NWM_EXT() = default;

} // namespace NWM
} // namespace Service
