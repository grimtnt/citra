// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <atomic>
#include <memory>
#include "core/core.h"
#include "core/frontend/emu_window.h"

class EmuWindow;
class Renderer;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Video Core namespace

namespace VideoCore {

extern std::unique_ptr<Renderer> g_renderer; ///< Renderer plugin

extern std::atomic<bool> g_hw_shaders_enabled;
extern std::atomic<bool> g_accurate_shaders_enabled;

// Screenshot
extern std::atomic<bool> g_renderer_screenshot_requested;
extern void* g_screenshot_bits;
extern std::function<void()> g_screenshot_complete_callback;

/// Initialize the video core
Core::System::ResultStatus Init(EmuWindow& emu_window);

/// Shutdown the video core
void Shutdown();

/// Request a screenshot of the next frame
void RequestScreenshot(void* data, std::function<void()> callback);

} // namespace VideoCore
