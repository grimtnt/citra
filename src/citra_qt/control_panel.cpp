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
    ui->adapter_connected->setChecked(Settings::values.ptm_values.adapter_connected);
    ui->battery_charging->setChecked(Settings::values.ptm_values.battery_charging);
    ui->battery_level->setCurrentIndex(Settings::values.ptm_values.battery_level - 1);
    connect(ui->adapter_connected, &QCheckBox::stateChanged, this, &ControlPanel::OnAdapterConnectedChanged);
    connect(ui->battery_charging, &QCheckBox::stateChanged, this, &ControlPanel::OnBatteryChargingChanged);
    connect(ui->battery_level, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ControlPanel::OnBatteryLevelChanged);
    setWindowTitle("Control Panel");
}

ControlPanel::~ControlPanel() {
    delete ui;
}

void ControlPanel::OnAdapterConnectedChanged() {
    Settings::values.ptm_values.adapter_connected = ui->adapter_connected->isChecked();
}

void ControlPanel::OnBatteryChargingChanged() {
    Settings::values.ptm_values.battery_charging = ui->battery_charging->isChecked();
}

void ControlPanel::OnBatteryLevelChanged() {
    Settings::values.ptm_values.battery_level = ui->battery_level->currentIndex() + 1;
}
