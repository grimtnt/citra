// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_qt/configuration/configure_hacks.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_hacks.h"

ConfigureHacks::ConfigureHacks(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureHacks>()) {
    ui->setupUi(this);

    setConfiguration();

    ui->toggle_priority_boost->setEnabled(!Core::System::GetInstance().IsPoweredOn());
    ui->toggle_cpu_jit_hacks->setEnabled(!Core::System::GetInstance().IsPoweredOn());
}

ConfigureHacks::~ConfigureHacks() {}

void ConfigureHacks::setConfiguration() {
    ui->toggle_priority_boost->setChecked(Settings::values.priority_boost);
    ui->toggle_cpu_jit_hacks->setChecked(Settings::values.cpu_jit_hacks);
}

void ConfigureHacks::applyConfiguration() {
    Settings::values.priority_boost = ui->toggle_priority_boost->isChecked();
    Settings::values.cpu_jit_hacks = ui->toggle_cpu_jit_hacks->isChecked();
}

void ConfigureHacks::retranslateUi() {
    ui->retranslateUi(this);
}
