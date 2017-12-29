// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/configure_hacks.h"
#include "core/settings.h"
#include "ui_configure_hacks.h"

ConfigureHacks::ConfigureHacks(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureHacks>()) {
    ui->setupUi(this);

    this->setConfiguration();
}

ConfigureHacks::~ConfigureHacks() {}

void ConfigureHacks::setConfiguration() {
    ui->toggle_pipe3->setChecked(Settings::values.enable_pipe3);
    ui->toggle_pkfix->setChecked(Settings::values.fix_pokemon);
}

void ConfigureHacks::applyConfiguration() {
    Settings::values.enable_pipe3 = ui->toggle_pipe3->isChecked();
    Settings::values.fix_pokemon = ui->toggle_pkfix->isChecked();
    Settings::Apply();
}

void ConfigureHacks::retranslateUi() {
    ui->retranslateUi(this);
}
