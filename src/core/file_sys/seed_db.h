// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <array>
#include <optional>
#include <vector>
#include "common/common_types.h"
#include "common/swap.h"

namespace FileSys {

struct Seed {
    u64_le title_id;
    std::array<u8, 16> data;
};

struct SeedDB {
    std::vector<Seed> seeds;

    void Load();
    void Save();
    void Add(const Seed& seed);

    u32 GetCount();
};

void AddSeed(const Seed& seed);
std::optional<std::array<u8, 16>> GetSeed(u64 title_id);
u32 GetSeedCount();

} // namespace FileSys
