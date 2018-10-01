// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <fmt/format.h>
#include "common/file_util.h"
#include "core/file_sys/seed_db.h"

namespace FileSys {

void SeedDB::Load() {
    count = 0;
    seeds.clear();
    const std::string path{fmt::format("{}/seeds.bin", FileUtil::GetUserPath(D_SYSDATA_IDX))};
    if (!FileUtil::Exists(path)) {
        FileUtil::CreateFullPath(path);
        Save();
        return;
    }
    FileUtil::IOFile file{path, "rb"};
    file.ReadBytes(&count, sizeof(count));
    for (u32 i{}; i < count; ++i) {
        Seed seed{};
        file.ReadBytes(&seed.title_id, sizeof(seed.title_id));
        file.ReadBytes(seed.data.data(), seed.data.size());
        seeds.push_back(seed);
    }
}

void SeedDB::Save() {
    const std::string path{fmt::format("{}/seeds.bin", FileUtil::GetUserPath(D_SYSDATA_IDX))};
    FileUtil::CreateFullPath(path);
    FileUtil::IOFile file{path, "wb"};
    file.WriteBytes(&count, sizeof(count));
    for (std::size_t i{}; i < count; ++i) {
        file.WriteBytes(&seeds[i].title_id, sizeof(seeds[i].title_id));
        file.WriteBytes(seeds[i].data.data(), seeds[i].data.size());
    }
}

void AddSeed(u64 title_id, std::array<u8, 16> seed) {
    SeedDB db;
    db.Load();
    ++db.count;
    db.seeds.push_back({title_id, seed});
    db.Save();
}

bool GetSeed(u64 title_id, std::array<u8, 16>& output) {
    SeedDB db;
    db.Load();
    for (const auto& seed : db.seeds) {
        if (seed.title_id == title_id) {
            output = seed.data;
            return true;
        }
    }
    const std::string seed_path{
        fmt::format("{}/seeds/{:016X}.bin", FileUtil::GetUserPath(D_SYSDATA_IDX), title_id)};
    if (FileUtil::Exists(seed_path)) {
        FileUtil::IOFile file{seed_path, "rb"};
        file.ReadBytes(output.data(), output.size());
        return true;
    }
    return false;
}

u32 GetSeedCount() {
    SeedDB db;
    db.Load();
    return db.count;
}

} // namespace FileSys
