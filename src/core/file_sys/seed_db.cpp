// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <fmt/format.h>
#include "common/file_util.h"
#include "core/file_sys/seed_db.h"

namespace FileSys {

void SeedDB::Load() {
    seeds.clear();
    const std::string path{fmt::format("{}/seed_db.bin", FileUtil::GetUserPath(D_SYSDATA_IDX))};
    if (!FileUtil::Exists(path)) {
        FileUtil::CreateFullPath(path);
        Save();
        return;
    }
    FileUtil::IOFile file{path, "rb"};
    u32 count;
    file.ReadBytes(&count, sizeof(count));
    for (u32 i{}; i < count; ++i) {
        Seed seed{};
        file.ReadBytes(&seed.title_id, sizeof(seed.title_id));
        file.ReadBytes(seed.data.data(), seed.data.size());
        seeds.push_back(seed);
    }
}

void SeedDB::Save() {
    const std::string path{fmt::format("{}/seed_db.bin", FileUtil::GetUserPath(D_SYSDATA_IDX))};
    FileUtil::CreateFullPath(path);
    FileUtil::IOFile file{path, "wb"};
    u32 count{static_cast<u32>(seeds.size())};
    file.WriteBytes(&count, sizeof(count));
    for (std::size_t i{}; i < count; ++i) {
        file.WriteBytes(&seeds[i].title_id, sizeof(seeds[i].title_id));
        file.WriteBytes(seeds[i].data.data(), seeds[i].data.size());
    }
}

void SeedDB::Add(const Seed& seed) {
    seeds.push_back(seed);
}

u32 SeedDB::GetCount() {
    return seeds.size();
}

void AddSeed(const Seed& seed) {
    // TODO: does this skip/replace if the SeedDB contains a seed for seed.title_id?
    SeedDB db;
    db.Load();
    db.Add(seed);
    db.Save();
}

std::optional<std::array<u8, 16>> GetSeed(u64 title_id) {
    SeedDB db;
    db.Load();
    for (const auto& seed : db.seeds) {
        if (seed.title_id == title_id) {
            return seed.data;
        }
    }
    const std::string seed_path{
        fmt::format("{}/seeds/{:016X}.bin", FileUtil::GetUserPath(D_SYSDATA_IDX), title_id)};
    if (FileUtil::Exists(seed_path)) {
        FileUtil::IOFile file{seed_path, "rb"};
        std::array<u8, 16> data;
        file.ReadBytes(data.data(), data.size());
        return data;
    }
    return std::nullopt;
}

u32 GetSeedCount() {
    SeedDB db;
    db.Load();
    return db.GetCount();
}

} // namespace FileSys
