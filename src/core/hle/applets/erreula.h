// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include "common/common_types.h"
#include "core/hle/applets/factory.h"
#include "core/hle/applets/applet.h"
#include "core/hle/kernel/shared_memory.h"
#include "core/hle/service/apt/apt.h"

namespace HLE {
namespace Applets {

enum class ErrEulaErrorType : u32 {
    ErrorCode,
    ErrorText,
    Eula,
    EulaFirstBoot,
    EulaDrawOnly,
    Agree,
    LocalizedErrorText = ErrorText | 0x100,
};

enum class ErrEulaResult : s32 {
    Unknown = -1,
    None,
    Success,
    NotSupported,
    HomeButton = 10,
    SoftwareReset,
    PowerButton
};

struct ErrEulaConfig {
    ErrEulaErrorType error_type;
    u32 error_code;
    u16 upper_screen_flag;
    u16 use_language;
    char16_t error_text[1900];
    bool home_button;
    bool software_reset;
    bool app_jump;
    INSERT_PADDING_BYTES(137);
    ErrEulaResult result_code;
    u16 eula_version;
    INSERT_PADDING_BYTES(10);
};

static_assert(sizeof(ErrEulaConfig) == 0xF80, "ErrEulaConfig structure size is wrong");

class ErrEula final : public Applet {
public:
    explicit ErrEula(Service::APT::AppletId id, std::weak_ptr<Service::APT::AppletManager> manager)
        : Applet(id, std::move(manager)) {}

    ResultCode ReceiveParameter(const Service::APT::MessageParameter& parameter) override;
    ResultCode StartImpl(const Service::APT::AppletStartupParameter& parameter) override;
    void Update() override;
    void Finalize();

private:
    /// This SharedMemory will be created when we receive the LibAppJustStarted message.
    /// It holds the framebuffer info retrieved by the application with
    /// GSPGPU::ImportDisplayCaptureInfo
    Kernel::SharedPtr<Kernel::SharedMemory> framebuffer_memory;
    ErrEulaConfig config;
};

using ErrEulaCallback = std::function<ErrEulaResult(const ErrEulaConfig&)>;

class ErrEulaFactory : public AppletFactory<ErrEulaCallback, ErrEulaResult, ErrEulaConfig> {
public:
    ErrEulaFactory() {
        default_result = ErrEulaResult::None;
    }
};
} // namespace Applets
} // namespace HLE
