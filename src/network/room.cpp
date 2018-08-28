// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <atomic>
#include <iomanip>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>
#include "common/logging/log.h"
#include "enet/enet.h"
#include "network/packet.h"
#include "network/room.h"

namespace Network {

class Room::RoomImpl {
public:
    // This MAC address is used to generate a 'Nintendo' like Mac address.
    const MacAddress NintendoOUI;
    std::mt19937 random_gen; ///< Random number generator. Used for GenerateMacAddress

    ENetHost* server{}; ///< Network interface.

    std::atomic<State> state{State::Closed}; ///< Current state of the room.
    u16 port;                                ///< Port of this room.

    struct Member {
        MacAddress mac_address; ///< The assigned mac address of the member.
        ENetPeer* peer;         ///< The remote peer.
    };
    using MemberList = std::vector<Member>;
    MemberList members;              ///< Information about the members of this room
    mutable std::mutex member_mutex; ///< Mutex for locking the members list

    RoomImpl()
        : random_gen(std::random_device()()), NintendoOUI{0x00, 0x1F, 0x32, 0x00, 0x00, 0x00} {}

    /// Thread that receives and dispatches network packets
    std::unique_ptr<std::thread> room_thread;

    /// Thread function that will receive and dispatch messages until the room is destroyed.
    void ServerLoop();
    void StartLoop();

    /**
     * Parses and answers a room join request from a client.
     * Validates the uniqueness of the username and assigns the MAC address
     * that the client will use for the remainder of the connection.
     */
    void HandleJoinRequest(const ENetEvent* event);

    /**
     * Returns whether the MAC address is valid, ie. isn't already taken by someone else in the
     * room.
     */
    bool IsValidMacAddress(const MacAddress& address) const;

    /**
     * Sends a ID_ROOM_MAC_COLLISION message telling the client that the MAC is invalid.
     */
    void SendMacCollision(ENetPeer* client);

    /**
     * Notifies the member that its connection attempt was successful,
     * and it is now part of the room.
     */
    void SendJoinSuccess(ENetPeer* client, MacAddress mac_address);

    /**
     * Notifies the members that the room is closed,
     */
    void SendCloseMessage();

    /**
     * Generates a free MAC address to assign to a new client.
     * The first 3 bytes are the NintendoOUI 0x00, 0x1F, 0x32
     */
    MacAddress GenerateMacAddress();

    /**
     * Broadcasts this packet to all members except the sender.
     * @param event The ENet event containing the data
     */
    void HandleWifiPacket(const ENetEvent* event);

    /**
     * Removes the client from the members list if it was in it and announces the change
     * to all other clients.
     */
    void HandleClientDisconnection(ENetPeer* client);
};

// RoomImpl
void Room::RoomImpl::ServerLoop() {
    while (state != State::Closed) {
        ENetEvent event;
        if (enet_host_service(server, &event, 50) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                switch (event.packet->data[0]) {
                case IdJoinRequest:
                    HandleJoinRequest(&event);
                    break;
                case IdWifiPacket:
                    HandleWifiPacket(&event);
                    break;
                }
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                HandleClientDisconnection(event.peer);
                break;
            case ENET_EVENT_TYPE_NONE:
            case ENET_EVENT_TYPE_CONNECT:
                break;
            }
        }
    }
    // Close the connection to all members:
    SendCloseMessage();
}

void Room::RoomImpl::StartLoop() {
    room_thread = std::make_unique<std::thread>(&Room::RoomImpl::ServerLoop, this);
}

void Room::RoomImpl::HandleJoinRequest(const ENetEvent* event) {
    Packet packet;
    packet.Append(event->packet->data, event->packet->dataLength);
    packet.IgnoreBytes(sizeof(u8)); // Ignore the message type

    MacAddress preferred_mac;
    packet >> preferred_mac;

    if (preferred_mac != NoPreferredMac) {
        // Verify if the preferred mac is available
        if (!IsValidMacAddress(preferred_mac)) {
            SendMacCollision(event->peer);
            return;
        }
    } else {
        // Assign a MAC address of this client automatically
        preferred_mac = GenerateMacAddress();
    }

    // At this point the client is ready to be added to the room.
    Member member{};
    member.mac_address = preferred_mac;
    member.peer = event->peer;

    {
        std::lock_guard<std::mutex> lock(member_mutex);
        members.push_back(std::move(member));
    }

    SendJoinSuccess(event->peer, preferred_mac);
}

bool Room::RoomImpl::IsValidMacAddress(const MacAddress& address) const {
    // A MAC address is valid if it is not already taken by anybody else in the room.
    std::lock_guard<std::mutex> lock(member_mutex);
    return std::all_of(members.begin(), members.end(),
                       [&address](const auto& member) { return member.mac_address != address; });
}

