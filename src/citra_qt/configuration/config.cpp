// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QSettings>
#include "citra_qt/configuration/config.h"
#include "citra_qt/ui_settings.h"
#include "common/file_util.h"
#include "input_common/main.h"
#include "input_common/udp/client.h"
#include "network/network.h"

Config::Config() {
    // TODO: Don't hardcode the path; let the frontend decide where to put the config files.
    qt_config_loc = FileUtil::GetUserPath(D_CONFIG_IDX) + "qt-config.ini";
    FileUtil::CreateFullPath(qt_config_loc);
    qt_config = new QSettings(QString::fromStdString(qt_config_loc), QSettings::IniFormat);

    Reload();
}

const std::array<int, Settings::NativeButton::NumButtons> Config::default_buttons = {
    Qt::Key_A, Qt::Key_S, Qt::Key_Z, Qt::Key_X, Qt::Key_T, Qt::Key_G, Qt::Key_F, Qt::Key_H,
    Qt::Key_Q, Qt::Key_W, Qt::Key_M, Qt::Key_N, Qt::Key_1, Qt::Key_2, Qt::Key_B,
};

const std::array<std::array<int, 5>, Settings::NativeAnalog::NumAnalogs> Config::default_analogs{{
    {
        Qt::Key_Up,
        Qt::Key_Down,
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_D,
    },
    {
        Qt::Key_I,
        Qt::Key_K,
        Qt::Key_J,
        Qt::Key_L,
        Qt::Key_D,
    },
}};

