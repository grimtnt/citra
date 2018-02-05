// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QCheckBox>
#include <QComboBox>
#include "core/settings.h"
#include "control_panel.h"
#include "ui_control_panel.h"

ControlPanel::ControlPanel(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);
    ui->adapter_connected->setChecked(Settings::values.p_adapter_connected);
    ui->battery_charging->setChecked(Settings::values.p_battery_charging);
    ui->battery_level->setCurrentIndex(Settings::values.p_battery_level - 1);
    connect(ui->adapter_connected, &QCheckBox::stateChanged, this, &ControlPanel::OnAdapterConnectedChanged);
    connect(ui->battery_charging, &QCheckBox::stateChanged, this, &ControlPanel::OnBatteryChargingChanged);
    connect(ui->battery_level, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ControlPanel::OnBatteryLevelChanged);
    setWindowTitle("Control Panel");
    setFixedSize(size());
}

ControlPanel::~ControlPanel() {
    delete ui;
}

void ControlPanel::OnAdapterConnectedChanged() {
    Settings::values.p_adapter_connected = ui->adapter_connected->isChecked();
}

void ControlPanel::OnBatteryChargingChanged() {
    Settings::values.p_battery_charging = ui->battery_charging->isChecked();
}

void ControlPanel::OnBatteryLevelChanged() {
    Settings::values.p_battery_level = ui->battery_level->currentIndex() + 1;
}
