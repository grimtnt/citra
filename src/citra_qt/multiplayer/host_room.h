// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <QDialog>
#include "citra_qt/multiplayer/validation.h"
#include "network/network.h"

namespace Ui {
class HostRoom;
} // namespace Ui

class ConnectionError;
class ComboBoxProxyModel;

class ChatMessage;

class HostRoomWindow : public QDialog {
    Q_OBJECT

public:
    explicit HostRoomWindow(QWidget* parent);
    ~HostRoomWindow();

private slots:
    /**
     * Handler for connection status changes. Launches the chat window if successful or
     * displays an error
     */
    void OnConnection();

private:
    void Host();

    std::unique_ptr<Ui::HostRoom> ui;
    Validation validation;
};
