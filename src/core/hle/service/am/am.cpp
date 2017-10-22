// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <array>
#include <cinttypes>
#include "common/file_util.h"
#include "common/logging/log.h"
#include "common/string_util.h"
#include "core/file_sys/cia_container.h"
#include "core/file_sys/file_backend.h"
#include "core/file_sys/ncch_container.h"
#include "core/file_sys/title_metadata.h"
#include "core/hle/ipc.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/client_port.h"
#include "core/hle/kernel/client_session.h"
#include "core/hle/kernel/errors.h"
#include "core/hle/kernel/handle_table.h"
#include "core/hle/kernel/server_session.h"
#include "core/hle/kernel/session.h"
#include "core/hle/result.h"
#include "core/hle/service/am/am.h"
#include "core/hle/service/am/am_app.h"
#include "core/hle/service/am/am_net.h"
#include "core/hle/service/am/am_sys.h"
#include "core/hle/service/am/am_u.h"
#include "core/hle/service/fs/archive.h"
#include "core/hle/service/service.h"
#include "core/loader/loader.h"

namespace Service {
namespace AM {

static bool lists_initialized = false;
static std::array<std::vector<u64_le>, 3> am_title_list;

// Current CIA install context
static bool cia_is_installing = false;
static bool cia_installing_update = false;
static bool cia_installing_header_loaded = false;
static bool cia_installing_tmd_loaded = false;
static u64 cia_installing_written = 0;
static FileSys::CIAContainer cia_installing;
static std::vector<u8> cia_installing_data;
static std::vector<u64> cia_installing_content_written;
Service::FS::MediaType cia_installing_media_type;

struct TitleInfo {
    u64_le tid;
    u64_le size;
    u16_le version;
    u16_le unused;
    u32_le type;
};

static_assert(sizeof(TitleInfo) == 0x18, "Title info structure size is wrong");

struct ContentInfo {
    u16_le index;
    u16_le type;
    u32_le content_id;
    u64_le size;
    u64_le romfs_size;
};

static_assert(sizeof(ContentInfo) == 0x18, "Content info structure size is wrong");

struct TicketInfo {
    u64_le title_id;
    u64_le ticket_id;
    u16_le version;
    u16_le unused;
    u32_le size;
};

static_assert(sizeof(TicketInfo) == 0x18, "Ticket info structure size is wrong");

// A file handled returned for CIAs to be written into and subsequently installed.
class CIAFile : public FileSys::FileBackend {
public:
    explicit CIAFile(Service::FS::MediaType media_type) : media_type(media_type) {}

    ResultVal<size_t> Read(u64 offset, size_t length, u8* buffer) const override {
        LOG_WARNING(Service_AM, "(STUBBED) ReadCIA");
        return MakeResult<size_t>(length);
    }

