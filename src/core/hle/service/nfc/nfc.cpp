// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/core.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/event.h"
#include "core/hle/service/nfc/nfc.h"
#include "core/hle/service/nfc/nfc_m.h"
#include "core/hle/service/nfc/nfc_u.h"

namespace Service::NFC {

struct TagInfo {
    u16 id_offset_size;
    u8 unk1;
    u8 unk2;
    std::array<u8, 10> uuid;
    INSERT_PADDING_BYTES(0x1D);
};
static_assert(sizeof(TagInfo) == 0x2C, "TagInfo has an invalid size");

struct AmiiboConfig {
    u16 lastwritedate_year;
    u8 lastwritedate_month;
    u8 lastwritedate_day;
    u16 write_counter;

    std::array<u8, 3> characterID; /// the first element is the collection ID, the second the
                                   /// character in this collection, the third the variant
    u8 series;                     /// ID of the series
    u16 amiiboID; /// ID shared by all exact same amiibo. Some amiibo are only distinguished by this
                  /// one like regular SMB Series Mario and the gold one
    u8 type;      /// Type of amiibo 0 = figure, 1 = card, 2 = plush
    u8 pagex4_byte3;
    u16 appdata_size; /// "NFC module writes hard-coded u8 value 0xD8 here. This is the size of the
                      /// Amiibo AppData, apps can use this with the AppData R/W commands. ..."
    INSERT_PADDING_BYTES(0x30); /// "Unused / reserved: this is cleared by NFC module but never
                                /// written after that."
};
static_assert(sizeof(AmiiboConfig) == 0x40, "AmiiboConfig has an invalid size");

void Module::Interface::Initialize(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x01, 1, 0};
    u8 param{rp.Pop<u8>()};

    Core::System& system{Core::System::GetInstance()};
    system.SetNFCTagState(TagState::NotScanning);

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called, param={}", param);
}

void Module::Interface::Shutdown(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x02, 1, 0};
    u8 param{rp.Pop<u8>()};

    Core::System& system{Core::System::GetInstance()};
    system.SetNFCTagState(TagState::NotInitialized);

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called, param={}", param);
}

void Module::Interface::StartCommunication(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x03, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::StopCommunication(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x04, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::StartTagScanning(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x05, 1, 0};
    u16 in_val{rp.Pop<u16>()};

    Core::System& system{Core::System::GetInstance()};
    system.SetNFCTagState(TagState::Scanning);

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called, in_val={:04x}", in_val);
}

void Module::Interface::GetTagInfo(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x11, 1, 0};

    LOG_CRITICAL(Service_NFC, "called");
    TagInfo tag_info{};
    Core::System& system{Core::System::GetInstance()};
    FileUtil::IOFile nfc_file{system.GetNFCFilename(), "rb"};
    std::size_t read_length{nfc_file.ReadBytes(tag_info.uuid.data(), tag_info.uuid.size())};
    tag_info.id_offset_size = static_cast<u8>(read_length);
    tag_info.unk1 = 0x0;
    tag_info.unk2 = 0x2;

    IPC::ResponseBuilder rb{rp.MakeBuilder(12, 0)};
    rb.Push(RESULT_SUCCESS);
    rb.PushRaw<TagInfo>(tag_info);
}

void Module::Interface::GetAmiiboConfig(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x18, 0, 0};

    // TODO: FILL THE STRUCT!
    AmiiboConfig amiibo_config{};

    // amiibo_config.lastwritedate_year = 2017;
    // amiibo_config.lastwritedate_month = 10;
    // amiibo_config.lastwritedate_day = 10;

    // amiibo_config.write_counter = 1;

    // amiibo_config.characterID[0] = 1; /// the first element is the collection ID, the second the
    // amiibo_config.characterID[1] = 2;
    // amiibo_config.characterID[2] = 3;

    /// character in this collection, the third the variant
    // amiibo_config.series = 4; /// ID of the series

    // TODO: Use
    // FileUtil::IOFile nfc_file{system.GetNFCFilename(), "rb"};
    // std::size_t read_length{nfc_file.ReadBytes(tag_info.uuid.data(),
    // tag_info.uuid.size())}; amiibo_config.amiiboID = 12345678; /// ID shared by all exact
    // same amiibo. Some amiibo are
    // only
    /// distinguished by this
    /// one like regular SMB Series Mario and the gold one
    // u8 type{0};                       /// Type of amiibo 0 = figure, 1 = card, 2 = plush
    // u8 pagex4_byte3{456};
    // u16 appdata_size{0xD8};

    IPC::ResponseBuilder rb{rp.MakeBuilder(17, 0)};
    rb.Push(RESULT_SUCCESS);
    rb.PushRaw<AmiiboConfig>(amiibo_config);

    LOG_CRITICAL(Service_NFC, "called");
}

void Module::Interface::StopTagScanning(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x06, 0, 0};

    Core::System& system{Core::System::GetInstance()};
    system.SetNFCTagState(TagState::NotScanning);

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::LoadAmiiboData(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x07, 0, 0};

    Core::System& system{Core::System::GetInstance()};
    system.SetNFCTagState(TagState::TagDataLoaded);

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::ResetTagScanState(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x08, 0, 0};

    Core::System& system{Core::System::GetInstance()};
    system.SetNFCTagState(TagState::TagInRange);

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::GetTagInRangeEvent(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x0B, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 2)};
    rb.Push(RESULT_SUCCESS);

    Core::System& system{Core::System::GetInstance()};
    rb.PushCopyObjects(system.GetNFCEvent());
    LOG_WARNING(Service_NFC, "called");
}

void Module::Interface::GetTagOutOfRangeEvent(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x0C, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 2)};
    rb.Push(RESULT_SUCCESS);
    rb.PushCopyObjects(nfc->tag_out_of_range_event);
    LOG_WARNING(Service_NFC, "called");
}

void Module::Interface::GetTagState(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x0D, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(2, 0)};
    rb.Push(RESULT_SUCCESS);
    Core::System& system{Core::System::GetInstance()};
    rb.PushEnum(system.GetNFCTagState());
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::CommunicationGetStatus(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x0F, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(2, 0)};
    rb.Push(RESULT_SUCCESS);
    rb.PushEnum(nfc->nfc_status);
    LOG_DEBUG(Service_NFC, "(STUBBED) called");
}

void Module::Interface::Unknown1(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x1A, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

Module::Interface::Interface(std::shared_ptr<Module> nfc, const char* name)
    : ServiceFramework{name, 1}, nfc{std::move(nfc)} {}

Module::Interface::~Interface() = default;

Module::Module() {
    tag_out_of_range_event =
        Kernel::Event::Create(Kernel::ResetType::OneShot, "NFC::tag_out_range_event");
}

Module::~Module() = default;

void InstallInterfaces(SM::ServiceManager& service_manager) {
    auto nfc{std::make_shared<Module>()};
    std::make_shared<NFC_M>(nfc)->InstallAsService(service_manager);
    std::make_shared<NFC_U>(nfc)->InstallAsService(service_manager);
}

} // namespace Service::NFC
