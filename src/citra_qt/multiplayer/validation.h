// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QRegExp>
#include <QValidator>

class Validation {
public:
    Validation() : ip(ip_regex), port(0, 65535) {}

    ~Validation() = default;

    const QValidator* GetIP() const {
        return &ip;
    }
    const QValidator* GetPort() const {
        return &port;
    }

private:
    /// ipv4 address only
    QRegExp ip_regex = QRegExp(
        "(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|"
        "2[0-4][0-9]|25[0-5])");
    QRegExpValidator ip;

    /// port must be between 0 and 65535
    QIntValidator port;
};