    ResultVal<size_t> Write(u64 offset, size_t length, bool flush,
                            const u8* buffer) const override {
        cia_installing_written += length;

        // TODO(shinyquagsire23): Can we assume that things will only be written in sequence?
        // Or does AM send an error if we write to things out of order/assume a sequence?
        u64 cia_header_size = 0x2020;
        if (!cia_installing_header_loaded) {
            cia_installing_data.resize(std::min(offset + length, cia_header_size));
            memcpy(cia_installing_data.data() + offset, buffer, std::min(length, static_cast<size_t>(cia_header_size)));

            if (cia_installing_written >= 0x2020) {
                cia_installing.LoadHeader(cia_installing_data);
                cia_installing.Print();
                cia_installing_header_loaded = true;
            }
        }

        if (cia_installing_header_loaded) {
            if (offset < cia_installing.GetContentOffset()) {
                cia_installing_data.resize(
                    std::min(offset + length, cia_installing.GetContentOffset()));
                memcpy(cia_installing_data.data() + offset, buffer,
                       std::min(length, static_cast<size_t>(cia_installing.GetContentOffset() - offset)));
            }

            if (cia_installing_written >= cia_installing.GetContentOffset() &&
                !cia_installing_tmd_loaded) {
                cia_installing.LoadTitleMetadata(cia_installing_data,
                                                 cia_installing.GetTitleMetadataOffset());
                FileSys::TitleMetadata tmd = cia_installing.GetTitleMetadata();
                tmd.Print();

                if (FileUtil::Exists(GetTitleMetadataPath(media_type, tmd.GetTitleID())))
                    cia_installing_update = true;

                std::string tmd_path =
                    GetTitleMetadataPath(media_type, tmd.GetTitleID(), cia_installing_update);

                // Create content/ folder if it doesn't exist
                std::string tmd_folder;
                Common::SplitPath(tmd_path, &tmd_folder, nullptr, nullptr);
                FileUtil::CreateFullPath(tmd_folder);

                // Save TMD so that we can start getting new .app paths
                tmd.SaveToFile(tmd_path);

                // Create any other .app folders which may exist
                std::string app_folder;
                Common::SplitPath(GetTitleContentPath(media_type, tmd.GetTitleID(),
                                                      FileSys::TMDContentIndex::Main,
                                                      cia_installing_update),
                                  &app_folder, nullptr, nullptr);
                FileUtil::CreateFullPath(app_folder);

                cia_installing_content_written.resize(
                    cia_installing.GetTitleMetadata().GetContentCount());
                cia_installing_tmd_loaded = true;
            }
        }

        // TODO(shinyquagsire23): Write out .tik files to nand?

        if (cia_installing_tmd_loaded) {
            u64 offset_max = offset + length;
            for (int i = 0; i < cia_installing.GetTitleMetadata().GetContentCount(); i++) {
                if (cia_installing_content_written[i] < cia_installing.GetContentSize(i)) {
                    u64 size = cia_installing.GetContentSize(i);
                    u64 range_min =
                        cia_installing.GetContentOffset(i) + cia_installing_content_written[i];
                    u64 range_max = cia_installing.GetContentOffset(i) + size;

                    // We don't have the buffered data for this
                    if (range_min > offset_max || range_max < offset)
                        continue;

                    u64 available_to_write = std::min(offset_max, range_max) - range_min;
                    FileSys::TitleMetadata tmd = cia_installing.GetTitleMetadata();
                    FileUtil::IOFile file(
                        GetTitleContentPath(media_type, tmd.GetTitleID(), i, cia_installing_update),
                        cia_installing_content_written[i] ? "a" : "w");

                    // TODO(shinyquagsire23): Return an error instead of continuing silently?
                    if (file.IsOpen()) {
                        file.WriteBytes(buffer + (range_min - offset), available_to_write);
                    }

                    cia_installing_content_written[i] += available_to_write;
                    LOG_DEBUG(Service_AM, "Wrote %" PRIx64 " to content %u, total %" PRIx64,
                              available_to_write, i, cia_installing_content_written[i]);
                }
            }
        }

        return MakeResult<size_t>(length);
    }

    u64 GetSize() const override {
        return 0;
    }

    bool SetSize(u64 size) const override {
        return false;
    }

    bool Close() const override {
        return true;
    }

