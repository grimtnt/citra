#include "common/logging/log.h"
#include "core/arm/arm_interface.h"
#include "core/core.h"
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
        Core::CPU().InvalidateCacheRange(address, data_size);
    }
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

void RPCServer::HandlePadState(Packet& packet, u32 raw) {
    Service::HID::SetPadState(raw);
    packet.SetPacketDataSize(0);
    packet.SendReply();
}

bool RPCServer::ValidatePacket(const PacketHeader& packet_header) {
    if (packet_header.version <= CURRENT_VERSION) {
        switch (packet_header.packet_type) {
        case PacketType::ReadMemory:
        case PacketType::WriteMemory:
        case PacketType::PadState:
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
    bool success = false;

    if (ValidatePacket(request_packet->GetHeader())) {
        // Currently, all request types use the address/data_size wire format
        u32 address = 0;
        u32 data_size = 0;
        std::memcpy(&address, request_packet->GetPacketData().data(), sizeof(address));
        std::memcpy(&data_size, request_packet->GetPacketData().data() + sizeof(address),
                    sizeof(data_size));
        switch (request_packet->GetPacketType()) {
        case PacketType::ReadMemory:
            if (data_size > 0 && data_size <= MAX_READ_SIZE) {
                HandleReadMemory(*request_packet, address, data_size);
                success = true;
            }
            break;
        case PacketType::WriteMemory:
            if (data_size > 0 && data_size <= MAX_PACKET_DATA_SIZE - (sizeof(u32) * 2)) {
                const u8* data = request_packet->GetPacketData().data() + (sizeof(u32) * 2);
                HandleWriteMemory(*request_packet, address, data, data_size);
                success = true;
            }
            break;
        case PacketType::PadState:
            if (data_size > 0 && data_size <= MAX_PACKET_DATA_SIZE - (sizeof(u32) * 2)) {
                const u8* data = request_packet->GetPacketData().data() + (sizeof(u32) * 2);
                u32 raw;
                std::memcpy(&raw, data, sizeof(u32));
                HandlePadState(*request_packet, raw);
                success = true;
            }
            break;
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
    std::unique_ptr<RPC::Packet> request_packet;

    LOG_INFO(RPC_Server, "Request handler started.");

    while (true) {
        std::unique_lock<std::mutex> lock(request_queue_mutex);
        request_queue_cv.wait(lock, [&] { return !running || request_queue.Pop(request_packet); });
        if (!running) {
            break;
        }
        HandleSingleRequest(std::move(request_packet));
    }
}

void RPCServer::QueueRequest(std::unique_ptr<RPC::Packet> request) {
    std::unique_lock<std::mutex> lock(request_queue_mutex);
    request_queue.Push(std::move(request));
    request_queue_cv.notify_one();
}

void RPCServer::Start() {
    running = true;
    const auto threadFunction = [this]() { HandleRequestsLoop(); };
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
