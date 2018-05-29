// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QCheckBox>
#include <QComboBox>
#include "citra_qt/control_panel.h"
#include "core/hle/shared_page.h"
#include "core/settings.h"
#include "ui_control_panel.h"

namespace SharedPageUtil {
static int nsti(u8 state) {
    switch (state) {
    case 2:
        return 3;
    case 3:
    case 4:
    case 6:
        return 2;
    case 7:
        return 1;
    default:
        return 0;
    }
}

static u8 itns(int index) {
    switch (index) {
    case 0:
        return 0;
    case 1:
        return 7;
    case 2:
        return 3;
    case 3:
        return 2;
    }
    return 0;
}
} // namespace SharedPageUtil

ControlPanel::ControlPanel(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint),
      ui(std::make_unique<Ui::ControlPanel>()) {
    ui->setupUi(this);
    ui->shared_page_enable_3d->setChecked(Settings::values.sp_enable_3d);
    ui->power_adapter_connected->setChecked(Settings::values.p_adapter_connected);
    ui->power_battery_charging->setChecked(Settings::values.p_battery_charging);
    ui->power_battery_level->setCurrentIndex(Settings::values.p_battery_level - 1);
    ui->network_wifi_status->setCurrentIndex(Settings::values.n_wifi_status);
    ui->network_link_level->setCurrentIndex(Settings::values.n_wifi_link_level);
    ui->network_state->setCurrentIndex(SharedPageUtil::nsti(Settings::values.n_state));

    connect(ui->shared_page_enable_3d, &QCheckBox::stateChanged, this,
            &ControlPanel::On3DEnabledChanged);
    connect(ui->power_adapter_connected, &QCheckBox::stateChanged, this,
            &ControlPanel::OnAdapterConnectedChanged);
    connect(ui->power_battery_charging, &QCheckBox::stateChanged, this,
            &ControlPanel::OnBatteryChargingChanged);
    connect(ui->power_battery_level,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ControlPanel::OnBatteryLevelChanged);
    connect(ui->network_wifi_status,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ControlPanel::OnWifiStatusChanged);
    connect(ui->network_link_level,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ControlPanel::OnWifiLinkLevelChanged);
    connect(ui->network_state,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ControlPanel::OnNetworkStateChanged);

    setWindowTitle("Control Panel");
    setFixedSize(size());
}

ControlPanel::~ControlPanel() {}

void ControlPanel::On3DEnabledChanged() {
    SharedPage::shared_page.ledstate_3d = Settings::values.sp_enable_3d ? 1 : 0;
    Settings::values.sp_enable_3d = ui->shared_page_enable_3d->isChecked();
}

void ControlPanel::OnAdapterConnectedChanged() {
    SharedPage::shared_page.battery_state.is_adapter_connected.Assign(
        static_cast<u8>(Settings::values.p_adapter_connected));
    Settings::values.p_adapter_connected = ui->power_adapter_connected->isChecked();
}

void ControlPanel::OnBatteryChargingChanged() {
    SharedPage::shared_page.battery_state.is_charging.Assign(
        static_cast<u8>(Settings::values.p_battery_charging));
    Settings::values.p_battery_charging = ui->power_battery_charging->isChecked();
}

void ControlPanel::OnBatteryLevelChanged() {
    SharedPage::shared_page.battery_state.charge_level.Assign(
        static_cast<u8>(Settings::values.p_battery_level));
    Settings::values.p_battery_level =
        static_cast<u32>(ui->power_battery_level->currentIndex() + 1);
}

void ControlPanel::OnWifiStatusChanged() {
    Settings::values.n_wifi_status = ui->network_wifi_status->currentIndex();
}

void ControlPanel::OnWifiLinkLevelChanged() {
    SharedPage::SetWifiLinkLevel(
        static_cast<SharedPage::WifiLinkLevel>(ui->network_link_level->currentIndex()));
    Settings::values.n_wifi_link_level = static_cast<u8>(ui->network_link_level->currentIndex());
}

void ControlPanel::OnNetworkStateChanged() {
    SharedPage::shared_page.network_state = SharedPageUtil::itns(ui->network_state->currentIndex());
    Settings::values.n_state = SharedPageUtil::itns(ui->network_state->currentIndex());
}
