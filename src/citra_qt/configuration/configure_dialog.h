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

    bool sd_card_root_changed;

private slots:
    void onLanguageChanged(const QString& locale);

signals:
    void languageChanged(const QString& locale);

private:
    std::unique_ptr<Ui::ConfigureDialog> ui;
};
