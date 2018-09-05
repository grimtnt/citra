// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>
#include "common/common_types.h"

namespace Network {

constexpr u16 DefaultRoomPort{24872};

constexpr std::size_t NumChannels{1}; // Number of channels used for the connection

using MacAddress = std::array<u8, 6>;
/// A special MAC address that tells the room we're joining to assign us a MAC address
/// automatically.
constexpr MacAddress NoPreferredMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// 802.11 broadcast MAC address
constexpr MacAddress BroadcastMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// The different types of messages that can be sent. The first byte of each packet defines the type
enum RoomMessageTypes : u8 {
    IdJoinRequest = 1,
    IdJoinSuccess,
    IdWifiPacket,
    IdMacCollision,
    IdCloseRoom
};

/// This is what a server [person creating a server] would use.
class Room final {
public:
    enum class State : u8 {
        Open,   ///< The room is open and ready to accept connections.
        Closed, ///< The room is not opened and can not accept connections.
    };

    Room();
    ~Room();

    /**
     * Gets the current state of the room.
     */
    State GetState() const;

    /**
     * Creates the socket for this room. Will bind to default address if
     * server is empty string.
     */
    bool Create(u16 port = DefaultRoomPort);

    /**
     * Destroys the socket
     */
    void Destroy();

private:
    class RoomImpl;
    std::unique_ptr<RoomImpl> room_impl;
};

} // namespace Network
