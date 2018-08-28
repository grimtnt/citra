// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QDialog>

namespace Ui {
class ConfigureDialog;
} // namespace Ui

class ConfigureDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureDialog(QWidget* parent);
    ~ConfigureDialog();

    void applyConfiguration();

private:
    std::unique_ptr<Ui::ConfigureDialog> ui;
};
