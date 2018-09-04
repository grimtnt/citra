// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <cstddef>
#include <vector>
#include "common/common_types.h"
#include "common/swap.h"
#include "core/hle/service/service.h"

// Local-WLAN service

namespace Service::NWM {

const size_t ApplicationDataSize = 0xC8;
const u8 DefaultNetworkChannel = 11;

// Number of milliseconds in a TU.
const double MillisecondsPerTU = 1.024;
// Interval measured in TU, the default value is 100TU = 102.4ms
const u16 DefaultBeaconInterval = 100;

/// The maximum number of nodes that can exist in an UDS session.
constexpr u32 UDSMaxNodes = 16;

struct NodeInfo {
    u64_le friend_code_seed;
    std::array<u16_le, 10> username;
    INSERT_PADDING_BYTES(4);
    u16_le network_node_id;
    INSERT_PADDING_BYTES(6);
};

static_assert(sizeof(NodeInfo) == 40, "NodeInfo has incorrect size.");

using NodeList = std::vector<NodeInfo>;

enum class NetworkStatus {
    NotConnected = 3,
    ConnectedAsHost = 6,
    Connecting = 7,
    ConnectedAsClient = 9,
    ConnectedAsSpectator = 10,
};

struct ConnectionStatus {
    u32_le status;
    INSERT_PADDING_WORDS(1);
    u16_le network_node_id;
    u16_le changed_nodes;
    u16_le nodes[UDSMaxNodes];
    u8 total_nodes;
    u8 max_nodes;
    u16_le node_bitmask;
};

static_assert(sizeof(ConnectionStatus) == 0x30, "ConnectionStatus has incorrect size.");

struct NetworkInfo {
    std::array<u8, 6> host_mac_address;
    u8 channel;
    INSERT_PADDING_BYTES(1);
    u8 initialized;
    INSERT_PADDING_BYTES(3);
    std::array<u8, 3> oui_value;
    u8 oui_type;
    // This field is received as BigEndian from the game.
    u32_be wlan_comm_id;
    u8 id;
    INSERT_PADDING_BYTES(1);
    u16_be attributes;
    u32_be network_id;
    u8 total_nodes;
    u8 max_nodes;
    INSERT_PADDING_BYTES(2);
    INSERT_PADDING_BYTES(0x1F);
    u8 application_data_size;
    std::array<u8, ApplicationDataSize> application_data;
};

static_assert(offsetof(NetworkInfo, oui_value) == 0xC, "oui_value is at the wrong offset.");
static_assert(offsetof(NetworkInfo, wlan_comm_id) == 0x10, "wlancommid is at the wrong offset.");
static_assert(sizeof(NetworkInfo) == 0x108, "NetworkInfo has incorrect size.");

/// Additional block tag ids in the Beacon and Association Response frames
enum class TagId : u8 {
    SSID = 0,
    SupportedRates = 1,
    DSParameterSet = 2,
    TrafficIndicationMap = 5,
    CountryInformation = 7,
    ERPInformation = 42,
    VendorSpecific = 221
};

/// A class that serializes data for network transfer. It also handles endianess
class NetworkPacket {
public:
    NetworkPacket() = default;
    ~NetworkPacket() = default;

    /**
     * Append data to the end of the packet
     * @param data        Pointer to the sequence of bytes to append
     * @param size_in_bytes Number of bytes to append
     */
    void Append(const void* data, std::size_t size_in_bytes);

    /**
     * Reads data from the current read position of the packet
     * @param out_data        Pointer where the data should get written to
     * @param size_in_bytes Number of bytes to read
     */
    void Read(void* out_data, std::size_t size_in_bytes);

    /**
     * Clear the packet
     * After calling Clear, the packet is empty.
     */
    void Clear();

    /**
     * Ignores bytes while reading
     * @param length THe number of bytes to ignore
     */
    void IgnoreBytes(u32 length);

    /**
     * Get a pointer to the data contained in the packet
     * @return Pointer to the data
     */
    const void* GetData() const;

    /**
     * This function returns the number of bytes pointed to by
     * what getData returns.
     * @return Data size, in bytes
     */
    std::size_t GetDataSize() const;

    /**
     * This function is useful to know if there is some data
     * left to be read, without actually reading it.
     * @return True if all data was read, false otherwise
     */
    bool EndOfPacket() const;

    explicit operator bool() const;