void Room::RoomImpl::SendMacCollision(ENetPeer* client) {
    Packet packet;
    packet << static_cast<u8>(IdMacCollision);

    ENetPacket* enet_packet =
        enet_packet_create(packet.GetData(), packet.GetDataSize(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(client, 0, enet_packet);
    enet_host_flush(server);
}

void Room::RoomImpl::SendJoinSuccess(ENetPeer* client, MacAddress mac_address) {
    Packet packet;
    packet << static_cast<u8>(IdJoinSuccess);
    packet << mac_address;
    ENetPacket* enet_packet =
        enet_packet_create(packet.GetData(), packet.GetDataSize(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(client, 0, enet_packet);
    enet_host_flush(server);
}

void Room::RoomImpl::SendCloseMessage() {
    Packet packet;
    packet << static_cast<u8>(IdCloseRoom);
    std::lock_guard<std::mutex> lock(member_mutex);
    if (!members.empty()) {
        ENetPacket* enet_packet =
            enet_packet_create(packet.GetData(), packet.GetDataSize(), ENET_PACKET_FLAG_RELIABLE);
        for (auto& member : members) {
            enet_peer_send(member.peer, 0, enet_packet);
        }
    }
    enet_host_flush(server);
    for (auto& member : members) {
        enet_peer_disconnect(member.peer, 0);
    }
}

MacAddress Room::RoomImpl::GenerateMacAddress() {
    MacAddress result_mac =
        NintendoOUI; // The first three bytes of each MAC address will be the NintendoOUI
    std::uniform_int_distribution<> dis(0x00, 0xFF); // Random byte between 0 and 0xFF
    do {
        for (size_t i = 3; i < result_mac.size(); ++i) {
            result_mac[i] = dis(random_gen);
        }
    } while (!IsValidMacAddress(result_mac));
    return result_mac;
}

void Room::RoomImpl::HandleWifiPacket(const ENetEvent* event) {
    Packet in_packet;
    in_packet.Append(event->packet->data, event->packet->dataLength);
    in_packet.IgnoreBytes(sizeof(u8));         // Message type
    in_packet.IgnoreBytes(sizeof(u8));         // WifiPacket Type
    in_packet.IgnoreBytes(sizeof(u8));         // WifiPacket Channel
    in_packet.IgnoreBytes(sizeof(MacAddress)); // WifiPacket Transmitter Address
    MacAddress destination_address;
    in_packet >> destination_address;

    Packet out_packet;
    out_packet.Append(event->packet->data, event->packet->dataLength);
    ENetPacket* enet_packet = enet_packet_create(out_packet.GetData(), out_packet.GetDataSize(),
                                                 ENET_PACKET_FLAG_RELIABLE);

    if (destination_address == BroadcastMac) { // Send the data to everyone except the sender
        std::lock_guard<std::mutex> lock(member_mutex);
        bool sent_packet = false;
        for (const auto& member : members) {
            if (member.peer != event->peer) {
                sent_packet = true;
                enet_peer_send(member.peer, 0, enet_packet);
            }
        }

        if (!sent_packet) {
            enet_packet_destroy(enet_packet);
        }
    } else { // Send the data only to the destination client
        std::lock_guard<std::mutex> lock(member_mutex);
        auto member = std::find_if(members.begin(), members.end(),
                                   [destination_address](const Member& member) -> bool {
                                       return member.mac_address == destination_address;
                                   });
        if (member != members.end()) {
            enet_peer_send(member->peer, 0, enet_packet);
        } else {
            LOG_ERROR(Network,
                      "Attempting to send to unknown MAC address: "
                      "{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
                      destination_address[0], destination_address[1], destination_address[2],
                      destination_address[3], destination_address[4], destination_address[5]);
            enet_packet_destroy(enet_packet);
        }
    }
    enet_host_flush(server);
}

void Room::RoomImpl::HandleClientDisconnection(ENetPeer* client) {
    // Remove the client from the members list.
    {
        std::lock_guard<std::mutex> lock(member_mutex);
        members.erase(
            std::remove_if(members.begin(), members.end(),
                           [client](const Member& member) { return member.peer == client; }),
            members.end());
    }

    enet_peer_disconnect(client, 0);
}

// Room
Room::Room() : room_impl{std::make_unique<RoomImpl>()} {}

Room::~Room() = default;

bool Room::Create(u16 port) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    room_impl->server = enet_host_create(&address, 48, NumChannels, 0, 0);
    if (!room_impl->server) {
        return false;
    }
    room_impl->state = State::Open;
    room_impl->StartLoop();
    return true;
}

Room::State Room::GetState() const {
    return room_impl->state;
}

std::vector<Room::Member> Room::GetRoomMemberList() const {
    std::vector<Room::Member> member_list;
    std::lock_guard<std::mutex> lock(room_impl->member_mutex);
    for (const auto& member_impl : room_impl->members) {
        member_list.push_back(member_impl.mac_address);
    }
    return member_list;
}

void Room::Destroy() {
    room_impl->state = State::Closed;
    room_impl->room_thread->join();
    room_impl->room_thread.reset();

    if (room_impl->server) {
        enet_host_destroy(room_impl->server);
    }
    room_impl->server = nullptr;
    {
        std::lock_guard<std::mutex> lock(room_impl->member_mutex);
        room_impl->members.clear();
    }
}

} // namespace Network
