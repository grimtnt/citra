// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QCheckBox>
#include <QComboBox>
#include "citra_qt/control_panel.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_control_panel.h"

namespace SharedPageUtil {

static int NetworkStateToIndex(SharedPage::NetworkState state) {
    switch (state) {
    case SharedPage::NetworkState::Internet:
        return 3;
    case SharedPage::NetworkState::Local:
        return 2;
    case SharedPage::NetworkState::Disabled:
        return 1;
    case SharedPage::NetworkState::Enabled:
        return 0;
    default:
        UNREACHABLE();
    }
}

static SharedPage::NetworkState IndexToNetworkState(int index) {
    switch (index) {
    case 0:
        return SharedPage::NetworkState::Enabled;
    case 1:
        return SharedPage::NetworkState::Disabled;
    case 2:
        return SharedPage::NetworkState::Local;
    case 3:
        return SharedPage::NetworkState::Internet;
    }
    return SharedPage::NetworkState::Enabled;
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
    ui->network_state->setCurrentIndex(SharedPageUtil::NetworkStateToIndex(
        static_cast<SharedPage::NetworkState>(Settings::values.n_state)));

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

    setWindowTitle(tr("Control Panel"));
    setFixedSize(size());
}

ControlPanel::~ControlPanel() {}

void ControlPanel::On3DEnabledChanged() {
    Settings::values.sp_enable_3d = ui->shared_page_enable_3d->isChecked();
    Core::System::GetInstance().GetSharedPageHandler()->Set3DLed(Settings::values.sp_enable_3d ? 1
                                                                                               : 0);
}

void ControlPanel::OnAdapterConnectedChanged() {
    Settings::values.p_adapter_connected = ui->power_adapter_connected->isChecked();
    Core::System::GetInstance().GetSharedPageHandler()->SetAdapterConnected(
        static_cast<u8>(Settings::values.p_adapter_connected));
}

void ControlPanel::OnBatteryChargingChanged() {
    Settings::values.p_battery_charging = ui->power_battery_charging->isChecked();
    Core::System::GetInstance().GetSharedPageHandler()->SetBatteryCharging(
        static_cast<u8>(Settings::values.p_battery_charging));
}

void ControlPanel::OnBatteryLevelChanged() {
    Settings::values.p_battery_level =
        static_cast<u32>(ui->power_battery_level->currentIndex() + 1);
    Core::System::GetInstance().GetSharedPageHandler()->SetBatteryLevel(
        static_cast<u8>(Settings::values.p_battery_level));
}

void ControlPanel::OnWifiStatusChanged() {
    Settings::values.n_wifi_status = ui->network_wifi_status->currentIndex();
}

void ControlPanel::OnWifiLinkLevelChanged() {
    Settings::values.n_wifi_link_level = static_cast<u8>(ui->network_link_level->currentIndex());
    Core::System::GetInstance().GetSharedPageHandler()->SetWifiLinkLevel(
        static_cast<SharedPage::WifiLinkLevel>(Settings::values.n_wifi_link_level));
}

void ControlPanel::OnNetworkStateChanged() {
    Settings::values.n_state =
        static_cast<u8>(SharedPageUtil::IndexToNetworkState(ui->network_state->currentIndex()));
    Core::System::GetInstance().GetSharedPageHandler()->SetNetworkState(
        static_cast<SharedPage::NetworkState>(Settings::values.n_state));
}
