// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QAction>
#include <QIcon>
#include <QMessageBox>
#include <QStandardItemModel>
#include "citra_qt/game_list.h"
#include "citra_qt/multiplayer/client_room.h"
#include "citra_qt/multiplayer/host_room.h"
#include "citra_qt/multiplayer/ip_connect.h"
#include "citra_qt/multiplayer/message.h"
#include "citra_qt/multiplayer/state.h"
#include "citra_qt/util/clickable_label.h"
#include "common/logging/log.h"

MultiplayerState::MultiplayerState(QWidget* parent, QStandardItemModel* game_list_model,
                                   QAction* leave_room, QAction* show_room)
    : QWidget(parent), game_list_model(game_list_model), leave_room(leave_room),
      show_room(show_room) {
    if (auto member = Network::GetRoomMember().lock()) {
        // register the network structs to use in slots and signals
        state_callback_handle = member->BindOnStateChanged(
            [this](const Network::RoomMember::State& state) { emit NetworkStateChanged(state); });
        connect(this, &MultiplayerState::NetworkStateChanged, this,
                &MultiplayerState::OnNetworkStateChanged);
    }

    qRegisterMetaType<Network::RoomMember::State>();

    status_icon = new ClickableLabel(this);
    status_icon->setPixmap(QIcon::fromTheme("disconnected").pixmap(16));

    connect(status_icon, &ClickableLabel::clicked, this, &MultiplayerState::OnOpenNetworkRoom);
}

MultiplayerState::~MultiplayerState() {
    if (state_callback_handle) {
        if (auto member = Network::GetRoomMember().lock()) {
            member->Unbind(state_callback_handle);
        }
    }
}

void MultiplayerState::Close() {
    if (host_room)
        host_room->close();
    if (ip_connect)
        ip_connect->close();
    if (client_room)
        client_room->close();
}

void MultiplayerState::UpdateThemedIcons() {
    if (current_state == Network::RoomMember::State::Joined) {
        status_icon->setPixmap(QIcon::fromTheme("connected").pixmap(16));
    } else {
        status_icon->setPixmap(QIcon::fromTheme("disconnected").pixmap(16));
    }
}

void MultiplayerState::OnNetworkStateChanged(const Network::RoomMember::State& state) {
    LOG_DEBUG(Frontend, "Network State: {}", Network::GetStateStr(state));
    bool is_connected = false;
    switch (state) {
    case Network::RoomMember::State::LostConnection:
        NetworkMessage::ShowError(NetworkMessage::LOST_CONNECTION);
        break;
    case Network::RoomMember::State::CouldNotConnect:
        NetworkMessage::ShowError(NetworkMessage::UNABLE_TO_CONNECT);
        break;
    case Network::RoomMember::State::NameCollision:
        NetworkMessage::ShowError(NetworkMessage::USERNAME_IN_USE);
        break;
    case Network::RoomMember::State::MacCollision:
        NetworkMessage::ShowError(NetworkMessage::MAC_COLLISION);
        break;
    case Network::RoomMember::State::WrongPassword:
        NetworkMessage::ShowError(NetworkMessage::WRONG_PASSWORD);
        break;
    case Network::RoomMember::State::Error:
        NetworkMessage::ShowError(NetworkMessage::UNABLE_TO_CONNECT);
        break;
    case Network::RoomMember::State::Joined:
        is_connected = true;
        OnOpenNetworkRoom();
        break;
    }
    if (is_connected) {
        status_icon->setPixmap(QIcon::fromTheme("connected").pixmap(16));
        leave_room->setEnabled(true);
        show_room->setEnabled(true);
    } else {
        status_icon->setPixmap(QIcon::fromTheme("disconnected").pixmap(16));
        leave_room->setEnabled(false);
        show_room->setEnabled(false);
    }

    current_state = state;
}

static void BringWidgetToFront(QWidget* widget) {
    widget->show();
    widget->activateWindow();
    widget->raise();
}

void MultiplayerState::OnCreateRoom() {
    if (host_room == nullptr) {
        host_room = new HostRoomWindow(this, game_list_model);
    }
    BringWidgetToFront(host_room);
}

bool MultiplayerState::OnCloseRoom() {
    if (!NetworkMessage::WarnCloseRoom())
        return false;
    if (auto room = Network::GetRoom().lock()) {
        // if you are in a room, leave it
        if (auto member = Network::GetRoomMember().lock()) {
            member->Leave();
            LOG_DEBUG(Frontend, "Left the room (as a client)");
        }

        // if you are hosting a room, also stop hosting
        if (room->GetState() != Network::Room::State::Open) {
            return true;
        }
        room->Destroy();
        LOG_DEBUG(Frontend, "Closed the room (as a server)");
    }
    return true;
}

void MultiplayerState::OnOpenNetworkRoom() {
    if (auto member = Network::GetRoomMember().lock()) {
        if (member->IsConnected()) {
            if (client_room == nullptr) {
                client_room = new ClientRoomWindow(this);
            }
            BringWidgetToFront(client_room);
            return;
        }
    }
}

void MultiplayerState::OnIpConnectToRoom() {
    if (ip_connect == nullptr) {
        ip_connect = new IpConnectWindow(this);
    }
    BringWidgetToFront(ip_connect);
}