    /// Overloads of operator >> to read data from the packet
    NetworkPacket& operator>>(bool& out_data);
    NetworkPacket& operator>>(s8& out_data);
    NetworkPacket& operator>>(u8& out_data);
    NetworkPacket& operator>>(s16& out_data);
    NetworkPacket& operator>>(u16& out_data);
    NetworkPacket& operator>>(s32& out_data);
    NetworkPacket& operator>>(u32& out_data);
    NetworkPacket& operator>>(s64& out_data);
    NetworkPacket& operator>>(u64& out_data);
    NetworkPacket& operator>>(float& out_data);
    NetworkPacket& operator>>(double& out_data);
    NetworkPacket& operator>>(char* out_data);
    NetworkPacket& operator>>(std::string& out_data);
    template <typename T>
    NetworkPacket& operator>>(std::vector<T>& out_data);
    template <typename T, std::size_t S>
    NetworkPacket& operator>>(std::array<T, S>& out_data);

    /// Overloads of operator << to write data into the packet
    NetworkPacket& operator<<(bool in_data);
    NetworkPacket& operator<<(s8 in_data);
    NetworkPacket& operator<<(u8 in_data);
    NetworkPacket& operator<<(s16 in_data);
    NetworkPacket& operator<<(u16 in_data);
    NetworkPacket& operator<<(s32 in_data);
    NetworkPacket& operator<<(u32 in_data);
    NetworkPacket& operator<<(s64 in_data);
    NetworkPacket& operator<<(u64 in_data);
    NetworkPacket& operator<<(float in_data);
    NetworkPacket& operator<<(double in_data);
    NetworkPacket& operator<<(const char* in_data);
    NetworkPacket& operator<<(const std::string& in_data);
    template <typename T>
    NetworkPacket& operator<<(const std::vector<T>& in_data);
    template <typename T, std::size_t S>
    NetworkPacket& operator<<(const std::array<T, S>& data);

private:
    /**
     * Check if the packet can extract a given number of bytes
     * This function updates accordingly the state of the packet.
     * @param size Size to check
     * @return True if size bytes can be read from the packet
     */
    bool CheckSize(std::size_t size);

