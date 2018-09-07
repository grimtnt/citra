// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include "common/threadsafe_queue.h"
#include "core/rpc/server.h"

namespace RPC {

class RPCServer {
public:
    RPCServer();
    ~RPCServer();

    void QueueRequest(std::unique_ptr<RPC::Packet> request);

private:
    void Start();
    void Stop();
    void HandleReadMemory(Packet& packet, u32 address, u32 data_size);
    void HandleWriteMemory(Packet& packet, u32 address, const u8* data, u32 data_size);
    void HandlePadState(Packet& packet, u32 raw);
    void HandleTouchState(Packet& packet, s16 x, s16 y, bool valid);
    void HandleMotionState(Packet& packet, s16 x, s16 y, s16 z, s16 roll, s16 pitch, s16 yaw);
    void HandleCircleState(Packet& packet, s16 x, s16 y);
    void HandleSetResolution(Packet& packet, u16 resolution);
    void HandleSetGame(Packet& packet, const std::string&);
    void HandleSetOverrideControls(Packet& packet, bool pad, bool touch, bool motion, bool circle);
    bool ValidatePacket(const PacketHeader& packet_header);
    void HandleSingleRequest(std::unique_ptr<Packet> request);
    void HandleRequestsLoop();

    Server server;
    Common::SPSCQueue<std::unique_ptr<Packet>> request_queue;
    bool running{};
    std::thread request_handler_thread;
    std::mutex request_queue_mutex;
    std::condition_variable request_queue_cv;
};

} // namespace RPC
