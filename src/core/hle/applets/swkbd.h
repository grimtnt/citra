// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include <string>
#include "common/common_funcs.h"
#include "common/common_types.h"
#include "core/hle/applets/applet.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/shared_memory.h"
#include "core/hle/result.h"
#include "core/hle/service/apt/apt.h"

namespace HLE {
namespace Applets {

/// Maximum number of buttons that can be in the keyboard.
#define SWKBD_MAX_BUTTON 3
/// Maximum button text length, in UTF-16 code units.
#define SWKBD_MAX_BUTTON_TEXT_LEN 16
/// Maximum hint text length, in UTF-16 code units.
#define SWKBD_MAX_HINT_TEXT_LEN 64
/// Maximum filter callback error message length, in UTF-16 code units.
#define SWKBD_MAX_CALLBACK_MSG_LEN 256

/// Keyboard types
enum class SwkbdType : u32 {
    Normal,  ///< Normal keyboard with several pages (QWERTY/accents/symbol/mobile)
    QWERTY,  ///< QWERTY keyboard only.
    Numpad,  ///< Number pad.
    Western, ///< On JPN systems, a text keyboard without Japanese input capabilities,
             /// otherwise same as SWKBD_TYPE_NORMAL.
};

/// Keyboard dialog buttons.
enum class SwkbdButtonConfig : u32 {
    SingleButton, ///< Ok button
    DualButton,   ///< Cancel | Ok buttons
    TripleButton, ///< Cancel | I Forgot | Ok buttons
    NoButton,     ///< No button (returned by swkbdInputText in special cases)
};

/// Accepted input types.
enum class SwkbdValidInput : u32 {
    Anything,         ///< All inputs are accepted.
    NotEmpty,         ///< Empty inputs are not accepted.
    NotEmptyNotBlank, ///< Empty or blank inputs (consisting solely of whitespace) are not
                      /// accepted.
    NotBlank, ///< Blank inputs (consisting solely of whitespace) are not accepted, but empty
              /// inputs are.
    FixedLen, ///< The input must have a fixed length (specified by maxTextLength in
              /// swkbdInit).
};

/// Keyboard password modes.
enum class SwkbdPasswordMode : u32 {
    None = 0,  ///< Characters are not concealed.
    Hide,      ///< Characters are concealed immediately.
    HideDelay, ///< Characters are concealed a second after they've been typed.
};

/// Keyboard input filtering flags.
enum SwkbdFilter {
    SwkbdFilter_Digits =
        1, ///< Disallow the use of more than a certain number of digits (0 or more)
    SwkbdFilter_At = 1 << 1,        ///< Disallow the use of the @ sign.
    SwkbdFilter_Percent = 1 << 2,   ///< Disallow the use of the % sign.
    SwkbdFilter_Backslash = 1 << 3, ///< Disallow the use of the \ sign.
    SwkbdFilter_Profanity = 1 << 4, ///< Disallow profanity using Nintendo's profanity filter.
    SwkbdFilter_Callback = 1 << 5,  ///< Use a callback in order to check the input.
};

/// Keyboard features.
enum class SwkbdFeatures {
    Parental = 1,             ///< Parental PIN mode.
    DarkenTopScreen = 1 << 1, ///< Darken the top screen when the keyboard is shown.
    PredictiveInput =
        1 << 2,             ///< Enable predictive input (necessary for Kanji input in JPN systems).
    MultiLine = 1 << 3,     ///< Enable multiline input.
    FixedWidth = 1 << 4,    ///< Enable fixed-width mode.
    AllowHome = 1 << 5,     ///< Allow the usage of the HOME button.
    AllowReset = 1 << 6,    ///< Allow the usage of a software-reset combination.
    AllowPower = 1 << 7,    ///< Allow the usage of the POWER button.
    DefaultQWERTY = 1 << 9, ///< Default to the QWERTY page when the keyboard is shown.
};

/// Keyboard filter callback return values.
enum class SwkbdCallbackResult : u32 {
    Ok,       ///< Specifies that the input is valid.
    Close,    ///< Displays an error message, then closes the keyboard.
    Continue, ///< Displays an error message and continues displaying the keyboard.
};

/// Keyboard return values.
enum class SwkbdResult : s32 {
    None = -1,         ///< Dummy/unused.
    InvalidInput = -2, ///< Invalid parameters to swkbd.
    OutOfMem = -3,     ///< Out of memory.

    D0Click = 0, ///< The button was clicked in 1-button dialogs.
    D1Click0,    ///< The left button was clicked in 2-button dialogs.
    D1Click1,    ///< The right button was clicked in 2-button dialogs.
    D2Click0,    ///< The left button was clicked in 3-button dialogs.
    D2Click1,    ///< The middle button was clicked in 3-button dialogs.
    D2Click2,    ///< The right button was clicked in 3-button dialogs.

    HomePressed = 10, ///< The HOME button was pressed.
    ResetPressed,     ///< The soft-reset key combination was pressed.
    PowerPressed,     ///< The POWER button was pressed.

    ParentalOk = 20, ///< The parental PIN was verified successfully.
    ParentalFail,    ///< The parental PIN was incorrect.

