// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include "network/network.h"

class QStandardItemModel;
class HostRoomWindow;
class IpConnectWindow;
class ClickableLabel;

class MultiplayerState : public QWidget {
    Q_OBJECT;

public:
    explicit MultiplayerState(QWidget* parent, QStandardItemModel* game_list, QAction* leave_room);
    ~MultiplayerState();

    /**
     * Close all open multiplayer related dialogs
     */
    void Close();

    QLabel* GetStatusIcon() const {
        return status_icon;
    }

    void SetCloseMs(int ms) {
        close_timer.setInterval(ms);
    }

    void StartTimer() {
        close_timer.start();
    }

public slots:
    void OnNetworkStateChanged(const Network::RoomMember::State& state);
    void OnCreateRoom();
    bool OnCloseRoom();
    void OnIpConnectToRoom();
    void UpdateThemedIcons();

signals:
    void NetworkStateChanged(const Network::RoomMember::State&);

private:
    HostRoomWindow* host_room{};
    IpConnectWindow* ip_connect{};
    QLabel* status_icon{};
    QStandardItemModel* game_list_model{};
    QTimer close_timer;
    QAction* leave_room;
    Network::RoomMember::State current_state = Network::RoomMember::State::Uninitialized;
    Network::RoomMember::CallbackHandle<Network::RoomMember::State> state_callback_handle;
};

Q_DECLARE_METATYPE(Network::RoomMember::State)
