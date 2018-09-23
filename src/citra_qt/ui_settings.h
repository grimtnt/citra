// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <vector>
#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include "common/common_types.h"

namespace UISettings {

using ContextualShortcut = std::pair<QString, int>;
using Shortcut = std::pair<QString, ContextualShortcut>;

static const std::array<std::pair<QString, QString>, 4> themes = {
    {std::make_pair(QString("Default"), QString("default")),
     std::make_pair(QString("Dark"), QString("qdarkstyle")),
     std::make_pair(QString("Colorful"), QString("colorful")),
     std::make_pair(QString("Colorful Dark"), QString("colorful_dark"))}};

struct GameDir {
    QString path;
    bool deep_scan;
    bool expanded;
    bool operator==(const GameDir& rhs) const {
        return path == rhs.path;
    };
    bool operator!=(const GameDir& rhs) const {
        return !operator==(rhs);
    };
};

struct Values {
    QByteArray geometry;
    QByteArray state;

    QByteArray renderwindow_geometry;

    QByteArray gamelist_header_state;

    bool single_window_mode;
    bool fullscreen;
    bool display_titlebar;
    bool show_filter_bar;
    bool show_status_bar;

    // Game List
    int game_list_icon_size;
    int game_list_row_1;
    int game_list_row_2;
    bool game_list_hide_no_icon;

    u16 screenshot_resolution_factor;

    QList<UISettings::GameDir> game_dirs;
    QStringList recent_files;
    QString amiibo_path;

    QString theme;

    // Shortcut name <Shortcut, context>
    std::vector<Shortcut> shortcuts;

    // multiplayer settings
    QString nickname;
    QString ip;
    QString port;
    QString room_nickname;
    QString room_name;
    quint32 max_player;
    QString room_port;
    uint host_type;
    qulonglong game_id;

    // logging
    bool show_console;
};

extern Values values;
} // namespace UISettings

Q_DECLARE_METATYPE(UISettings::GameDir*);