    // Member data
    std::vector<char> data; ///< Data stored in the packet
    std::size_t read_pos{}; ///< Current reading position in the packet
    bool is_valid = true;   ///< Reading state of the packet
};

template <typename T>
NetworkPacket& NetworkPacket::operator>>(std::vector<T>& out_data) {
    // First extract the size
    u32 size{};
    *this >> size;
    out_data.resize(size);

    // Then extract the data
    for (std::size_t i{}; i < out_data.size(); ++i) {
        T character{};
        *this >> character;
        out_data[i] = character;
    }
    return *this;
}

template <typename T, std::size_t S>
NetworkPacket& NetworkPacket::operator>>(std::array<T, S>& out_data) {
    for (std::size_t i{}; i < out_data.size(); ++i) {
        T character{};
        *this >> character;
        out_data[i] = character;
    }
    return *this;
}

template <typename T>
NetworkPacket& NetworkPacket::operator<<(const std::vector<T>& in_data) {
    // First insert the size
    *this << static_cast<u32>(in_data.size());

    // Then insert the data
    for (std::size_t i{}; i < in_data.size(); ++i) {
        *this << in_data[i];
    }
    return *this;
}

template <typename T, std::size_t S>
NetworkPacket& NetworkPacket::operator<<(const std::array<T, S>& in_data) {
    for (std::size_t i{}; i < in_data.size(); ++i) {
        *this << in_data[i];
    }
    return *this;
}
// NetworkPacket end

class NWM_UDS final : public ServiceFramework<NWM_UDS> {
public:
    NWM_UDS();
    ~NWM_UDS();

private:
    void UpdateNetworkAttribute(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::Shutdown service function
     *  Inputs:
     *      1 : None
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void Shutdown(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::DestroyNetwork service function.
     * Closes the network that we're currently hosting.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void DestroyNetwork(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::DisconnectNetwork service function.
     * This disconnects this device from the network.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void DisconnectNetwork(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::GetConnectionStatus service function.
     * Returns the connection status structure for the currently open network connection.
     * This structure contains information about the connection,
     * like the number of connected nodes, etc.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2-13 : Channel of the current WiFi network connection.
     */
    void GetConnectionStatus(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::GetNodeInformation service function.
     * Returns the node inforamtion structure for the currently connected node.
     *  Inputs:
     *      1 : Node ID.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2-11 : NodeInfo structure.
     */
    void GetNodeInformation(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::RecvBeaconBroadcastData service function
     * Returns the raw beacon data for nearby networks that match the supplied WlanCommId.
     *  Inputs:
     *      1 : Output buffer max size
     *    2-3 : Unknown
     *    4-5 : Host MAC address.
     *   6-14 : Unused
     *     15 : WLan Comm Id
     *     16 : Id
     *     17 : Value 0
     *     18 : Input handle
     *     19 : (Size<<4) | 12
     *     20 : Output buffer ptr
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2, 3: output buffer return descriptor & ptr
     */
    void RecvBeaconBroadcastData(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::SetApplicationData service function.
     * Updates the application data that is being broadcast in the beacon frames
     * for the network that we're hosting.
     *  Inputs:
     *      1 : Data size.
     *      3 : VAddr of the data.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Channel of the current WiFi network connection.
     */
    void SetApplicationData(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::Bind service function.
     * Binds a BindNodeId to a data channel and retrieves a data event.
     *  Inputs:
     *      1 : BindNodeId
     *      2 : Receive buffer size.
     *      3 : u8 Data channel to bind to.
     *      4 : Network node id.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Copy handle descriptor.
     *      3 : Data available event handle.
     */
    void Bind(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::Unbind service function.
     * Unbinds a BindNodeId from a data channel.
     *  Inputs:
     *      1 : BindNodeId
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void Unbind(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::PullPacket service function.
     * Receives a data frame from the specified bind node id
     *  Inputs:
     *      1 : Bind node id.
     *      2 : Max out buff size >> 2.
     *      3 : Max out buff size.
     *     64 : Output buffer descriptor
     *     65 : Output buffer address
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Received data size
     *      3 : u16 Source network node id
     *      4 : Buffer descriptor
     *      5 : Buffer address
     */
    void PullPacket(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::SendTo service function.
     * Sends a data frame to the UDS network we're connected to.
     *  Inputs:
     *      1 : Unknown.
     *      2 : u16 Destination network node id.
     *      3 : u8 Data channel.
     *      4 : Buffer size >> 2
     *      5 : Data size
     *      6 : Flags
     *      7 : Input buffer descriptor
     *      8 : Input buffer address
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void SendTo(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::GetChannel service function.
     * Returns the WiFi channel in which the network we're connected to is transmitting.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Channel of the current WiFi network connection.
     */
    void GetChannel(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::Initialize service function
     *  Inputs:
     *      1 : Shared memory size
     *   2-11 : Input NodeInfo Structure
     *     12 : 2-byte Version
     *     13 : Value 0
     *     14 : Shared memory handle
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2 : Value 0
     *      3 : Output event handle
     */
    void InitializeWithVersion(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::BeginHostingNetwork service function.
     * Creates a network and starts broadcasting its presence.
     *  Inputs:
     *      1 : Passphrase buffer size.
     *      3 : VAddr of the NetworkInfo structure.
     *      5 : VAddr of the passphrase.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void BeginHostingNetwork(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::ConnectToNetwork service function.
     * This connects to the specified network
     *  Inputs:
     *      1 : Connection type: 0x1 = Client, 0x2 = Spectator.
     *      2 : Passphrase buffer size
     *      3 : (NetworkStructSize<<12) | 0x402
     *      4 : Network struct buffer ptr
     *      5 : (PassphraseSize<<12) | 2
     *      6 : Input passphrase buffer ptr
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     */
    void ConnectToNetwork(Kernel::HLERequestContext& ctx);

    /**
     * NWM_UDS::DecryptBeaconData service function.
     * Decrypts the encrypted data tags contained in the 802.11 beacons.
     *  Inputs:
     *      1 : Input network struct buffer descriptor.
     *      2 : Input network struct buffer ptr.
     *      3 : Input tag0 encrypted buffer descriptor.
     *      4 : Input tag0 encrypted buffer ptr.
     *      5 : Input tag1 encrypted buffer descriptor.
     *      6 : Input tag1 encrypted buffer ptr.
     *     64 : Output buffer descriptor.
     *     65 : Output buffer ptr.
     *  Outputs:
     *      1 : Result of function, 0 on success, otherwise error code
     *      2, 3: output buffer return descriptor & ptr
     */
    void DecryptBeaconData(Kernel::HLERequestContext& ctx);
};

} // namespace Service::NWM
