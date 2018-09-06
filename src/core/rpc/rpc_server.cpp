#include "common/logging/log.h"
#include "core/core.h"
#include "core/cpu/cpu.h"
#include "core/hle/service/hid/hid.h"
#include "core/memory.h"
#include "core/rpc/packet.h"
#include "core/rpc/rpc_server.h"

namespace RPC {

RPCServer::RPCServer() : server(*this) {
    LOG_INFO(RPC_Server, "Starting RPC server ...");

    Start();

    LOG_INFO(RPC_Server, "RPC started.");
}

RPCServer::~RPCServer() {
    LOG_INFO(RPC_Server, "Stopping RPC ...");

    Stop();

    LOG_INFO(RPC_Server, "RPC stopped.");
}

void RPCServer::HandleReadMemory(Packet& packet, u32 address, u32 data_size) {
    if (data_size > MAX_READ_SIZE) {
        return;
    }

    // Note: Memory read occurs asynchronously from the state of the emulator
    Memory::ReadBlock(address, packet.GetPacketData().data(), data_size);
    packet.SetPacketDataSize(data_size);
    packet.SendReply();
}

void RPCServer::HandleWriteMemory(Packet& packet, u32 address, const u8* data, u32 data_size) {
    // Only allow writing to certain memory regions
    if ((address >= Memory::PROCESS_IMAGE_VADDR && address <= Memory::PROCESS_IMAGE_VADDR_END) ||
        (address >= Memory::HEAP_VADDR && address <= Memory::HEAP_VADDR_END) ||
        (address >= Memory::N3DS_EXTRA_RAM_VADDR && address <= Memory::N3DS_EXTRA_RAM_VADDR_END)) {
        // Note: Memory write occurs asynchronously from the state of the emulator
        Memory::WriteBlock(address, data, data_size);
        // If the memory happens to be executable code, make sure the changes become visible
        Core::GetCPU().InvalidateCacheRange(address, data_size);
    }
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

void RPCServer::HandlePadState(Packet& packet, u32 raw) {
    Service::HID::SetPadState(raw);
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

void RPCServer::HandleTouchState(Packet& packet, s16 x, s16 y, bool valid) {
    Service::HID::SetTouchState(x, y, valid);
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

void RPCServer::HandleMotionState(Packet& packet, s16 x, s16 y, s16 z, s16 roll, s16 pitch,
                                  s16 yaw) {
    Service::HID::SetMotionState(x, y, z, roll, pitch, yaw);
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

void RPCServer::HandleCircleState(Packet& packet, s16 x, s16 y) {
    Service::HID::SetCircleState(x, y);
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

void RPCServer::HandleSetResolution(Packet& packet, u16 resolution) {
    Settings::values.resolution_factor = resolution;
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

void RPCServer::HandleSetGame(Packet& packet, const std::string& path) {
    packet.SetPacketDataSize(0);
    packet.SendReply();
    Core::System::GetInstance().SetGame(path);
}

void RPCServer::HandleSetOverrideControls(Packet& packet, bool pad, bool touch, bool motion,
                                          bool circle) {
    Service::HID::SetOverrideControls(pad, touch, motion, circle);
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

bool RPCServer::ValidatePacket(const PacketHeader& packet_header) {
    if (packet_header.version <= CURRENT_VERSION) {
        switch (packet_header.packet_type) {
        case PacketType::ReadMemory:
        case PacketType::WriteMemory:
        case PacketType::PadState:
        case PacketType::TouchState:
        case PacketType::MotionState:
        case PacketType::CircleState:
        case PacketType::SetResolution:
        case PacketType::SetGame:
        case PacketType::SetOverrideControls:
            if (packet_header.packet_size >= (sizeof(u32) * 2)) {
                return true;
            }
            break;
        default:
            break;
        }
    }
    return false;
}

void RPCServer::HandleSingleRequest(std::unique_ptr<Packet> request_packet) {
    bool success{};

    if (ValidatePacket(request_packet->GetHeader())) {
        // Currently, all request types use the address/data_size wire format
        u32 address{};
        u32 data_size{};
        std::memcpy(&address, request_packet->GetPacketData().data(), sizeof(address));
        std::memcpy(&data_size, request_packet->GetPacketData().data() + sizeof(address),
                    sizeof(data_size));
        switch (request_packet->GetPacketType()) {
        case PacketType::ReadMemory: {
            if (data_size > 0 && data_size <= MAX_READ_SIZE) {
                HandleReadMemory(*request_packet, address, data_size);
                success = true;
            }
            break;
        }
        case PacketType::WriteMemory: {
            if (data_size > 0 && data_size <= MAX_PACKET_DATA_SIZE - (sizeof(u32) * 2)) {
                const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
                HandleWriteMemory(*request_packet, address, data, data_size);
                success = true;
            }
            break;
        }
        case PacketType::PadState: {
            const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
            u32 raw{};
            std::memcpy(&raw, data, sizeof(u32));
            HandlePadState(*request_packet, raw);
            success = true;
            break;
        }
        case PacketType::TouchState: {
            const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
            struct State {
                s16 x;
                s16 y;
                bool valid;
            };
            State state{};
            std::memcpy(&state, data, sizeof(State));
            HandleTouchState(*request_packet, state.x, state.y, state.valid);
            success = true;
            break;
        }
        case PacketType::MotionState: {
            const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
            struct State {
                s16 x;
                s16 y;
                s16 z;
                s16 roll;
                s16 pitch;
                s16 yaw;
            };
            State state{};
            std::memcpy(&state, data, sizeof(State));
            HandleMotionState(*request_packet, state.x, state.y, state.z, state.roll, state.pitch,
                              state.yaw);
            success = true;
            break;
        }
        case PacketType::CircleState: {
            const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
            struct State {
                s16 x;
                s16 y;
            };
            State state{};
            std::memcpy(&state, data, sizeof(State));
            HandleCircleState(*request_packet, state.x, state.y);
            success = true;
            break;
        }
        case PacketType::SetResolution: {
            const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
            u16 resolution{};
            std::memcpy(&resolution, data, sizeof(u16));
            HandleSetResolution(*request_packet, resolution);
            success = true;
            break;
        }
        case PacketType::SetGame: {
            const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
            std::string path{};
            path.resize(request_packet->GetPacketDataSize() - (sizeof(u32) * 2));
            std::memcpy(&path[0], data, request_packet->GetPacketDataSize() - (sizeof(u32) * 2));
            HandleSetGame(*request_packet, path);
            success = true;
            break;
        }
        case PacketType::SetOverrideControls: {
            const u8* data{request_packet->GetPacketData().data() + (sizeof(u32) * 2)};
            struct State {
                bool pad;
                bool touch;
                bool motion;
                bool circle;
            };
            State state{};
            std::memcpy(&state, data, sizeof(State));
            HandleSetOverrideControls(*request_packet, state.pad, state.touch, state.motion,
                                      state.circle);
            success = true;
            break;
        }
        default:
            break;
        }
    }

    if (!success) {
        // Send an empty reply, so as not to hang the client
        request_packet->SetPacketDataSize(0);
        request_packet->SendReply();
    }
}

void RPCServer::HandleRequestsLoop() {
    std::unique_ptr<RPC::Packet> request_packet{};

    LOG_INFO(RPC_Server, "Request handler started.");

    for (;;) {
        std::unique_lock<std::mutex> lock{request_queue_mutex};
        request_queue_cv.wait(lock, [&] { return !running || request_queue.Pop(request_packet); });
        if (!running) {
            break;
        }
        HandleSingleRequest(std::move(request_packet));
    }
}

void RPCServer::QueueRequest(std::unique_ptr<RPC::Packet> request) {
    std::unique_lock<std::mutex> lock{request_queue_mutex};
    request_queue.Push(std::move(request));
    request_queue_cv.notify_one();
}

void RPCServer::Start() {
    running = true;
    const auto threadFunction{[this]() { HandleRequestsLoop(); }};
    request_handler_thread = std::thread(threadFunction);
    server.Start();
}

void RPCServer::Stop() {
    running = false;
    request_queue_cv.notify_one();
    request_handler_thread.join();
    server.Stop();
}

}; // namespace RPC
