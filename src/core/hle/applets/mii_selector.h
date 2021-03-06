// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include "common/common_funcs.h"
#include "common/common_types.h"
#include "core/hle/applets/applet.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/shared_memory.h"
#include "core/hle/result.h"
#include "core/hle/service/apt/apt.h"

namespace HLE::Applets {

struct MiiConfig {
    u8 enable_cancel_button;
    u8 enable_guest_mii;
    u8 show_on_top_screen;
    INSERT_PADDING_BYTES(5);
    std::array<u16_le, 0x40> title;
    INSERT_PADDING_BYTES(4);
    u8 show_guest_miis;
    INSERT_PADDING_BYTES(3);
    u32_le initially_selected_mii_index;
    u8 guest_mii_whitelist[6];
    u8 user_mii_whitelist[0x64];
    INSERT_PADDING_BYTES(2);
    u32_le magic_value;
};
static_assert(sizeof(MiiConfig) == 0x104, "MiiConfig structure has incorrect size");
#define ASSERT_REG_POSITION(field_name, position)                                                  \
    static_assert(offsetof(MiiConfig, field_name) == position,                                     \
                  "Field " #field_name " has invalid position")
ASSERT_REG_POSITION(title, 0x08);
ASSERT_REG_POSITION(show_guest_miis, 0x8C);
ASSERT_REG_POSITION(initially_selected_mii_index, 0x90);
ASSERT_REG_POSITION(guest_mii_whitelist, 0x94);
#undef ASSERT_REG_POSITION

struct MiiResult {
    u32_le return_code;
    u32_le is_guest_mii_selected;
    u32_le selected_guest_mii_index;
    // TODO(mailwl): expand to Mii Format structure: https://www.3dbrew.org/wiki/Mii
    u8 selected_mii_data[0x5C];
    INSERT_PADDING_BYTES(2);
    u16_be mii_data_checksum;
    u16_le guest_mii_name[0xC];
};
static_assert(sizeof(MiiResult) == 0x84, "MiiResult structure has incorrect size");
#define ASSERT_REG_POSITION(field_name, position)                                                  \
    static_assert(offsetof(MiiResult, field_name) == position,                                     \
                  "Field " #field_name " has invalid position")
ASSERT_REG_POSITION(selected_mii_data, 0x0C);
ASSERT_REG_POSITION(guest_mii_name, 0x6C);
#undef ASSERT_REG_POSITION

class MiiSelector final : public Applet {
public:
    MiiSelector(Service::APT::AppletId id, std::weak_ptr<Service::APT::AppletManager> manager)
        : Applet(id, std::move(manager)) {}

    ResultCode ReceiveParameter(const Service::APT::MessageParameter& parameter) override;
    ResultCode StartImpl(const Service::APT::AppletStartupParameter& parameter) override;
    void Update() override;

    static inline std::function<void(const MiiConfig&, MiiResult&)> cb;

private:
    /// This SharedMemory will be created when we receive the LibAppJustStarted message.
    /// It holds the framebuffer info retrieved by the application with
    /// GSPGPU::ImportDisplayCaptureInfo
    Kernel::SharedPtr<Kernel::SharedMemory> framebuffer_memory;

    MiiConfig config;
};
} // namespace HLE::Applets
