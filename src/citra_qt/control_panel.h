// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QDialog>
#include "common/common_types.h"

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QDialog {
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = 0);
    ~ControlPanel();

public slots:
    int nsti(u8 state);
    u8 itns(int index);
    void OnAdapterConnectedChanged();
    void OnBatteryChargingChanged();
    void OnBatteryLevelChanged();
    void OnWifiStatusChanged();
    void OnWifiLinkLevelChanged();
    void OnNetworkStateChanged();

private:
    Ui::ControlPanel *ui;
};
