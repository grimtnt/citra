// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <ctime>
#include <string>
#include <discord_rpc.h>
#include "common/common_types.h"

static s64 start_time;
static bool send_presence = true;

static void updateDiscordPresence(const std::string& title) {
    if (send_presence) {
        DiscordRichPresence discordPresence{};
        discordPresence.state = title.empty() ? "Unknown game" : title.c_str();
        discordPresence.details = "Playing";
        discordPresence.startTimestamp = start_time;
        discordPresence.largeImageKey = "citra-logo";
        Discord_UpdatePresence(&discordPresence);
    } else {
        Discord_ClearPresence();
    }
}

static void handleDiscordReady(const DiscordUser* request) {}

static void handleDiscordDisconnected(int errorCode, const char* message) {}

static void handleDiscordError(int errorCode, const char* message) {}

static void handleDiscordJoin(const char* joinSecret) {}

static void handleDiscordSpectate(const char* spectateSecret) {}

static void handleDiscordJoinRequest(const DiscordUser* request) {
    Discord_Respond(request->userId, DISCORD_REPLY_NO);
}

static void discordInit() {
    start_time = time(NULL);
    send_presence = true;
    DiscordEventHandlers handlers{};
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    handlers.joinGame = handleDiscordJoin;
    handlers.spectateGame = handleDiscordSpectate;
    handlers.joinRequest = handleDiscordJoinRequest;
    Discord_Initialize("432553040755556374", &handlers, 1, NULL);
}

static void discordLoop() {
#ifdef DISCORD_DISABLE_IO_THREAD
    Discord_UpdateConnection();
#endif
    Discord_RunCallbacks();
}

static void discordShutdown() {
    send_presence = false;
    Discord_ClearPresence();
    Discord_Shutdown();
}
