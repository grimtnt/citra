// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <boost/algorithm/string.hpp>
#include "common/logging/log.h"
#include "common/string_util.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/shared_memory.h"
#include "core/hle/service/act/act.h"
#include "core/hle/service/act/act_a.h"
#include "core/hle/service/act/act_u.h"
#include "core/hle/service/cfg/cfg.h"
#include "core/hle/service/fs/archive.h"

namespace Service {
namespace ACT {

void Module::Interface::Initialize(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x0001, 2, 4);
    u32 version = rp.Pop<u32>();
    u32 shared_memory_size = rp.Pop<u32>();
    ASSERT(rp.Pop<u32>() == 0x20);
    rp.Skip(1, false);
    ASSERT(rp.Pop<u32>() == 0);
    u32 shared_memory = rp.Pop<u32>();

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_ACT,
                "(STUBBED) called, version=0x%08X, shared_memory_size=0x%X, shared_memory=0x%08X",
                version, shared_memory_size, shared_memory);
}

void Module::Interface::GetErrorCode(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x0002, 1, 0);
    u32 error_code = rp.Pop<u32>();
    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(error_code); // TODO(valentinvanelslande): convert

    LOG_WARNING(Service_ACT, "(STUBBED) called");
}

void Module::Interface::GetAccountDataBlock(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp(ctx, 0x0006, 3, 2);
    u8 unk = rp.Pop<u8>();
    u32 size = rp.Pop<u32>();
    BlkID id = rp.PopEnum<BlkID>();
    ASSERT(rp.Pop<u32>() == ((size << 4) | 0xC));
    VAddr addr = (VAddr)Kernel::GetCommandBuffer()[5];
    switch (id) {
    case BlkID::NNID: {
        std::string nnid = Common::UTF16ToUTF8(Service::CFG::GetCurrentModule()->GetUsername());
        nnid.resize(0x11);
        boost::algorithm::replace_all(nnid, " ", "_");
        Memory::WriteBlock(addr, nnid.c_str(), nnid.length());
        break;
    }
    case BlkID::Unknown6: {
        u32 a = 1;
        Memory::WriteBlock(addr, &a, sizeof(a));
        break;
    }
    case BlkID::U16MiiName: {
        std::u16string username = Service::CFG::GetCurrentModule()->GetUsername();
        Memory::WriteBlock(addr, username.c_str(), username.length());
        break;
    }
    case BlkID::PrincipalID: {
        u32 principal_id = 0xDEADBEEF;
        Memory::WriteBlock(addr, &principal_id, sizeof(principal_id));
        break;
    }
    case BlkID::CountryName: {
        u8 country_code = std::get<1>(Service::CFG::GetCurrentModule()->GetCountryInfo());
        Memory::Write16(addr, Service::CFG::country_codes[country_code]);
        break;
    }
    case BlkID::MiiImageURL: {
        const char* url = "https://avatars0.githubusercontent.com/u/4592895";
        Memory::WriteBlock(addr, url, std::strlen(url));
        break;
    }
    case BlkID::Age: {
        u16 age = unk == 0xFE ? 0x00 : 0x0C;
        Memory::WriteBlock(addr, &age, sizeof(age));
    }
    default: {
        NGLOG_ERROR(Service_ACT, "Unimplemented block ID");
        UNIMPLEMENTED();
        break;
    }
    }

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_ACT, "(STUBBED) called, unk=0x%02X, size=0x%X, id=0x%X", unk, size, id);
}

Module::Interface::Interface(std::shared_ptr<Module> act, const char* name, u32 max_session)
    : ServiceFramework(name, max_session), act(std::move(act)) {}

void InstallInterfaces(SM::ServiceManager& service_manager) {
    auto act = std::make_shared<Module>();
    std::make_shared<ACT_A>(act)->InstallAsService(service_manager);
    std::make_shared<ACT_U>(act)->InstallAsService(service_manager);
}

} // namespace ACT
} // namespace Service