    void Flush() const override {}

private:
    Service::FS::MediaType media_type;
};

std::string GetTitleMetadataPath(Service::FS::MediaType media_type, u64 tid, bool update) {
    std::string content_path = GetTitlePath(media_type, tid) + "content/";

    if (media_type == Service::FS::MediaType::GameCard) {
        LOG_ERROR(Service_AM, "Invalid request for nonexistent gamecard title metadata!");
        return "";
    }

    // The TMD ID is usually held in the title databases, which we don't implement.
    // For now, just scan for any .tmd files which exist and use the first .tmd
    // found (there should only really be one unless the directories are meddled with)
    FileUtil::FSTEntry entries;
    FileUtil::ScanDirectoryTree(content_path, entries);
    for (const FileUtil::FSTEntry& entry : entries.children) {
        std::string filename_filename, filename_extension;
        Common::SplitPath(entry.virtualName, nullptr, &filename_filename, &filename_extension);

        u32 id = std::stoul(filename_filename.c_str(), nullptr, 16);
        if (filename_extension == ".tmd")
            return content_path + Common::StringFromFormat("%08x.tmd", id + (update ? 1 : 0));
    }

    // If we can't find an existing .tmd, return a path for one to be created.
    return content_path + "00000000.tmd";
}

std::string GetTitleContentPath(Service::FS::MediaType media_type, u64 tid, u16 index,
                                bool update) {
    std::string content_path = GetTitlePath(media_type, tid) + "content/";

    if (media_type == Service::FS::MediaType::GameCard) {
        // TODO(shinyquagsire23): get current app file if TID matches?
        LOG_ERROR(Service_AM, "Request for gamecard partition %u content path unimplemented!",
                  static_cast<u32>(index));
        return "";
    }

    std::string tmd_path = GetTitleMetadataPath(media_type, tid, update);

    u32 content_id = 0;
    FileSys::TitleMetadata tmd;
    if (tmd.LoadFromFile(tmd_path) == Loader::ResultStatus::Success) {
        content_id = tmd.GetContentIDByIndex(index);

        // TODO(shinyquagsire23): how does DLC actually get this folder on hardware?
        // For now, check if the second (index 1) content has the optional flag set, for most
        // apps this is usually the manual and not set optional, DLC has it set optional.
        // All .apps (including index 0) will be in the 00000000/ folder for DLC.
        if (tmd.GetContentCount() > 1 &&
            tmd.GetContentTypeByIndex(1) & FileSys::TMDContentTypeFlag::Optional) {
            content_path += "00000000/";
        }
    }

    return Common::StringFromFormat("%s%08x.app", content_path.c_str(), content_id);
}

std::string GetTitlePath(Service::FS::MediaType media_type, u64 tid) {
    u32 high = static_cast<u32>(tid >> 32);
    u32 low = static_cast<u32>(tid & 0xFFFFFFFF);

    if (media_type == Service::FS::MediaType::NAND || media_type == Service::FS::MediaType::SDMC)
        return Common::StringFromFormat("%s%08x/%08x/", GetMediaTitlePath(media_type).c_str(), high,
                                        low);

    if (media_type == Service::FS::MediaType::GameCard) {
        // TODO(shinyquagsire23): get current app path if TID matches?
        LOG_ERROR(Service_AM, "Request for gamecard title path unimplemented!");
        return "";
    }

    return "";
}

std::string GetMediaTitlePath(Service::FS::MediaType media_type) {
    if (media_type == Service::FS::MediaType::NAND)
        return Common::StringFromFormat("%s%s/title/", FileUtil::GetUserPath(D_NAND_IDX).c_str(),
                                        SYSTEM_ID);

    if (media_type == Service::FS::MediaType::SDMC)
        return Common::StringFromFormat("%sNintendo 3DS/%s/%s/title/",
                                        FileUtil::GetUserPath(D_SDMC_IDX).c_str(), SYSTEM_ID,
                                        SDCARD_ID);

    if (media_type == Service::FS::MediaType::GameCard) {
        // TODO(shinyquagsire23): get current app parent folder if TID matches?
        LOG_ERROR(Service_AM, "Request for gamecard parent path unimplemented!");
        return "";
    }

    return "";
}

void ScanForTitles(Service::FS::MediaType media_type) {
    am_title_list[static_cast<u32>(media_type)].clear();

    std::string title_path = GetMediaTitlePath(media_type);

    FileUtil::FSTEntry entries;
    FileUtil::ScanDirectoryTree(title_path, entries, 1);
    for (const FileUtil::FSTEntry& tid_high : entries.children) {
        for (const FileUtil::FSTEntry& tid_low : tid_high.children) {
            std::string tid_string = tid_high.virtualName + tid_low.virtualName;
            u64 tid = std::stoull(tid_string.c_str(), nullptr, 16);

            FileSys::NCCHContainer container(GetTitleContentPath(media_type, tid));
            if (container.Load() == Loader::ResultStatus::Success)
                am_title_list[static_cast<u32>(media_type)].push_back(tid);
        }
    }
}

void ScanForAllTitles() {
    ScanForTitles(Service::FS::MediaType::NAND);
    ScanForTitles(Service::FS::MediaType::SDMC);
}

void GetNumPrograms(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x1, 1, 0); // 0x00010040
    u32 media_type = rp.Pop<u8>();

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(am_title_list[media_type].size());
}

void FindContentInfos(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x1002, 4, 2); // 0x10020104

