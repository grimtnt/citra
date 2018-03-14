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
    // TODO: find out why direct command buffer modifying is needed to this to work correctly
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x0006, 3, 2);
    u8 unk = rp.Pop<u8>();
    u32 size = rp.Pop<u32>();
    BlkID id = rp.PopEnum<BlkID>();
    ASSERT(rp.Pop<u32>() == ((size << 4) | 0xC));
    VAddr addr = rp.Pop<VAddr>();
    switch (id) {
    case BlkID::NNID: {
        std::string nnid = Common::UTF16ToUTF8(Service::CFG::GetUsername());
        // TODO: find out why max length is 7 in citra
        if (nnid.length() > 7)
            nnid = nnid.substr(0, 7);
        boost::algorithm::replace_all(nnid, " ", "_");
        const char* network_id = nnid.c_str();
        Memory::WriteBlock(addr, network_id, sizeof(network_id));
        break;
    }
    case BlkID::Unknown6: {
        u32 a = 1;
        Memory::WriteBlock(addr, &a, sizeof(a));
        break;
    }
    case BlkID::U16MiiName: {
        const char16_t* mii_name = Service::CFG::GetUsername().c_str();
        Memory::WriteBlock(addr, mii_name, sizeof(mii_name));
        break;
    }
    case BlkID::PrincipalID: {
        u32 principal_id = 0xDEADBEEF;
        Memory::WriteBlock(addr, &principal_id, sizeof(principal_id));
        break;
    }
    case BlkID::CountryName: {
        std::tuple<unsigned char*, u8> country_tuple = Service::CFG::GetCountryInfo();
        u8 country_code = std::get<1>(country_tuple);
        Memory::Write16(addr, Service::CFG::country_codes[country_code]);
        break;
    }
    case BlkID::MiiImageURL: {
        char url[0x101] = "https://avatars0.githubusercontent.com/u/4592895";
        Memory::WriteBlock(addr, url, sizeof(url));
        break;
    }
    case BlkID::Age: {
        if (unk == 0xFE) {
            char age[2] = {0x00, 0x00};
            Memory::WriteBlock(addr, age, sizeof(age));
            break;
        } else {
            char age[2] = {0x00, 0x0C};
            Memory::WriteBlock(addr, age, sizeof(age));
            break;
        }
    }
    default: {
        LOG_ERROR(Service_ACT, "Unimplemented block ID");
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
