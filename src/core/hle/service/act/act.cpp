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

void Initialize(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x1, 2, 4);
    u32 version = rp.Pop<u32>();
    u32 shared_memory_size = rp.Pop<u32>();
    rp.Skip(3, false);
    u32 shared_memory = rp.Pop<u32>();

    IPC::RequestBuilder rb = rp.MakeBuilder(1, 0);
    rb.Push(RESULT_SUCCESS);

    LOG_WARNING(Service_ACT,
                "(STUBBED) called, version=0x%08X, shared_memory_size=0x%X, shared_memory=0x%08X",
                version, shared_memory_size, shared_memory);
}

void GetErrorCode(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x2, 1, 0);
    u32 error_code = rp.Pop<u32>();
    IPC::RequestBuilder rb = rp.MakeBuilder(2, 0);
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(error_code); // TODO(valentinvanelslande): convert

    LOG_WARNING(Service_ACT, "(STUBBED) called");
}

void GetAccountDataBlock(Service::Interface* self) {
    IPC::RequestParser rp(Kernel::GetCommandBuffer(), 0x6, 3, 2);
    u8 unk = rp.Pop<u8>();
    u32 size = rp.Pop<u32>();
    BlkId id = rp.PopEnum<BlkId>();
    u32 assert1 = rp.Pop<u32>();
    ASSERT(assert1 == ((size << 4) | 0xC));
    VAddr addr = static_cast<VAddr>(rp.Pop<u32>());

    switch (id) {
    case BlkId::NNID: {
        std::string nnid = Common::UTF16ToUTF8(Service::CFG::GetUsername());
        if (nnid.length() > 7)
            nnid = nnid.substr(0, 7);
        boost::algorithm::replace_all(nnid, " ", "_");
        const char* network_id = nnid.c_str();
        Memory::WriteBlock(addr, network_id, sizeof(network_id));
        break;
    }
    case BlkId::Unknown6: {
        u32 a = 1;
        Memory::WriteBlock(addr, &a, sizeof(a));
        break;
    }
    case BlkId::U16MiiName: {
        const char16_t* mii_name = Service::CFG::GetUsername().c_str();
        Memory::WriteBlock(addr, mii_name, sizeof(mii_name));
        break;
    }
    case BlkId::PrincipalID: {
        u32 principal_id = 0xDEADBEEF;
        Memory::WriteBlock(addr, &principal_id, sizeof(principal_id));
        break;
    }
    case BlkId::CountryName: {
        std::tuple<unsigned char*, u8> country_tuple = Service::CFG::GetCountryInfo();
        u8 country_code = std::get<1>(country_tuple);
        Memory::Write16(addr, Service::CFG::country_codes[country_code]);
        break;
    }
    case BlkId::MiiImageURL: {
        char url[0x101] = "https://avatars0.githubusercontent.com/u/4592895";
        Memory::WriteBlock(addr, url, sizeof(url));
        break;
    }
    case BlkId::Age: {
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

void Init() {
    AddService(new ACT_A);
    AddService(new ACT_U);
}

} // namespace ACT
} // namespace Service