    auto media_type = static_cast<Service::FS::MediaType>(rp.Pop<u8>());
    u64 title_id = rp.Pop<u64>();
    u32 content_count = rp.Pop<u32>();
    VAddr content_requested_in = rp.PopMappedBuffer();
    VAddr content_info_out = rp.PopMappedBuffer();

    std::vector<u16_le> content_requested(content_count);
    Memory::ReadBlock(content_requested_in, content_requested.data(), content_count * sizeof(u16));

    std::string tmd_path = GetTitleMetadataPath(media_type, title_id);

    u32 content_read = 0;
    FileSys::TitleMetadata tmd;
    if (tmd.LoadFromFile(tmd_path) == Loader::ResultStatus::Success) {
        // Get info for each content index requested
        for (size_t i = 0; i < content_count; i++) {
            std::shared_ptr<FileUtil::IOFile> romfs_file;
            u64 romfs_offset = 0;
            u64 romfs_size = 0;

            FileSys::NCCHContainer ncch_container(GetTitleContentPath(media_type, title_id, i));
            ncch_container.ReadRomFS(romfs_file, romfs_offset, romfs_size);

            ContentInfo content_info = {};
            content_info.index = static_cast<u16>(i);
            content_info.type = tmd.GetContentTypeByIndex(content_requested[i]);
            content_info.content_id = tmd.GetContentIDByIndex(content_requested[i]);
            content_info.size = tmd.GetContentSizeByIndex(content_requested[i]);
            content_info.romfs_size = romfs_size;

            Memory::WriteBlock(content_info_out, &content_info, sizeof(ContentInfo));
            content_info_out += sizeof(ContentInfo);
            content_read++;
        }
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);
}

void ListContentInfos(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x1003, 5, 1); // 0x10030142
    u32 content_count = rp.Pop<u32>();
    auto media_type = static_cast<Service::FS::MediaType>(rp.Pop<u8>());
    u64 title_id = rp.Pop<u64>();
    u32 start_index = rp.Pop<u32>();
    VAddr content_info_out = rp.PopMappedBuffer();

    std::string tmd_path = GetTitleMetadataPath(media_type, title_id);

    u32 copied = 0;
    FileSys::TitleMetadata tmd;
    if (tmd.LoadFromFile(tmd_path) == Loader::ResultStatus::Success) {
        copied = std::min(content_count, static_cast<u32>(tmd.GetContentCount()));
        for (u32 i = start_index; i < copied; i++) {
            std::shared_ptr<FileUtil::IOFile> romfs_file;
            u64 romfs_offset = 0;
            u64 romfs_size = 0;

            FileSys::NCCHContainer ncch_container(GetTitleContentPath(media_type, title_id, i));
            ncch_container.ReadRomFS(romfs_file, romfs_offset, romfs_size);

            ContentInfo content_info = {};
            content_info.index = static_cast<u16>(i);
            content_info.type = tmd.GetContentTypeByIndex(i);
            content_info.content_id = tmd.GetContentIDByIndex(i);
            content_info.size = tmd.GetContentSizeByIndex(i);
            content_info.romfs_size = romfs_size;

            Memory::WriteBlock(content_info_out, &content_info, sizeof(ContentInfo));
            content_info_out += sizeof(ContentInfo);
        }
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push(copied);
}

void DeleteContents(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x1004, 4, 1); // 0x10040102
    u8 media_type = rp.Pop<u8>();
    u64 title_id = rp.Pop<u64>();
    u32 content_count = rp.Pop<u32>();
    VAddr content_ids_in = rp.PopMappedBuffer();

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_AM, "(STUBBED) media_type=%u, title_id=0x%016" PRIx64
                            ", content_count=%u, content_ids_in=0x%08x",
                media_type, title_id, content_count, content_ids_in);
}