void Config::ReadValues() {
    qt_config->beginGroup("ControlPanel");
    Settings::values.volume = qt_config->value("volume", 1).toFloat();
    Settings::values.headphones_connected =
        qt_config->value("headphones_connected", false).toBool();
    Settings::values.factor_3d = qt_config->value("factor_3d", 0).toInt();
    Settings::values.p_adapter_connected = qt_config->value("p_adapter_connected", true).toBool();
    Settings::values.p_battery_charging = qt_config->value("p_battery_charging", true).toBool();
    Settings::values.p_battery_level =
        static_cast<u32>(qt_config->value("p_battery_level", 5).toInt());
    Settings::values.n_wifi_status = static_cast<u32>(qt_config->value("n_wifi_status", 0).toInt());
    Settings::values.n_wifi_link_level =
        static_cast<u8>(qt_config->value("n_wifi_link_level", 0).toInt());
    Settings::values.n_state = static_cast<u8>(qt_config->value("n_state", 0).toInt());
    qt_config->endGroup();

    qt_config->beginGroup("Controls");
    for (int i{}; i < Settings::NativeButton::NumButtons; ++i) {
        std::string default_param{InputCommon::GenerateKeyboardParam(default_buttons[i])};
        Settings::values.buttons[i] =
            qt_config
                ->value(Settings::NativeButton::mapping[i], QString::fromStdString(default_param))
                .toString()
                .toStdString();
        if (Settings::values.buttons[i].empty())
            Settings::values.buttons[i] = default_param;
    }

    for (int i{}; i < Settings::NativeAnalog::NumAnalogs; ++i) {
        std::string default_param{InputCommon::GenerateAnalogParamFromKeys(
            default_analogs[i][0], default_analogs[i][1], default_analogs[i][2],
            default_analogs[i][3], default_analogs[i][4], 0.5f)};
        Settings::values.analogs[i] =
            qt_config
                ->value(Settings::NativeAnalog::mapping[i], QString::fromStdString(default_param))
                .toString()
                .toStdString();
        if (Settings::values.analogs[i].empty())
            Settings::values.analogs[i] = default_param;
    }

    Settings::values.motion_device =
        qt_config
            ->value("motion_device",
                    "engine:motion_emu,update_period:100,sensitivity:0.01,tilt_clamp:90.0")
            .toString()
            .toStdString();
    Settings::values.touch_device =
        qt_config->value("touch_device", "engine:emu_window").toString().toStdString();

    Settings::values.udp_input_address =
        qt_config->value("udp_input_address", InputCommon::CemuhookUDP::DEFAULT_ADDR)
            .toString()
            .toStdString();
    Settings::values.udp_input_port = static_cast<u16>(
        qt_config->value("udp_input_port", InputCommon::CemuhookUDP::DEFAULT_PORT).toInt());
    Settings::values.udp_pad_index = static_cast<u8>(qt_config->value("udp_pad_index", 0).toUInt());
    qt_config->endGroup();

    qt_config->beginGroup("Renderer");
    Settings::values.use_hw_shaders = qt_config->value("use_hw_shaders", true).toBool();
    Settings::values.accurate_shaders = qt_config->value("accurate_shaders", true).toBool();
    qt_config->endGroup();

    qt_config->beginGroup("Audio");
    Settings::values.output_device =
        qt_config->value("output_device", "auto").toString().toStdString();
    Settings::values.input_device =
        qt_config->value("input_device", "auto").toString().toStdString();
    qt_config->endGroup();

    using namespace Service::CAM;
    qt_config->beginGroup("Camera");
    Settings::values.camera_name[OuterRightCamera] =
        qt_config->value("camera_outer_right_name", "blank").toString().toStdString();
    Settings::values.camera_config[OuterRightCamera] =
        qt_config->value("camera_outer_right_config", "").toString().toStdString();
    Settings::values.camera_flip[OuterRightCamera] =
        qt_config->value("camera_outer_right_flip", 0).toInt();
    Settings::values.camera_name[InnerCamera] =
        qt_config->value("camera_inner_name", "blank").toString().toStdString();
    Settings::values.camera_config[InnerCamera] =
        qt_config->value("camera_inner_config", "").toString().toStdString();
    Settings::values.camera_flip[InnerCamera] = qt_config->value("camera_inner_flip", 0).toInt();
    Settings::values.camera_name[OuterLeftCamera] =
        qt_config->value("camera_outer_left_name", "blank").toString().toStdString();
    Settings::values.camera_config[OuterLeftCamera] =
        qt_config->value("camera_outer_left_config", "").toString().toStdString();
    Settings::values.camera_flip[OuterLeftCamera] =
        qt_config->value("camera_outer_left_flip", 0).toInt();
    qt_config->endGroup();

    qt_config->beginGroup("Data Storage");
    Settings::values.sdmc_dir = qt_config->value("sdmc_dir", "").toString().toStdString();
    qt_config->endGroup();

    qt_config->beginGroup("System");
    Settings::values.region_value =
        qt_config->value("region_value", Settings::REGION_VALUE_AUTO_SELECT).toInt();
    Settings::values.init_clock = static_cast<Settings::InitClock>(
        qt_config->value("init_clock", static_cast<u32>(Settings::InitClock::SystemTime)).toInt());
    Settings::values.init_time = qt_config->value("init_time", 946681277ULL).toULongLong();
    Settings::values.memory_developer_mode =
        qt_config->value("memory_developer_mode", false).toBool();
    qt_config->endGroup();

    qt_config->beginGroup("Miscellaneous");
    Settings::values.log_filter = qt_config->value("log_filter", "*:Info").toString().toStdString();
    qt_config->endGroup();

    qt_config->beginGroup("Hacks");
    Settings::values.priority_boost = qt_config->value("priority_boost", false).toBool();
    Settings::values.ticks_mode =
        static_cast<Settings::TicksMode>(qt_config->value("ticks_mode", 0).toInt());
    Settings::values.ticks = qt_config->value("ticks", 0).toULongLong();
    Settings::values.use_bos = qt_config->value("use_bos", false).toBool();
    qt_config->endGroup();

    qt_config->beginGroup("UI");
    UISettings::values.theme = qt_config->value("theme", UISettings::themes[0].second).toString();

    qt_config->beginGroup("Paths");
    int size{qt_config->beginReadArray("gamedirs")};
    for (int i{}; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::GameDir game_dir{};
        game_dir.path = qt_config->value("path").toString();
        game_dir.deep_scan = qt_config->value("deep_scan", false).toBool();
        game_dir.expanded = qt_config->value("expanded", true).toBool();
        UISettings::values.game_dirs.append(game_dir);
    }
    qt_config->endArray();
    // create NAND and SD card directories if empty, these are not removable through the UI
    if (UISettings::values.game_dirs.isEmpty()) {
        UISettings::GameDir game_dir;
        game_dir.path = "INSTALLED";
        game_dir.expanded = true;
        UISettings::values.game_dirs.append(game_dir);
        game_dir.path = "SYSTEM";
        UISettings::values.game_dirs.append(game_dir);
    }
    UISettings::values.recent_files = qt_config->value("recentFiles").toStringList();
    qt_config->endGroup();

    qt_config->beginGroup("Shortcuts");
    QStringList groups = qt_config->childGroups();
    for (auto group : groups) {
        qt_config->beginGroup(group);

        QStringList hotkeys = qt_config->childGroups();
        for (auto hotkey : hotkeys) {
            qt_config->beginGroup(hotkey);
            UISettings::values.shortcuts.emplace_back(UISettings::Shortcut(
                group + "/" + hotkey,
                UISettings::ContextualShortcut(qt_config->value("KeySeq").toString(),
                                               qt_config->value("Context").toInt())));
            qt_config->endGroup();
        }

        qt_config->endGroup();
    }
    qt_config->endGroup();

    UISettings::values.show_console = qt_config->value("showConsole", false).toBool();

    qt_config->beginGroup("Multiplayer");
    UISettings::values.ip = qt_config->value("ip", "").toString();
    UISettings::values.port = qt_config->value("port", Network::DefaultRoomPort).toString();
    UISettings::values.port_host =
        qt_config->value("port_host", Network::DefaultRoomPort).toString();
    qt_config->endGroup();

    qt_config->endGroup();
}

