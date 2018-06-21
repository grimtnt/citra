// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include "common/common_types.h"
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
    ErrEulaResult return_code;
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

// Factory class for ErrEula applet
class ErrEulaFactory {
public:
    bool IsRegistered(const std::string& name) const {
        auto it = callbacks.find(name);
        return it != callbacks.end();
    }

    void Register(std::string name, std::function<void(ErrEulaConfig&)> callback) {
        callbacks.emplace(std::move(name), std::move(callback));
    }

    void Launch(const std::string& name, ErrEulaConfig& config) {
        auto it = callbacks.find(name);
        if (it != callbacks.end())
            it->second(config);
    }

private:
    std::map<std::string, std::function<void(ErrEulaConfig&)>> callbacks;
};
} // namespace Applets
} // namespace HLE