void GetProgramList(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 2, 2, 1); // 0x00020082

    u32 count = rp.Pop<u32>();
    u8 media_type = rp.Pop<u8>();
    VAddr title_ids_output_pointer = rp.PopMappedBuffer();

    if (!Memory::IsValidVirtualAddress(title_ids_output_pointer) || media_type > 2) {
        IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
        rb.Push<u32>(-1); // TODO(shinyquagsire23): Find the right error code
        rb.Push<u32>(0);
        return;
    }

    u32 media_count = static_cast<u32>(am_title_list[media_type].size());
    u32 copied = std::min(media_count, count);

    Memory::WriteBlock(title_ids_output_pointer, am_title_list[media_type].data(),
                       copied * sizeof(u64));

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push(copied);
}

void GetProgramInfos(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 3, 2, 2); // 0x00030084

    auto media_type = static_cast<Service::FS::MediaType>(rp.Pop<u8>());
    u32 title_count = rp.Pop<u32>();
    VAddr title_id_list_pointer = rp.PopMappedBuffer();
    VAddr title_info_out = rp.PopMappedBuffer();

    std::vector<u64> title_id_list(title_count);
    Memory::ReadBlock(title_id_list_pointer, title_id_list.data(), title_count * sizeof(u64));

    for (u32 i = 0; i < title_count; i++) {
        std::string tmd_path = GetTitleMetadataPath(media_type, title_id_list[i]);

        TitleInfo title_info = {};
        title_info.tid = title_id_list[i];

        FileSys::TitleMetadata tmd;
        if (tmd.LoadFromFile(tmd_path) == Loader::ResultStatus::Success) {
            // TODO(shinyquagsire23): This is the total size of all files this process owns,
            // including savefiles and other content. This comes close but is off.
            title_info.size = tmd.GetContentSizeByIndex(FileSys::TMDContentIndex::Main);
            title_info.version = tmd.GetTitleVersion();
            title_info.type = tmd.GetTitleType();
        }
        Memory::WriteBlock(title_info_out, &title_info, sizeof(TitleInfo));
        title_info_out += sizeof(TitleInfo);
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);
}

void GetPatchTitleInfos(Service::Interface* self) {
    u32* cmd_buff = Kernel::GetCommandBuffer();
    u32 media_type = cmd_buff[1] & 0xFF;
    u32 title_count = cmd_buff[2];
    u64 title_id = Memory::Read64(cmd_buff[4]);
    TitleInfo ti{ title_id, 12345, 120, 0, 1 };
    Memory::WriteBlock(cmd_buff[6], &ti, sizeof ti);
    LOG_WARNING(Service_AM, "(STUBBED) called, title_id=0x%16x", title_id);
}

void GetDataTitleInfos(Service::Interface* self) {
    GetProgramInfos(self);

    LOG_WARNING(Service_AM, "(STUBBED) called");
}

void ListDataTitleTicketInfos(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x1007, 4, 1); // 0x10070102
    u32 ticket_count = rp.Pop<u32>();
    u64 title_id = rp.Pop<u64>();
    u32 start_index = rp.Pop<u32>();
    VAddr ticket_info_out = rp.PopMappedBuffer();
    VAddr ticket_info_write = ticket_info_out;

    for (u32 i = 0; i < ticket_count; i++) {
        TicketInfo ticket_info = {};
        ticket_info.title_id = title_id;
        ticket_info.version = 0; // TODO
        ticket_info.size = 0;    // TODO

        Memory::WriteBlock(ticket_info_write, &ticket_info, sizeof(TicketInfo));
        ticket_info_write += sizeof(TicketInfo);
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push(ticket_count);

    LOG_WARNING(Service_AM, "(STUBBED) ticket_count=0x%08X, title_id=0x%016" PRIx64
                            ", start_index=0x%08X, ticket_info_out=0x%08X",
                ticket_count, title_id, start_index, ticket_info_out);
}

