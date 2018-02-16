// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QDialog>

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QDialog {
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = 0);
    ~ControlPanel();

public slots:
    void OnAdapterConnectedChanged();
    void OnBatteryChargingChanged();
    void OnBatteryLevelChanged();
    void OnWifiStatusChanged();
    void OnWifiLinkLevelChanged();

private:
    Ui::ControlPanel *ui;
};
