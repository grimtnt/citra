// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QString>

namespace NetworkMessage {

extern const QString IP_ADDRESS_NOT_VALID;
extern const QString PORT_NOT_VALID;
extern const QString UNABLE_TO_CONNECT;
extern const QString COULD_NOT_CREATE_ROOM;
extern const QString LOST_CONNECTION;
extern const QString MAC_COLLISION;

/**
 *  Shows a standard QMessageBox with a error message
 */
void ShowError(const QString& e);

} // namespace NetworkMessage
