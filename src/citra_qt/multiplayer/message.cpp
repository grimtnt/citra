// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QMessageBox>
#include <QString>
#include "citra_qt/multiplayer/message.h"

namespace NetworkMessage {

const ConnectionError IP_ADDRESS_NOT_VALID("IP is not a valid IPv4 address.");
const ConnectionError PORT_NOT_VALID("Port must be a number between 0 to 65535.");
const ConnectionError UNABLE_TO_CONNECT(
    "Unable to connect to the host. Verify that the connection settings are correct. If "
    "you still cannot connect, contact the room host and verify that the host is "
    "properly configured with the external port forwarded.");
const ConnectionError COULD_NOT_CREATE_ROOM(
    "Creating a room failed. Please retry. Restarting Citra might be necessary.");
const ConnectionError LOST_CONNECTION("Connection to room lost. Try to reconnect.");
const ConnectionError MAC_COLLISION("MAC address is already in use. Please choose another.");

void ShowError(const ConnectionError& e) {
    QMessageBox::critical(nullptr, "Error", QString::fromStdString(e.GetString()));
}

} // namespace NetworkMessage