void GetNumContentInfos(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x1001, 3, 0); // 0x100100C0
    auto media_type = static_cast<Service::FS::MediaType>(rp.Pop<u8>());
    u64 title_id = rp.Pop<u64>();

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS); // No error

    std::string tmd_path = GetTitleMetadataPath(media_type, title_id);

    FileSys::TitleMetadata tmd;
    if (tmd.LoadFromFile(tmd_path) == Loader::ResultStatus::Success) {
        rb.Push<u32>(tmd.GetContentCount());
    } else {
        rb.Push<u32>(1); // Number of content infos plus one
        LOG_WARNING(Service_AM, "(STUBBED) called media_type=%u, title_id=0x%016" PRIx64,
                    media_type, title_id);
    }
}

void DeleteTicket(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 7, 2, 0); // 0x00070080
    u64 title_id = rp.Pop<u64>();

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_AM, "(STUBBED) called title_id=0x%016" PRIx64 "", title_id);
}

void GetNumTickets(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 8, 0, 0); // 0x00080000
    u32 ticket_count = 0;

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push(ticket_count);
    LOG_WARNING(Service_AM, "(STUBBED) called ticket_count=0x%08x", ticket_count);
}

void GetTicketList(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 9, 2, 1); // 0x00090082
    u32 ticket_list_count = rp.Pop<u32>();
    u32 ticket_index = rp.Pop<u32>();
    VAddr ticket_tids_out = rp.PopMappedBuffer();

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push(ticket_list_count);
    LOG_WARNING(Service_AM,
                "(STUBBED) ticket_list_count=0x%08x, ticket_index=0x%08x, ticket_tids_out=0x%08x",
                ticket_list_count, ticket_index, ticket_tids_out);
}

void QueryAvailableTitleDatabase(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x19, 1, 0); // 0x190040
    u8 media_type = rp.Pop<u8>();

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS); // No error
    rb.Push(true);

    LOG_WARNING(Service_AM, "(STUBBED) media_type=%u", media_type);
}

void CheckContentRights(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x25, 3, 0); // 0x2500C0
    u64 tid = rp.Pop<u64>();
    u16 content_index = rp.Pop<u16>();

    // TODO(shinyquagsire23): Read tickets for this instead?
    bool has_rights =
        FileUtil::Exists(GetTitleContentPath(Service::FS::MediaType::SDMC, tid, content_index));

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS); // No error
    rb.Push(has_rights);

    LOG_WARNING(Service_AM, "(STUBBED) tid=%016" PRIx64 ", content_index=%u", tid, content_index);
}

void CheckContentRightsIgnorePlatform(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x2D, 3, 0); // 0x2D00C0
    u64 tid = rp.Pop<u64>();
    u16 content_index = rp.Pop<u16>();

    // TODO(shinyquagsire23): Read tickets for this instead?
    bool has_rights =
        FileUtil::Exists(GetTitleContentPath(Service::FS::MediaType::SDMC, tid, content_index));

    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS); // No error
    rb.Push(has_rights);

    LOG_WARNING(Service_AM, "(STUBBED) tid=%016" PRIx64 ", content_index=%u", tid, content_index);
}

void BeginImportProgram(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x0402, 1, 0); // 0x04020040
    auto media_type = static_cast<Service::FS::MediaType>(rp.Pop<u8>());

    if (cia_is_installing) {
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::CIACurrentlyInstalling, ErrorModule::AM,
                           ErrorSummary::InvalidState, ErrorLevel::Permanent));
        return;
    }

    // Create our CIAFile handle for the app to write to, and while the app writes
    // Citra will store contents out to sdmc/nand
    auto file = std::shared_ptr<Service::FS::File>(
        new Service::FS::File(std::make_unique<CIAFile>(media_type), {}));
    auto sessions = Kernel::ServerSession::CreateSessionPair(file->GetName());
    file->ClientConnected(std::get<Kernel::SharedPtr<Kernel::ServerSession>>(sessions));

    // Reset CIA install context
    cia_installing_content_written.clear();
    cia_installing_data.clear();
    cia_installing_media_type = media_type;
    cia_installing_header_loaded = false;
    cia_installing_tmd_loaded = false;
    cia_installing_update = false;
    cia_installing_written = 0;
    cia_installing = FileSys::CIAContainer();
    cia_is_installing = true;

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 1);
    rb.Push(RESULT_SUCCESS); // No error
    rb.PushCopyHandles(
        Kernel::g_handle_table.Create(std::get<Kernel::SharedPtr<Kernel::ClientSession>>(sessions))
            .Unwrap());

    LOG_WARNING(Service_AM, "(STUBBED) media_type=%u", media_type);
}

