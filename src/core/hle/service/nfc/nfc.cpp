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
    std::array<u8, 10> uuid;
    u8 uuid_length; // TODO(ogniK): Figure out if this is actual the uuid length or does it mean
                    // something else
    INSERT_PADDING_BYTES(0x15);
    u32_le protocol;
    u32_le tag_type;
    INSERT_PADDING_BYTES(0x30);
};
static_assert(sizeof(TagInfo) == 0x58, "TagInfo is an invalid size");

struct ModelInfo {
    std::array<u8, 0x8> amiibo_identification_block;
    INSERT_PADDING_BYTES(0x38);
};
static_assert(sizeof(ModelInfo) == 0x40, "ModelInfo is an invalid size");

void Module::Interface::Initialize(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x01, 1, 0};
    u8 param{rp.Pop<u8>()};

    nfc->nfc_tag_state = TagState::NotScanning;

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called, param={}", param);
}

void Module::Interface::Shutdown(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x02, 1, 0};
    u8 param{rp.Pop<u8>()};

    nfc->nfc_tag_state = TagState::NotInitialized;

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

    switch (nfc->nfc_tag_state) {
    case TagState::TagInRange:
    case TagState::TagDataLoaded:
        nfc->tag_out_of_range_event->Signal();
        nfc->nfc_tag_state = TagState::NotScanning;
        break;
    case TagState::Scanning:
    case TagState::TagOutOfRange:
        nfc->nfc_tag_state = TagState::NotScanning;
        break;
    }

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::StartTagScanning(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x05, 1, 0}; // 0x00050040
    u16 in_val{rp.Pop<u16>()};

    // this works
    if (nfc->nfc_tag_state == TagState::NotScanning) {
        nfc->nfc_tag_state = TagState::Scanning;
    }

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
    size_t read_length{nfc_file.ReadBytes(tag_info.uuid.data(), sizeof(tag_info.uuid.size()))};
    tag_info.uuid_length = static_cast<u8>(read_length);
    tag_info.protocol = 1; // TODO(ogniK): Figure out actual values
    tag_info.tag_type = 2;

    IPC::ResponseBuilder rb{rp.MakeBuilder(2, 0)};
    rb.Push(RESULT_SUCCESS);

    // TODO: Fix
    rb.PushRaw<TagInfo>(tag_info);
}

void Module::Interface::StopTagScanning(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x06, 0, 0};

    nfc->nfc_tag_state = TagState::NotScanning;

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::LoadAmiiboData(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x07, 0, 0};

    nfc->nfc_tag_state = TagState::TagDataLoaded;

    IPC::ResponseBuilder rb{rp.MakeBuilder(1, 0)};
    rb.Push(RESULT_SUCCESS);
    LOG_WARNING(Service_NFC, "(STUBBED) called");
}

void Module::Interface::ResetTagScanState(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x08, 0, 0};

    nfc->nfc_tag_state = TagState::NotScanning;

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
    rb.PushEnum(nfc->nfc_tag_state);
    LOG_DEBUG(Service_NFC, "(STUBBED) called");
}

void Module::Interface::CommunicationGetStatus(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx, 0x0F, 0, 0};

    IPC::ResponseBuilder rb{rp.MakeBuilder(2, 0)};
    rb.Push(RESULT_SUCCESS);
    rb.PushEnum(nfc->nfc_status);
    LOG_DEBUG(Service_NFC, "(STUBBED) called");
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
