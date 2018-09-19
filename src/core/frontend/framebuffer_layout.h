// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/math_util.h"

namespace Layout {

/// Describes the layout of the window framebuffer (size and top/bottom screen positions)
struct FramebufferLayout {
    unsigned width;
    unsigned height;
    bool top_screen_enabled;
    bool bottom_screen_enabled;
    MathUtil::Rectangle<unsigned> top_screen;
    MathUtil::Rectangle<unsigned> bottom_screen;

    /**
     * Returns the ration of pixel size of the top screen, compared to the native size of the 3DS
     * screen.
     */
    u16 GetScalingRatio() const;
};

/**
 * Factory method for constructing a FramebufferLayout
 * @param width Window framebuffer width in pixels
 * @param height Window framebuffer height in pixels
 * @param is_swapped if true, the bottom screen will be displayed above the top screen
 * @return Newly created FramebufferLayout object with default screen regions initialized
 */
FramebufferLayout GetLayout(unsigned width, unsigned height);

} // namespace Layout
