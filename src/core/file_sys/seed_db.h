// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <array>
#include <vector>
#include "common/common_types.h"

namespace FileSys {

struct Seed {
    u64 title_id;
    std::array<u8, 16> data;
};

struct SeedDB {
    u32 count;
    std::vector<Seed> seeds;

    void Load();
    void Save();
};

SeedDB GetSeedDB();
void AddSeed(u64 title_id, std::array<u8, 16> seed);
bool GetSeed(u64 title_id, std::array<u8, 16>& output);
u32 GetSeedCount();

} // namespace FileSys
