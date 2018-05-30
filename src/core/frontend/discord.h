// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <string>

namespace DiscordRPC {

void Init();
void Shutdown();
void Update(const std::string& title);

} // namespace DiscordRPC
