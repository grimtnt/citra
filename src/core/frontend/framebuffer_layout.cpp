// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cmath>

#include "common/assert.h"
#include "core/3ds.h"
#include "core/frontend/framebuffer_layout.h"
#include "core/settings.h"

namespace Layout {

static const float TOP_SCREEN_ASPECT_RATIO{static_cast<float>(Core::kScreenTopHeight) /
                                           Core::kScreenTopWidth};
static const float BOT_SCREEN_ASPECT_RATIO{static_cast<float>(Core::kScreenBottomHeight) /
                                           Core::kScreenBottomWidth};

u16 FramebufferLayout::GetScalingRatio() const {
    return static_cast<u16>(((top_screen.GetWidth() - 1) / Core::kScreenTopWidth) + 1);
}

// Finds the largest size subrectangle contained in window area that is confined to the aspect ratio
template <class T>
static MathUtil::Rectangle<T> maxRectangle(MathUtil::Rectangle<T> window_area,
                                           float screen_aspect_ratio) {
    float scale = std::min(static_cast<float>(window_area.GetWidth()),
                           window_area.GetHeight() / screen_aspect_ratio);
    return MathUtil::Rectangle<T>{0, 0, static_cast<T>(std::round(scale)),
                                  static_cast<T>(std::round(scale * screen_aspect_ratio))};
}

FramebufferLayout GetLayout(unsigned width, unsigned height) {
    ASSERT(width > 0);
    ASSERT(height > 0);

    FramebufferLayout res{width, height, true, true, {}, {}};
    // Default layout gives equal screen sizes to the top and bottom screen
    MathUtil::Rectangle<unsigned> screen_window_area{0, 0, width, height / 2};
    MathUtil::Rectangle<unsigned> top_screen{
        maxRectangle(screen_window_area, TOP_SCREEN_ASPECT_RATIO)};
    MathUtil::Rectangle<unsigned> bot_screen{
        maxRectangle(screen_window_area, BOT_SCREEN_ASPECT_RATIO)};

    float window_aspect_ratio{static_cast<float>(height) / width};
    // both screens height are taken into account by multiplying by 2
    float emulation_aspect_ratio{TOP_SCREEN_ASPECT_RATIO * 2};

    if (window_aspect_ratio < emulation_aspect_ratio) {
        // Apply borders to the left and right sides of the window.
        top_screen =
            top_screen.TranslateX((screen_window_area.GetWidth() - top_screen.GetWidth()) / 2);
        bot_screen =
            bot_screen.TranslateX((screen_window_area.GetWidth() - bot_screen.GetWidth()) / 2);
    } else {
        // Window is narrower than the emulation content => apply borders to the top and bottom
        // Recalculate the bottom screen to account for the width difference between top and bottom
        screen_window_area = {0, 0, width, top_screen.GetHeight()};
        bot_screen = maxRectangle(screen_window_area, BOT_SCREEN_ASPECT_RATIO);
        bot_screen = bot_screen.TranslateX((top_screen.GetWidth() - bot_screen.GetWidth()) / 2);
        top_screen = top_screen.TranslateY(height / 2 - top_screen.GetHeight());
    }
    // Move the top screen to the bottom if we are swapped.
    res.top_screen = top_screen;
    res.bottom_screen = bot_screen.TranslateY(height / 2);
    return res;
}

} // namespace Layout
