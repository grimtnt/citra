// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QDialog>
#include <QFutureWatcher>
#include "citra_qt/multiplayer/validation.h"

namespace Ui {
class IpConnect;
} // namespace Ui

class IpConnectWindow : public QDialog {
    Q_OBJECT

public:
    explicit IpConnectWindow(QWidget* parent = nullptr);
    ~IpConnectWindow();

signals:
    /**
     * Signalled by this widget when it is closing itself and destroying any state such as
     * connections that it might have.
     */
    void Closed();

private slots:
    void OnConnection();

private:
    void Connect();
    void BeginConnecting();
    void EndConnecting();

    QFutureWatcher<void>* watcher;
    std::unique_ptr<Ui::IpConnect> ui;
    Validation validation;
};
