// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <ctime>
#include "common/common_types.h"
#include "core/frontend/discord.h"

namespace DiscordRPC {

s64 g_start_time;
bool g_send_presence = true;

static void HandleDiscordReady(const DiscordUser* request) {}

static void HandleDiscordDisconnected(int errorCode, const char* message) {}

static void HandleDiscordError(int errorCode, const char* message) {}

static void HandleDiscordJoin(const char* joinSecret) {}

static void HandleDiscordSpectate(const char* spectateSecret) {}

static void HandleDiscordJoinRequest(const DiscordUser* request) {
    Discord_Respond(request->userId, DISCORD_REPLY_NO);
}

void Init() {
    start_time = time(NULL);
    send_presence = true;
    DiscordEventHandlers handlers{};
    handlers.ready = HandleDiscordReady;
    handlers.disconnected = HandleDiscordDisconnected;
    handlers.errored = HandleDiscordError;
    handlers.joinGame = HandleDiscordJoin;
    handlers.spectateGame = HandleDiscordSpectate;
    handlers.joinRequest = HandleDiscordJoinRequest;
    Discord_Initialize("432553040755556374", &handlers, 1, NULL);
}

void Shutdown() {
    g_send_presence = false;
    Discord_ClearPresence();
    Discord_Shutdown();
}

void Update(const std::string& title) {
    if (g_send_presence) {
        DiscordRichPresence discordPresence{};
        discordPresence.state = title.empty() ? "Unknown game" : title.c_str();
        discordPresence.details = "Playing";
        discordPresence.startTimestamp = g_start_time;
        discordPresence.largeImageKey = "citra-logo";
        Discord_UpdatePresence(&discordPresence);
    } else {
        Discord_ClearPresence();
    }
}

} // namespace DiscordRPC
