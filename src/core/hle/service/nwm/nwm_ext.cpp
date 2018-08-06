// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/logging/log.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/service/nwm/nwm_ext.h"
#include "core/settings.h"
#include "core/core.h"

namespace Service {
namespace NWM {

NWM_EXT::NWM_EXT() : ServiceFramework("nwm::EXT") {
    static const FunctionInfo functions[] = {
        {0x00080040, &NWM_EXT::ControlWirelessEnabled, "ControlWirelessEnabled"},
    };
    RegisterHandlers(functions);
}

NWM_EXT::~NWM_EXT() = default;

void NWM_EXT::ControlWirelessEnabled(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x0008, 1, 0);
    u8 enabled = rp.Pop<u8>();

    ResultCode result = RESULT_SUCCESS;

    switch (enabled) {
    case 0: {
        if (Core::System::GetInstance().GetSharedPageHandler()->GetNetworkState() != SharedPage::NetworkState::Internet) {
            result =
                ResultCode(13, ErrorModule::NWM, ErrorSummary::InvalidState, ErrorLevel::Status);
            break;
        }

        Settings::values.n_wifi_status = Settings::values.enable_new_mode ? 2 : 1;
        Settings::values.n_wifi_link_level = 3;
        Settings::values.n_state = 2;
        SharedPage::shared_page.wifi_link_level = 3;
        SharedPage::shared_page.network_state = SharedPage::NetworkState::Disabled;
        break;
    }

    case 1: {
        if (Core::System::GetInstance().GetSharedPageHandler()->GetNetworkState() == SharedPage::NetworkState::Internet) {
            result =
                ResultCode(13, ErrorModule::NWM, ErrorSummary::InvalidState, ErrorLevel::Status);
            break;
        }
        Settings::values.n_wifi_status = 0;
        Settings::values.n_wifi_link_level = 0;
        Settings::values.n_state = 7;
        SharedPage::shared_page.wifi_link_level = 0;
        SharedPage::shared_page.network_state = SharedPage::NetworkState::Internet;
        break;
    }

    default: {
        LOG_ERROR(Service_NWM, "Invalid enabled value {}", enabled);
        break;
    }
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(result);
}
} // namespace NWM
} // namespace Service
