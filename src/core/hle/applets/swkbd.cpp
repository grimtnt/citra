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

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace HLE {
namespace Applets {

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
    Service::APT::CaptureBufferInfo capture_info;
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
    Service::APT::MessageParameter result;
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
    if (Settings::values.swkbd_implementation == Settings::SwkbdImplementation::Qt &&
        Core::System::GetInstance().GetAppletFactories().swkbd.IsRegistered("qt")) {
        std::u16string text;
        // Call the function registered by the frontend
        Core::System::GetInstance().GetAppletFactories().swkbd.Launch("qt", config, text);
        memcpy(text_memory->GetPointer(), text.c_str(), text.length() * sizeof(char16_t));
        Finalize();
    } else {
        // Read from stdin
        std::string input;
        std::cout << "Software Keyboard" << std::endl;
        // Display hint text
        std::u16string hint(reinterpret_cast<char16_t*>(config.hint_text));
        if (!hint.empty()) {
            std::cout << "Hint text: " << Common::UTF16ToUTF8(hint) << std::endl;
        }
        ValidationError error = ValidationError::ButtonOutOfRange;
        auto ValidateInputString = [&]() -> bool {
            ValidationError error = ValidateInput(config, input);
            if (error != ValidationError::None) {
                switch (error) {
                case ValidationError::AtSignNotAllowed:
                    std::cout << "Input must not contain the @ symbol" << std::endl;
                    break;
                case ValidationError::BackslashNotAllowed:
                    std::cout << "Input must not contain the \\ symbol" << std::endl;
                    break;
                case ValidationError::BlankInputNotAllowed:
                    std::cout << "Input must not be blank." << std::endl;
                    break;
                case ValidationError::CallbackFailed:
                    std::cout << "Callbak failed." << std::endl;
                    break;
                case ValidationError::DigitNotAllowed:
                    std::cout << "Input must not contain any digits" << std::endl;
                    break;
                case ValidationError::EmptyInputNotAllowed:
                    std::cout << "Input must not be empty." << std::endl;
                    break;
                case ValidationError::FixedLengthRequired:
                    std::cout << Common::StringFromFormat("Input must be exactly %u characters.",
                                                          config.max_text_length)
                              << std::endl;
                    break;
                case ValidationError::InputNotNumber:
                    std::cout << "All characters must be numbers." << std::endl;
                    break;
                case ValidationError::MaxLengthExceeded:
                    std::cout << Common::StringFromFormat(
                                     "Input is longer than the maximum length. Max: %u",
                                     config.max_text_length)
                              << std::endl;
                    break;
                case ValidationError::PercentNotAllowed:
                    std::cout << "Input must not contain the % symbol" << std::endl;
                    break;
                default:
                    UNREACHABLE();
                }
            }
            return error == ValidationError::None;
        };
        do {
            std::cout << "Enter the text you will send to the application:" << std::endl;
            std::getline(std::cin, input);
        } while (!ValidateInputString());

        std::string option_text;
        // convert all of the button texts into something we can output
        // num_buttons is in the range of 0-2 so use <= instead of <
        u32 num_buttons = static_cast<u32>(config.num_buttons_m1);
        for (u32 i = 0; i <= num_buttons; ++i) {
            std::string button_text;
            // apps are allowed to set custom text to display on the button
            std::u16string custom_button_text(reinterpret_cast<char16_t*>(config.button_text[i]));
            if (custom_button_text.empty()) {
                // Use the system default text for that button
                button_text = default_button_text[num_buttons][i];
            } else {
                button_text = Common::UTF16ToUTF8(custom_button_text);
            }
            option_text += "\t(" + std::to_string(i) + ") " + button_text + "\t";
        }
        std::string option;
        error = ValidationError::ButtonOutOfRange;
        auto ValidateButtonString = [&]() -> bool {
            bool valid = false;
            try {
                u32 num = std::stoul(option);
                valid = ValidateButton(config, static_cast<u8>(num)) == ValidationError::None;
                if (!valid) {
                    std::cout << Common::StringFromFormat("Please choose a number between 0 and %u",
                                                          static_cast<u32>(config.num_buttons_m1))
                              << std::endl;
                }
            } catch (const std::invalid_argument&) {
                std::cout << "Unable to parse input as a number." << std::endl;
            } catch (const std::out_of_range&) {
                std::cout << "Input number is not valid." << std::endl;
            }
            return valid;
        };
        do {
            std::cout << "\nPlease type the number of the button you will press: \n"
                      << option_text << std::endl;
            std::getline(std::cin, option);
        } while (!ValidateButtonString());

        s32 button = std::stol(option);
        switch (config.num_buttons_m1) {
        case SwkbdButtonConfig::SingleButton:
            config.return_code = SwkbdResult::D0Click;
            break;
        case SwkbdButtonConfig::DualButton:
            if (button == 0) {
                config.return_code = SwkbdResult::D1Click0;
            } else {
                config.return_code = SwkbdResult::D1Click1;
            }
            break;
        case SwkbdButtonConfig::TripleButton:
            if (button == 0) {
                config.return_code = SwkbdResult::D2Click0;
            } else if (button == 1) {
                config.return_code = SwkbdResult::D2Click1;
            } else {
                config.return_code = SwkbdResult::D2Click2;
            }
            break;
        default:
            // TODO: what does the hardware do
            LOG_WARNING(Applet_Swkbd, "Unknown option for num_buttons_m1: {}",
                        static_cast<u32>(config.num_buttons_m1));
            config.return_code = SwkbdResult::None;
            break;
        }

        std::u16string utf16_input = Common::UTF8ToUTF16(input);
        memcpy(text_memory->GetPointer(), utf16_input.c_str(),
               utf16_input.length() * sizeof(char16_t));
        config.text_length = static_cast<u16>(utf16_input.size());
        config.text_offset = 0;
        Finalize();
    }
}

void SoftwareKeyboard::Finalize() {
    // Let the application know that we're closing
    Service::APT::MessageParameter message;
    message.buffer.resize(sizeof(SoftwareKeyboardConfig));
    std::memcpy(message.buffer.data(), &config, message.buffer.size());
    message.signal = Service::APT::SignalType::WakeupByExit;
    message.destination_id = Service::APT::AppletId::Application;
    message.sender_id = id;
    SendParameter(message);

    is_running = false;
}
} // namespace Applets
} // namespace HLE
