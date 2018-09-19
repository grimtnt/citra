// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <iostream>
#include "common/assert.h"
#include "common/logging/log.h"
#include "common/string_util.h"
#include "core/core.h"
#include "core/hle/applets/swkbd.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/shared_memory.h"
#include "core/hle/result.h"
#include "core/hle/service/gsp/gsp.h"
#include "core/hle/service/hid/hid.h"
#include "core/memory.h"
#include "video_core/video_core.h"

namespace HLE::Applets {

ValidationError ValidateFilters(const SoftwareKeyboardConfig& config, const std::string& input) {
    if ((config.filter_flags & SwkbdFilter_Digits) == SwkbdFilter_Digits) {
        if (std::any_of(input.begin(), input.end(),
                        [](unsigned char c) { return std::isdigit(c); })) {
            return ValidationError::DigitNotAllowed;
        }
    }
    if ((config.filter_flags & SwkbdFilter_At) == SwkbdFilter_At) {
        if (input.find('@') != std::string::npos) {
            return ValidationError::AtSignNotAllowed;
        }
    }
    if ((config.filter_flags & SwkbdFilter_Percent) == SwkbdFilter_Percent) {
        if (input.find('%') != std::string::npos) {
            return ValidationError::PercentNotAllowed;
        }
    }
    if ((config.filter_flags & SwkbdFilter_Backslash) == SwkbdFilter_Backslash) {
        if (input.find('\\') != std::string::npos) {
            return ValidationError::BackslashNotAllowed;
        }
    }
    if ((config.filter_flags & SwkbdFilter_Profanity) == SwkbdFilter_Profanity) {
        // TODO: check the profanity filter
        LOG_INFO(Applet_Swkbd, "App requested swkbd profanity filter, but its not implemented.");
    }
    if ((config.filter_flags & SwkbdFilter_Callback) == SwkbdFilter_Callback) {
        // TODO: check the callback
        LOG_INFO(Applet_Swkbd, "App requested a swkbd callback, but its not implemented.");
    }
    return ValidationError::None;
}

ValidationError ValidateInput(const SoftwareKeyboardConfig& config, const std::string& input) {
    ValidationError error;
    if ((error = ValidateFilters(config, input)) != ValidationError::None) {
        return error;
    }

    // TODO(jroweboy): Is max_text_length inclusive or exclusive?
    if (input.size() > config.max_text_length) {
        return ValidationError::MaxLengthExceeded;
    }

    if (!config.multiline && (input.find('\n') != std::string::npos)) {
        return ValidationError::NewLineNotAllowed;
    }
    auto is_blank = [&] {
        return std::all_of(input.begin(), input.end(),
                           [](unsigned char c) { return std::isspace(c); });
    };
    auto is_empty = [&] { return input.empty(); };
    switch (config.valid_input) {
    case SwkbdValidInput::FixedLen:
        if (input.size() != config.max_text_length) {
            return ValidationError::FixedLengthRequired;
        }
        break;
    case SwkbdValidInput::NotEmptyNotBlank:
        if (is_blank()) {
            return ValidationError::BlankInputNotAllowed;
        }
        if (is_empty()) {
            return ValidationError::EmptyInputNotAllowed;
        }
        break;
    case SwkbdValidInput::NotBlank:
        if (is_blank()) {
            return ValidationError::BlankInputNotAllowed;
        }
        break;
    case SwkbdValidInput::NotEmpty:
        if (is_empty()) {
            return ValidationError::EmptyInputNotAllowed;
        }
        break;
    case SwkbdValidInput::Anything:
        break;
    default:
        // TODO(jroweboy): What does hardware do in this case?
        LOG_CRITICAL(Frontend, "Application requested unknown validation method. Method: {}",
                     static_cast<u32>(config.valid_input));
        UNREACHABLE();
    }

    switch (config.type) {
    case SwkbdType::QWERTY:
    case SwkbdType::Western:
    case SwkbdType::Normal:
        return ValidationError::None;
    case SwkbdType::Numpad:
        return std::all_of(input.begin(), input.end(), [](const char c) { return std::isdigit(c); })
                   ? ValidationError::None
                   : ValidationError::InputNotNumber;
    default:
        return ValidationError::None;
    }
}

ValidationError ValidateButton(const SoftwareKeyboardConfig& config, u8 button) {
    switch (config.num_buttons_m1) {
    case SwkbdButtonConfig::NoButton:
        return ValidationError::None;
    case SwkbdButtonConfig::SingleButton:
        if (button != 0) {
            return ValidationError::ButtonOutOfRange;
        }
        break;
    case SwkbdButtonConfig::DualButton:
        if (button > 1) {
            return ValidationError::ButtonOutOfRange;
        }
        break;
    case SwkbdButtonConfig::TripleButton:
        if (button > 2) {
            return ValidationError::ButtonOutOfRange;
        }
        break;
    default:
        UNREACHABLE();
    }
    return ValidationError::None;
}

ResultCode SoftwareKeyboard::ReceiveParameter(Service::APT::MessageParameter const& parameter) {
    if (parameter.signal != Service::APT::SignalType::Request) {
        LOG_ERROR(Applet_Swkbd, "unsupported signal {}", static_cast<u32>(parameter.signal));
        UNIMPLEMENTED();
        // TODO(Subv): Find the right error code
        return ResultCode(-1);
    }

    // The LibAppJustStarted message contains a buffer with the size of the framebuffer shared
    // memory.
    // Create the SharedMemory that will hold the framebuffer data
    Service::APT::CaptureBufferInfo capture_info{};
    ASSERT(sizeof(capture_info) == parameter.buffer.size());

    memcpy(&capture_info, parameter.buffer.data(), sizeof(capture_info));

    using Kernel::MemoryPermission;
    // Allocate a heap block of the required size for this applet.
    heap_memory = std::make_shared<std::vector<u8>>(capture_info.size);
    // Create a SharedMemory that directly points to this heap block.
    framebuffer_memory = Kernel::SharedMemory::CreateForApplet(
        heap_memory, 0, static_cast<u32>(heap_memory->size()), MemoryPermission::ReadWrite,
        MemoryPermission::ReadWrite, "SoftwareKeyboard Memory");

    // Send the response message with the newly created SharedMemory
    Service::APT::MessageParameter result{};
    result.signal = Service::APT::SignalType::Response;
    result.buffer.clear();
    result.destination_id = Service::APT::AppletId::Application;
    result.sender_id = id;
    result.object = framebuffer_memory;

    SendParameter(result);
    return RESULT_SUCCESS;
}

ResultCode SoftwareKeyboard::StartImpl(Service::APT::AppletStartupParameter const& parameter) {
    ASSERT_MSG(parameter.buffer.size() == sizeof(config),
               "The size of the parameter (SoftwareKeyboardConfig) is wrong");

    memcpy(&config, parameter.buffer.data(), parameter.buffer.size());
    text_memory =
        boost::static_pointer_cast<Kernel::SharedMemory, Kernel::Object>(parameter.object);

    // TODO(Subv): Verify if this is the correct behavior
    memset(text_memory->GetPointer(), 0, text_memory->size);

    is_running = true;
    return RESULT_SUCCESS;
}

void SoftwareKeyboard::Update() {
    auto cb{Core::System::GetInstance().GetQtCallbacks().swkbd};
    if (!cb)
        UNREACHABLE_MSG("Qt keyboard callback is nullptr");
    std::u16string text{};
    cb(config, text);
    memcpy(text_memory->GetPointer(), text.c_str(), text.length() * sizeof(char16_t));
    Finalize();
}

void SoftwareKeyboard::Finalize() {
    // Let the application know that we're closing
    Service::APT::MessageParameter message{};
    message.buffer.resize(sizeof(SoftwareKeyboardConfig));
    std::memcpy(message.buffer.data(), &config, message.buffer.size());
    message.signal = Service::APT::SignalType::WakeupByExit;
    message.destination_id = Service::APT::AppletId::Application;
    message.sender_id = id;
    SendParameter(message);

    is_running = false;
}
} // namespace HLE::Applets
