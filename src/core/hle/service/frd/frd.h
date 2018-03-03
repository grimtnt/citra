// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/common_types.h"

namespace Service {

class Interface;

namespace FRD {

struct FriendKey {
    u32 friend_id;
    u32 padding;
    u64 friend_code;
    bool operator== (const FriendKey& other) const {
        return (friend_id == other.friend_id) && (friend_code == other.friend_code);
    }
};

struct MyPresence {
    u8 unknown[0x12C];
};

struct Profile {
    u8 region;
    u8 country;
    u8 area;
    u8 language;
    u32 unknown;
};

/**
 * FRD::GetMyPresence service function
 *  Inputs:
 *      64 : sizeof (MyPresence) << 14 | 2
 *      65 : Address of MyPresence structure
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void GetMyPresence(Service::Interface* self);

/**
 * FRD::GetMyPreference service function
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : Public mode (0 = private, non-zero = public) 
 *      3 : Show current game (0 = don't show, non-zero = show)
 *      4 : Show game history (0 = don't show, non-zero = show)
 */
void GetMyPreference(Service::Interface* self);

/**
 * FRD::GetFriendKeyList service function
 *  Inputs:
 *      1 : Unknown
 *      2 : Max friends count
 *      65 : Address of FriendKey List
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : FriendKey count filled
 */
void GetFriendKeyList(Service::Interface* self);

/**
 * FRD::GetFriendProfile service function
 *  Inputs:
 *      1 : Friends count
 *      2 : Friends count << 18 | 2
 *      3 : Address of FriendKey List
 *      64 : (count * sizeof (Profile)) << 10 | 2
 *      65 : Address of Profiles List
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void GetFriendProfile(Service::Interface* self);

/**
 * FRD::GetFriendAttributeFlags service function
 *  Inputs:
 *      1 : Friends count
 *      2 : Friends count << 18 | 2
 *      3 : Address of FriendKey List
 *      65 : Address of AttributeFlags
 */
void GetFriendAttributeFlags(Service::Interface* self);

/**
 * FRD::HasLoggedIn service function
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : Logged in, 0 if not logged in, 1 if logged in
 */
void HasLoggedIn(Service::Interface* self);

/**
 * FRD::IsOnline service function
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : Online state, 0 if not online, 1 if online
 */
void IsOnline(Service::Interface* self);

/**
 * FRD::Login service function
 *  Inputs:
 *      1 : 0x0
 *      2 : Completion event handle
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void Login(Service::Interface* self);

/**
 * FRD::Logout service function
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void Logout(Service::Interface* self);

/**
 * FRD::GetMyFriendKey service function
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2-5 : FriendKey
 */
void GetMyFriendKey(Service::Interface* self);

/**
 * FRD::GetMyScreenName service function
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : UTF16 encoded name (max 11 symbols)
 */
void GetMyScreenName(Service::Interface* self);

/**
 * FRD::IsValidFriendCode service function
 * Inputs:
 *      1-2 : Friend code
 * Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 *      2 : 1 if the provided friend code is valid, 0 otherwise 
 */
void IsValidFriendCode(Service::Interface* self);

/**
 * FRD::UnscrambleLocalFriendCode service function
 *  Inputs:
 *      1 : Friend code count
 *      2 : ((count * 12) << 14) | 0x402
 *      3 : Pointer to encoded friend codes. Each is 12 bytes large
 *      64 : ((count * 8) << 14) | 2
 *      65 : Pointer to write decoded local friend codes to. Each is 8 bytes large.
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void UnscrambleLocalFriendCode(Service::Interface* self);

/**
 * FRD::SetClientSdkVersion service function
 *  Inputs:
 *      1 : Used SDK Version
 *  Outputs:
 *      1 : Result of function, 0 on success, otherwise error code
 */
void SetClientSdkVersion(Service::Interface* self);

/// Initialize FRD service(s)
void Init();

/// Shutdown FRD service(s)
void Shutdown();

} // namespace FRD
} // namespace Service
