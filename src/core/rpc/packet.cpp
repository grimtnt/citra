// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <cstring>
#include "core/rpc/packet.h"

namespace RPC {

Packet::Packet(const PacketHeader& header, u8* data,
               std::function<void(Packet&)> send_reply_callback)
    : header{header}, send_reply_callback{std::move(send_reply_callback)} {

    std::memcpy(packet_data.data(), data, std::min(header.packet_size, MAX_PACKET_DATA_SIZE));
}

}; // namespace RPC
