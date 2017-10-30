// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QWidget>
#include "common/logging/backend.h"
#include "common/logging/filter.h"
#include "common/logging/log.h"

namespace Ui {
class ConfigureLog;
}

class ConfigureLog : public QWidget {
    Q_OBJECT

public:
    explicit ConfigureLog(QWidget* parent = nullptr);
    ~ConfigureLog();

    void applyConfiguration();

private:
    void setConfiguration();

private:
    std::unique_ptr<Ui::ConfigureLog> ui;
};