void Config::SaveValues() {
    qt_config->beginGroup("ControlPanel");
    qt_config->setValue("volume", Settings::values.volume);
    qt_config->setValue("headphones_connected", Settings::values.headphones_connected);
    qt_config->setValue("factor_3d", Settings::values.factor_3d);
    qt_config->setValue("p_adapter_connected", Settings::values.p_adapter_connected);
    qt_config->setValue("p_battery_charging", Settings::values.p_battery_charging);
    qt_config->setValue("p_battery_level", Settings::values.p_battery_level);
    qt_config->setValue("n_wifi_status", Settings::values.n_wifi_status);
    qt_config->setValue("n_wifi_link_level", Settings::values.n_wifi_link_level);
    qt_config->setValue("n_state", Settings::values.n_state);
    qt_config->endGroup();

    qt_config->beginGroup("Controls");
    for (int i{}; i < Settings::NativeButton::NumButtons; ++i) {
        qt_config->setValue(QString::fromStdString(Settings::NativeButton::mapping[i]),
                            QString::fromStdString(Settings::values.buttons[i]));
    }
    for (int i{}; i < Settings::NativeAnalog::NumAnalogs; ++i) {
        qt_config->setValue(QString::fromStdString(Settings::NativeAnalog::mapping[i]),
                            QString::fromStdString(Settings::values.analogs[i]));
    }
    qt_config->setValue("motion_device", QString::fromStdString(Settings::values.motion_device));
    qt_config->setValue("touch_device", QString::fromStdString(Settings::values.touch_device));
    qt_config->setValue("udp_input_address",
                        QString::fromStdString(Settings::values.udp_input_address));
    qt_config->setValue("udp_input_port", Settings::values.udp_input_port);
    qt_config->setValue("udp_pad_index", Settings::values.udp_pad_index);
    qt_config->endGroup();

    qt_config->beginGroup("Renderer");
    qt_config->setValue("use_hw_shaders", Settings::values.use_hw_shaders);
    qt_config->setValue("accurate_shaders", Settings::values.accurate_shaders);
    qt_config->endGroup();

    qt_config->beginGroup("Audio");
    qt_config->setValue("output_device", QString::fromStdString(Settings::values.output_device));
    qt_config->setValue("input_device", QString::fromStdString(Settings::values.input_device));
    qt_config->endGroup();

    using namespace Service::CAM;
    qt_config->beginGroup("Camera");
    qt_config->setValue("camera_outer_right_name",
                        QString::fromStdString(Settings::values.camera_name[OuterRightCamera]));
    qt_config->setValue("camera_outer_right_config",
                        QString::fromStdString(Settings::values.camera_config[OuterRightCamera]));
    qt_config->setValue("camera_inner_name",
                        QString::fromStdString(Settings::values.camera_name[InnerCamera]));
    qt_config->setValue("camera_inner_config",
                        QString::fromStdString(Settings::values.camera_config[InnerCamera]));
    qt_config->setValue("camera_outer_left_name",
                        QString::fromStdString(Settings::values.camera_name[OuterLeftCamera]));
    qt_config->setValue("camera_outer_left_config",
                        QString::fromStdString(Settings::values.camera_config[OuterLeftCamera]));
    qt_config->endGroup();

    qt_config->beginGroup("Data Storage");
    qt_config->setValue("sdmc_dir", QString::fromStdString(Settings::values.sdmc_dir));
    qt_config->endGroup();

    qt_config->beginGroup("System");
    qt_config->setValue("region_value", Settings::values.region_value);
    qt_config->setValue("init_clock", static_cast<u32>(Settings::values.init_clock));
    qt_config->setValue("init_time", static_cast<unsigned long long>(Settings::values.init_time));
    qt_config->setValue("memory_developer_mode", Settings::values.memory_developer_mode);
    qt_config->endGroup();

    qt_config->beginGroup("Miscellaneous");
    qt_config->setValue("log_filter", QString::fromStdString(Settings::values.log_filter));
    qt_config->endGroup();

    qt_config->beginGroup("Hacks");
    qt_config->setValue("priority_boost", Settings::values.priority_boost);
    qt_config->setValue("ticks_mode", static_cast<int>(Settings::values.ticks_mode));
    qt_config->setValue("ticks", static_cast<unsigned long long>(Settings::values.ticks));
    qt_config->setValue("use_bos", Settings::values.use_bos);
    qt_config->endGroup();

    qt_config->beginGroup("UI");
    qt_config->setValue("theme", UISettings::values.theme);

    qt_config->beginGroup("Paths");
    qt_config->beginWriteArray("gamedirs");
    for (int i{}; i < UISettings::values.game_dirs.size(); ++i) {
        qt_config->setArrayIndex(i);
        const auto& game_dir = UISettings::values.game_dirs.at(i);
        qt_config->setValue("path", game_dir.path);
        qt_config->setValue("deep_scan", game_dir.deep_scan);
        qt_config->setValue("expanded", game_dir.expanded);
    }
    qt_config->endArray();
    qt_config->setValue("recentFiles", UISettings::values.recent_files);
    qt_config->endGroup();

    qt_config->beginGroup("Shortcuts");
    for (auto shortcut : UISettings::values.shortcuts) {
        qt_config->setValue(shortcut.first + "/KeySeq", shortcut.second.first);
        qt_config->setValue(shortcut.first + "/Context", shortcut.second.second);
    }
    qt_config->endGroup();

    qt_config->setValue("showConsole", UISettings::values.show_console);

    qt_config->beginGroup("Multiplayer");
    qt_config->setValue("ip", UISettings::values.ip);
    qt_config->setValue("port", UISettings::values.port);
    qt_config->setValue("port_host", UISettings::values.port_host);
    qt_config->endGroup();

    qt_config->endGroup();
}

void Config::Reload() {
    ReadValues();
    Settings::Apply();
}

void Config::Save() {
    SaveValues();
}

Config::~Config() {
    Save();

    delete qt_config;
}
