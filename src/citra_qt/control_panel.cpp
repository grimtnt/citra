// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QCheckBox>
#include <QComboBox>

#include "core/settings.h"
#include "core/hle/shared_page.h"
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
    ui->wifi_status->setCurrentIndex(Settings::values.n_wifi_status);
    ui->link_level->setValue(Settings::values.n_link_level);
    connect(ui->adapter_connected, &QCheckBox::stateChanged, this, &ControlPanel::OnAdapterConnectedChanged);
    connect(ui->battery_charging, &QCheckBox::stateChanged, this, &ControlPanel::OnBatteryChargingChanged);
    connect(ui->battery_level, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ControlPanel::OnBatteryLevelChanged);
    connect(ui->wifi_status, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ControlPanel::OnWifiStatusChanged);
    connect(ui->link_level, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ControlPanel::OnWifiLinkLevelChanged);
    setWindowTitle("Control Panel");
    setFixedSize(size());
}

ControlPanel::~ControlPanel() {
    delete ui;
}

void ControlPanel::OnAdapterConnectedChanged() {
    Settings::values.p_adapter_connected = ui->adapter_connected->isChecked();
    SharedPage::shared_page.battery_state.is_adapter_connected.Assign(
                static_cast<u8>(Settings::values.p_adapter_connected));
}

void ControlPanel::OnBatteryChargingChanged() {
    Settings::values.p_battery_charging = ui->battery_charging->isChecked();
    SharedPage::shared_page.battery_state.is_charging.Assign(
                static_cast<u8>(Settings::values.p_battery_charging));
}

void ControlPanel::OnBatteryLevelChanged() {
    Settings::values.p_battery_level = ui->battery_level->currentIndex() + 1;
    SharedPage::shared_page.battery_state.charge_level.Assign(
                static_cast<u8>(Settings::values.p_battery_level));
}

void ControlPanel::OnWifiStatusChanged() {
    Settings::values.n_wifi_status = ui->wifi_status->currentIndex();
}

void ControlPanel::OnWifiLinkLevelChanged() {
    Settings::values.n_wifi_link_level = ui->link_level->value();
    SharedPage::shared_page.wifi_link_level = ui->link_level->value();
}