    BannedInput = 30, ///< The filter callback returned SWKBD_CALLBACK_CLOSE.
};

enum class ValidationError {
    None,
    // Button Selection
    ButtonOutOfRange,
    // Configured Filters
    DigitNotAllowed,
    AtSignNotAllowed,
    PercentNotAllowed,
    BackslashNotAllowed,
    ProfanityNotAllowed,
    CallbackFailed,
    // Allowed Input Type
    FixedLengthRequired,
    MaxLengthExceeded,
    BlankInputNotAllowed,
    EmptyInputNotAllowed,
    NewLineNotAllowed,
    InputNotNumber,
};

struct SoftwareKeyboardConfig {
    SwkbdType type;
    SwkbdButtonConfig num_buttons_m1;
    SwkbdValidInput valid_input;
    SwkbdPasswordMode password_mode;
    s32 is_parental_screen;
    s32 darken_top_screen;
    u32 filter_flags;
    u32 save_state_flags;
    u16 max_text_length; ///< Maximum length of the input text
    u16 dict_word_count;
    u16 max_digits;
    u16 button_text[SWKBD_MAX_BUTTON][SWKBD_MAX_BUTTON_TEXT_LEN + 1];
    u16 numpad_keys[2];
    u16 hint_text[SWKBD_MAX_HINT_TEXT_LEN + 1]; ///< Text to display when asking the user for input
    bool predictive_input;
    bool multiline;
    bool fixed_width;
    bool allow_home;
    bool allow_reset;
    bool allow_power;
    bool unknown;
    bool default_qwerty;
    bool button_submits_text[4];
    u16 language;

    u32 initial_text_offset; ///< Offset of the default text in the output SharedMemory
    u32 dict_offset;
    u32 initial_status_offset;
    u32 initial_learning_offset;
    u32 shared_memory_size; ///< Size of the SharedMemory
    u32 version;

    SwkbdResult return_code;

    u32 status_offset;
    u32 learning_offset;

    u32 text_offset; ///< Offset in the SharedMemory where the output text starts
    u16 text_length; ///< Length in characters of the output text

    int callback_result;
    u16 callback_msg[SWKBD_MAX_CALLBACK_MSG_LEN + 1];
    bool skip_at_check;
    INSERT_PADDING_BYTES(0xAB);
};

/**
 * The size of this structure (0x400) has been verified via reverse engineering of multiple games
 * that use the software keyboard.
 */
static_assert(sizeof(SoftwareKeyboardConfig) == 0x400, "SoftwareKeyboardConfig size is wrong");

const std::string default_button_text[][3]{
    {
        "OK",
    },
    {
        "Cancel",
        "OK",
    },
    {
        "Cancel",
        "OK",
        "I Forgot",
    },
};

const SwkbdResult results[][3]{
    {
        SwkbdResult::D0Click,
    },
    {
        SwkbdResult::D1Click0,
        SwkbdResult::D1Click1,
    },
    {
        SwkbdResult::D2Click0,
        SwkbdResult::D2Click1,
        SwkbdResult::D2Click2,
    },
};

ValidationError ValidateFilters(const SoftwareKeyboardConfig& config, const std::string& input);

ValidationError ValidateInput(const SoftwareKeyboardConfig& config, const std::string& input);

ValidationError ValidateButton(const SoftwareKeyboardConfig& config, u8 button);

class SoftwareKeyboard final : public Applet {
public:
    SoftwareKeyboard(Service::APT::AppletId id, std::weak_ptr<Service::APT::AppletManager> manager)
        : Applet(id, std::move(manager)) {}

    ResultCode ReceiveParameter(const Service::APT::MessageParameter& parameter) override;
    ResultCode StartImpl(const Service::APT::AppletStartupParameter& parameter) override;
    void Update() override;

    /**
     * Sends the LibAppletClosing signal to the application,
     * along with the relevant data buffers.
     */
    void Finalize();

private:
    /// This SharedMemory will be created when we receive the LibAppJustStarted message.
    /// It holds the framebuffer info retrieved by the application with
    /// GSPGPU::ImportDisplayCaptureInfo
    Kernel::SharedPtr<Kernel::SharedMemory> framebuffer_memory;

    /// SharedMemory where the output text will be stored
    Kernel::SharedPtr<Kernel::SharedMemory> text_memory;

    /// Configuration of this instance of the SoftwareKeyboard, as received from the application
    SoftwareKeyboardConfig config;
};

class SwkbdFactory {
public:
    bool IsRegistered(const std::string& name) const {
        auto it = callbacks.find(name);
        return it != callbacks.end();
    }

    void Register(std::string name,
                  std::function<void(SoftwareKeyboardConfig&, std::u16string&)> callback) {
        callbacks.emplace(std::move(name), std::move(callback));
    }

    void Launch(const std::string& name, SoftwareKeyboardConfig& config, std::u16string& text) {
        auto it = callbacks.find(name);
        if (it != callbacks.end())
            it->second(config, text);
    }

private:
    std::map<std::string, std::function<void(SoftwareKeyboardConfig&, std::u16string&)>> callbacks;
};
} // namespace Applets
} // namespace HLE
