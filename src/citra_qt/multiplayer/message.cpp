// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QMessageBox>
#include <QString>

#include "citra_qt/multiplayer/message.h"

namespace NetworkMessage {
const ConnectionError IP_ADDRESS_NOT_VALID(QT_TR_NOOP("IP is not a valid IPv4 address."));
const ConnectionError PORT_NOT_VALID(QT_TR_NOOP("Port must be a number between 0 to 65535."));
const ConnectionError UNABLE_TO_CONNECT(
    QT_TR_NOOP("Unable to connect to the host. Verify that the connection settings are correct. If "
               "you still cannot connect, contact the room host and verify that the host is "
               "properly configured with the external port forwarded."));
const ConnectionError COULD_NOT_CREATE_ROOM(
    QT_TR_NOOP("Creating a room failed. Please retry. Restarting Citra might be necessary."));
const ConnectionError LOST_CONNECTION(QT_TR_NOOP("Connection to room lost. Try to reconnect."));
const ConnectionError MAC_COLLISION(
    QT_TR_NOOP("MAC address is already in use. Please choose another."));

static bool WarnMessage(const std::string& title, const std::string& text) {
    return QMessageBox::Ok == QMessageBox::warning(nullptr, QObject::tr(title.c_str()),
                                                   QObject::tr(text.c_str()),
                                                   QMessageBox::Ok | QMessageBox::Cancel);
}

void ShowError(const ConnectionError& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr(e.GetString().c_str()));
}

} // namespace NetworkMessage