void EndImportProgram(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x0405, 0, 1); // 0x04050002
    auto cia_handle = rp.PopHandle();

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    // Clean up older data
    if (cia_installing_update) {
        FileSys::TitleMetadata old_tmd;
        FileSys::TitleMetadata new_tmd = cia_installing.GetTitleMetadata();

        std::string old_tmd_path =
            GetTitleMetadataPath(cia_installing_media_type, new_tmd.GetTitleID(), false);
        std::string new_tmd_path =
            GetTitleMetadataPath(cia_installing_media_type, new_tmd.GetTitleID(), true);
        old_tmd.LoadFromFile(old_tmd_path);

        for (u16 i = 0; i < old_tmd.GetContentCount(); i++) {
            bool abort = false;
            for (u16 j = 0; j < new_tmd.GetContentCount(); j++) {
                if (old_tmd.GetContentIDByIndex(i) == new_tmd.GetContentIDByIndex(j))
                    abort = true;
            }
            if (abort)
                break;

            FileUtil::Delete(
                GetTitleContentPath(cia_installing_media_type, old_tmd.GetTitleID(), i));
        }

        FileUtil::Delete(old_tmd_path);
    }
    ScanForAllTitles();

    cia_is_installing = false;
    Kernel::g_handle_table.Close(cia_handle);
}

void GetProgramInfoFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x0408, 1, 1); // 0x04080042
    auto media_type = static_cast<Service::FS::MediaType>(rp.Pop<u8>());

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());
    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            FileSys::TitleMetadata tmd = container.GetTitleMetadata();
            TitleInfo title_info = {};
            container.Print();

            // TODO(shinyquagsire23): Sizes allegedly depend on the mediatype, and will double
            // on some mediatypes. Since this is more of a required install size we'll report
            // what Citra needs, but it would be good to be more accurate here.
            title_info.tid = tmd.GetTitleID();
            title_info.size = tmd.GetContentSizeByIndex(FileSys::TMDContentIndex::Main);
            title_info.version = tmd.GetTitleVersion();
            title_info.type = tmd.GetTitleType();

            IPC::RequestBuilder rb = rp.MakeBuilder(8, 0);
            rb.Push(RESULT_SUCCESS);
            rb.PushRaw<TitleInfo>(title_info);
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}
void GetSystemMenuDataFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x0409, 0, 2); // 0x04090004

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());

    size_t output_buffer_size;
    VAddr output_buffer = rp.PopMappedBuffer(&output_buffer_size);
    output_buffer_size = std::min(
        output_buffer_size,
        static_cast<size_t>(0x36C0)); // TODO(shinyquagsire23): is this size defined anywhere?

    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            std::vector<u8> temp(output_buffer_size);

            //  Read from the Meta offset + 0x400 for the 0x36C0-large SMDH
            auto read_result = file->backend->Read(container.GetMetadataOffset() + 0x400,
                                                   output_buffer_size, temp.data());
            if (read_result.Failed() || *read_result != output_buffer_size) {
                IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
                rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                                   ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
                return;
            }

            Memory::WriteBlock(output_buffer, temp.data(), output_buffer_size);

            IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
            rb.Push(RESULT_SUCCESS);
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}
void GetDependencyListFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x040A, 0, 2); // 0x040A0002

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());

    size_t output_buffer_size;
    VAddr output_buffer = rp.PopStaticBuffer(&output_buffer_size);
    output_buffer_size = std::min(output_buffer_size, static_cast<size_t>(0x300));

    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            Memory::WriteBlock(output_buffer, container.GetDependencies().data(),
                               output_buffer_size);

            IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
            rb.Push(RESULT_SUCCESS);
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}
void GetTransferSizeFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x040B, 0, 1); // 0x040B0002

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());

    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            IPC::RequestBuilder rb = rp.MakeBuilder(3, 0);
            rb.Push(RESULT_SUCCESS);
            rb.Push(container.GetMetadataOffset());
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}
void GetCoreVersionFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x040C, 0, 1); // 0x040C0002

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());

    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
            rb.Push(RESULT_SUCCESS);
            rb.Push(container.GetCoreVersion());
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}

void GetRequiredSizeFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x040D, 1, 1); // 0x040D0042
    auto media_type = static_cast<Service::FS::MediaType>(rp.Pop<u8>());

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());
    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            // TODO(shinyquagsire23): Sizes allegedly depend on the mediatype, and will double
            // on some mediatypes. Since this is more of a required install size we'll report
            // what Citra needs, but it would be good to be more accurate here.
            IPC::RequestBuilder rb = rp.MakeBuilder(3, 0);
            rb.Push(RESULT_SUCCESS);
            rb.Push(
                container.GetTitleMetadata().GetContentSizeByIndex(FileSys::TMDContentIndex::Main));
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}

void GetMetaSizeFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x0413, 0, 1); // 0x04130002

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());

    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
            rb.Push(RESULT_SUCCESS);
            rb.Push(container.GetMetadataSize());
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}

void GetMetaDataFromCia(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x0414, 0, 1); // 0x04140044

    // We need our FileBackend to get info, so we've gotta step up the chain from
    // Handle->ClientSession->ServerSession->Backend
    u32 output_buffer_size = rp.Pop<u32>();
    auto file_session = Kernel::g_handle_table.Get<Kernel::ClientSession>(rp.PopHandle());
    VAddr output_buffer = rp.PopStaticBuffer();

    if (file_session == nullptr || file_session->parent == nullptr) {
        LOG_WARNING(Service_AM, "Invalid file handle!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_INVALID_HANDLE);
        return;
    }

    Kernel::SharedPtr<Kernel::ServerSession> server = file_session->parent->server;
    if (server == nullptr) {
        LOG_WARNING(Service_AM, "File handle ServerSession disconnected!");
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_SESSION_CLOSED_BY_REMOTE);
        return;
    }

    if (server->hle_handler != nullptr) {
        auto file = std::dynamic_pointer_cast<Service::FS::File>(server->hle_handler);

        FileSys::CIAContainer container;
        if (container.LoadFromFileBackend(*file->backend) == Loader::ResultStatus::Success) {
            std::vector<u8> temp(output_buffer_size);

            //  Read from the Meta offset for the specified size
            auto read_result =
                file->backend->Read(container.GetMetadataOffset(), output_buffer_size, temp.data());
            if (read_result.Failed() || *read_result != output_buffer_size) {
                IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
                rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                                   ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
                return;
            }

            Memory::WriteBlock(output_buffer, temp.data(), output_buffer_size);

            IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
            rb.Push(RESULT_SUCCESS);
            return;
        }

        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(ResultCode(ErrCodes::InvalidCIAHeader, ErrorModule::AM,
                           ErrorSummary::InvalidArgument, ErrorLevel::Permanent));
        return;
    } else {
        // TODO(shinyquagsire23): LLE file handling?
        IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
        rb.Push(Kernel::ERR_NOT_IMPLEMENTED);
        return;
    }
}

void Init() {
    AddService(new AM_APP_Interface);
    AddService(new AM_NET_Interface);
    AddService(new AM_SYS_Interface);
    AddService(new AM_U_Interface);

    ScanForAllTitles();
}

void Shutdown() {}

} // namespace AM

} // namespace Service
