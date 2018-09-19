// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include "common/common_types.h"
#include "core/hle/service/cam/cam.h"

namespace Settings {

enum class KeyboardMode { StdIn, Qt };

enum class TicksMode { Auto, Accurate, Custom };

enum class InitClock {
    SystemTime = 0,
    FixedTime = 1,
};

namespace NativeButton {
enum Values {
    A,
    B,
    X,
    Y,
    Up,
    Down,
    Left,
    Right,
    L,
    R,
    Start,
    Select,

    ZL,
    ZR,

    Home,

    NumButtons,
};

constexpr int BUTTON_HID_BEGIN{A};
constexpr int BUTTON_IR_BEGIN{ZL};
constexpr int BUTTON_NS_BEGIN{Home};

constexpr int BUTTON_HID_END{BUTTON_IR_BEGIN};
constexpr int BUTTON_IR_END{BUTTON_NS_BEGIN};
constexpr int BUTTON_NS_END{NumButtons};

constexpr int NUM_BUTTONS_HID{BUTTON_HID_END - BUTTON_HID_BEGIN};
constexpr int NUM_BUTTONS_IR{BUTTON_IR_END - BUTTON_IR_BEGIN};
constexpr int NUM_BUTTONS_NS{BUTTON_NS_END - BUTTON_NS_BEGIN};

static const std::array<const char*, NumButtons> mapping = {{
    "button_a",
    "button_b",
    "button_x",
    "button_y",
    "button_up",
    "button_down",
    "button_left",
    "button_right",
    "button_l",
    "button_r",
    "button_start",
    "button_select",
    "button_zl",
    "button_zr",
    "button_home",
}};
} // namespace NativeButton

namespace NativeAnalog {
enum Values {
    CirclePad,
    CStick,

    NumAnalogs,
};

static const std::array<const char*, NumAnalogs> mapping = {{
    "circle_pad",
    "c_stick",
}};
} // namespace NativeAnalog

struct Values {
    // Control Panel
    float volume;
    bool headphones_connected;
    u8 factor_3d;
    bool p_adapter_connected;
    bool p_battery_charging;
    u32 p_battery_level;
    u32 n_wifi_status;
    u8 n_wifi_link_level;
    u8 n_state;

    // Controls
    std::array<std::string, NativeButton::NumButtons> buttons;
    std::array<std::string, NativeAnalog::NumAnalogs> analogs;
    std::string motion_device;
    std::string touch_device;
    std::string udp_input_address;
    u16 udp_input_port;
    u8 udp_pad_index;

    // Data Storage
    std::string sdmc_dir;

    // System
    int region_value;
    InitClock init_clock;
    u64 init_time;
    bool memory_developer_mode;

    // Renderer
    bool use_hw_shaders;
    bool accurate_shaders;

    // Logging
    std::string log_filter;

    // Audio
    std::string output_device;
    std::string input_device;

    // Camera
    std::array<std::string, Service::CAM::NumCameras> camera_name;
    std::array<std::string, Service::CAM::NumCameras> camera_config;
    std::array<int, Service::CAM::NumCameras> camera_flip;

    // Hacks
    bool priority_boost;
    TicksMode ticks_mode;
    u64 ticks;
    bool use_bos;
} extern values;

// a special value for Values::region_value indicating that citra will automatically select a region
// value to fit the region lockout info of the game
static constexpr int REGION_VALUE_AUTO_SELECT = -1;

void Apply();
void LogSettings();
} // namespace Settings
