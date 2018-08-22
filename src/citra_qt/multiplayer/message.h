// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <utility>

namespace NetworkMessage {

class ConnectionError {

public:
    explicit ConnectionError(std::string str) : err(std::move(str)) {}
    const std::string& GetString() const {
        return err;
    }

private:
    std::string err;
};

extern const ConnectionError IP_ADDRESS_NOT_VALID;
extern const ConnectionError PORT_NOT_VALID;
extern const ConnectionError UNABLE_TO_CONNECT;
extern const ConnectionError COULD_NOT_CREATE_ROOM;
extern const ConnectionError LOST_CONNECTION;
extern const ConnectionError MAC_COLLISION;

/**
 *  Shows a standard QMessageBox with a error message
 */
void ShowError(const ConnectionError& e);

} // namespace NetworkMessage
