// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
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

const static std::array<std::string, 1> swkbd_default_1_button = {"Ok"};
const static std::array<std::string, 2> swkbd_default_2_button = {"Cancel", "Ok"};
const static std::array<std::string, 3> swkbd_default_3_button = {"Cancel", "I Forgot", "Ok"};

ResultCode SoftwareKeyboard::ReceiveParameter(Service::APT::MessageParameter const& parameter) {
    if (parameter.signal != Service::APT::SignalType::Request) {
        LOG_ERROR(Service_APT, "unsupported signal %u", static_cast<u32>(parameter.signal));
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

static bool ValidateFilters(const u32 filters, const std::string& input) {
    bool valid = true;
    bool local_filter = true;
    if ((filters & SwkbdFilter_Digits) == SwkbdFilter_Digits) {
        valid &= local_filter =
            std::all_of(input.begin(), input.end(), [](const char c) { return !std::isdigit(c); });
        if (!local_filter) {
            std::cout << "Input must not contain any digits" << std::endl;
        }
    }
    if ((filters & SwkbdFilter_At) == SwkbdFilter_At) {
        valid &= local_filter = input.find("@") == std::string::npos;
        if (!local_filter) {
            std::cout << "Input must not contain the @ symbol" << std::endl;
        }
    }
    if ((filters & SwkbdFilter_Percent) == SwkbdFilter_Percent) {
        valid &= local_filter = input.find("%") == std::string::npos;
        if (!local_filter) {
            std::cout << "Input must not contain the % symbol" << std::endl;
        }
    }
    if ((filters & SwkbdFilter_Backslash) == SwkbdFilter_Backslash) {
        valid &= local_filter = input.find("\\") == std::string::npos;
        if (!local_filter) {
            std::cout << "Input must not contain the \\ symbol" << std::endl;
        }
    }
    if ((filters & SwkbdFilter_Profanity) == SwkbdFilter_Profanity) {
        // TODO: check the profanity filter
        LOG_WARNING(Service_APT, "App requested profanity filter, but its not implemented.");
    }
    if ((filters & SwkbdFilter_Callback) == SwkbdFilter_Callback) {
        // TODO: check the callback
        LOG_WARNING(Service_APT, "App requested a callback check, but its not implemented.");
    }
    return valid;
}

static bool ValidateInput(const SoftwareKeyboardConfig& config, const std::string input) {
    // TODO(jroweboy): Is max_text_length inclusive or exclusive?
    if (input.size() > config.max_text_length) {
        std::cout << Common::StringFromFormat("Input is longer than the maximum length. Max: %u",
                                              config.max_text_length)
                  << std::endl;
        return false;
    }
    // return early if the text is filtered
    if (config.filter_flags && !ValidateFilters(config.filter_flags, input)) {
        return false;
    }

    bool valid;
    switch (config.valid_input) {
    case SwkbdValidInput::FixedLen:
        valid = input.size() == config.max_text_length;
        if (!valid) {
            std::cout << Common::StringFromFormat("Input must be exactly %u characters.",
                                                  config.max_text_length)
                      << std::endl;
        }
        break;
    case SwkbdValidInput::NotEmptyNotBlank:
    case SwkbdValidInput::NotBlank:
        valid =
            std::any_of(input.begin(), input.end(), [](const char c) { return !std::isspace(c); });
        if (!valid) {
            std::cout << "Input must not be blank." << std::endl;
        }
        break;
    case SwkbdValidInput::NotEmpty:
        valid = input.empty();
        if (!valid) {
            std::cout << "Input must not be empty." << std::endl;
        }
        break;
    case SwkbdValidInput::Anything:
        valid = true;
        break;
    default:
        // TODO(jroweboy): What does hardware do in this case?
        LOG_CRITICAL(Service_APT, "Application requested unknown validation method. Method: %u",
                     static_cast<u32>(config.valid_input));
        UNREACHABLE();
    }

    return valid;
}

static bool ValidateButton(u32 num_buttons, const std::string& input) {
    // check that the input is a valid number
    bool valid = false;
    try {
        u32 num = std::stoul(input);
        valid = num <= num_buttons;
        if (!valid) {
            std::cout << Common::StringFromFormat("Please choose a number between 0 and %u",
                                                  num_buttons)
                      << std::endl;
        }
    } catch (const std::invalid_argument& e) {
        (void)e;
        std::cout << "Unable to parse input as a number." << std::endl;
    } catch (const std::out_of_range& e) {
        (void)e;
        std::cout << "Input number is not valid." << std::endl;
    }
    return valid;
}

void SoftwareKeyboard::Update() {
    if (Settings::values.swkbd_mode == Settings::SwkbdMode::Qt &&
        Core::System::GetInstance().GetSwkbdFactory().IsRegistered("qt")) {
        auto res = Core::System::GetInstance().GetSwkbdFactory().Launch("qt", config);
        std::u16string utf16_input = Common::UTF8ToUTF16(res.first);
        memcpy(text_memory->GetPointer(), utf16_input.c_str(),
               utf16_input.length() * sizeof(char16_t));
        config.return_code = res.second;
        config.text_length = static_cast<u16>(utf16_input.size());
        config.text_offset = 0;
        Finalize();
        return;
    }

    // TODO(Subv): Handle input using the touch events from the HID module
    // Until then, just read input from the terminal
    std::string input;
    std::cout << "SOFTWARE KEYBOARD" << std::endl;
    // Display hint text
    std::u16string hint(reinterpret_cast<char16_t*>(config.hint_text));
    if (!hint.empty()) {
        std::cout << "Hint text: " << Common::UTF16ToUTF8(hint) << std::endl;
    }
    do {
        std::cout << "Enter the text you will send to the application:" << std::endl;
        std::getline(std::cin, input);
    } while (!ValidateInput(config, input));

    std::string option_text;
    // convert all of the button texts into something we can output
    // num_buttons is in the range of 0-2 so use <= instead of <
    u32 num_buttons = static_cast<u32>(config.num_buttons_m1);
    for (u32 i = 0; i <= num_buttons; ++i) {
        std::string final_text;
        // apps are allowed to set custom text to display on the button
        std::u16string custom_button_text(reinterpret_cast<char16_t*>(config.button_text[i]));
        if (custom_button_text.empty()) {
            // Use the system default text for that button
            if (num_buttons == 0) {
                final_text = swkbd_default_1_button[i];
            } else if (num_buttons == 1) {
                final_text = swkbd_default_2_button[i];
            } else {
                final_text = swkbd_default_3_button[i];
            }
        } else {
            final_text = Common::UTF16ToUTF8(custom_button_text);
        }
        option_text += "\t(" + std::to_string(i) + ") " + final_text + "\t";
    }
    std::string option;
    do {
        std::cout << "\nPlease type the number of the button you will press: \n"
                  << option_text << std::endl;
        std::getline(std::cin, option);
    } while (!ValidateButton(static_cast<u32>(config.num_buttons_m1), option));

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
        LOG_WARNING(Service_APT, "Unknown option for num_buttons_m1: %u",
                    static_cast<u32>(config.num_buttons_m1));
        config.return_code = SwkbdResult::None;
        break;
    }

    std::u16string utf16_input = Common::UTF8ToUTF16(input);
    memcpy(text_memory->GetPointer(), utf16_input.c_str(), utf16_input.length() * sizeof(char16_t));
    config.text_length = static_cast<u16>(utf16_input.size());
    config.text_offset = 0;

    // TODO(Subv): We're finalizing the applet immediately after it's started,
    // but we should defer this call until after all the input has been collected.
    Finalize();
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

SwkbdFactory::~SwkbdFactory() {
    Clear();
}

void SwkbdFactory::Clear() {
    callbacks.clear();
}

void SwkbdFactory::Register(const std::string& name, SwkbdCallback callback) {
    callbacks.emplace(std::move(name), std::move(callback));
}

bool SwkbdFactory::IsRegistered(const std::string& name) const {
    auto it = callbacks.find(name);
    return it != callbacks.end();
}

std::pair<std::string, SwkbdResult> SwkbdFactory::Launch(const std::string& name,
                                                         const SoftwareKeyboardConfig& config) {
    auto it = callbacks.find(name);
    if (it != callbacks.end())
        return it->second(config);
    return std::make_pair("", SwkbdResult::None);
}
} // namespace Applets
} // namespace HLE
