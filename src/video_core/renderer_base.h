// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include "core/core.h"
#include "video_core/rasterizer_interface.h"

class EmuWindow;

class RendererBase : NonCopyable {
public:
    explicit RendererBase(EmuWindow& window,
                          std::unique_ptr<VideoCore::RasterizerInterface> rasterizer);
    virtual ~RendererBase();

    /// Swap buffers (render frame)
    virtual void SwapBuffers() = 0;

    /// Initialize the renderer
    virtual Core::System::ResultStatus Init() = 0;

    /// Updates the framebuffer layout of the contained render window handle.
    void UpdateCurrentFramebufferLayout();

    VideoCore::RasterizerInterface* Rasterizer() const {
        return rasterizer.get();
    }

    EmuWindow& GetRenderWindow() const {
        return render_window;
    }

protected:
    EmuWindow& render_window; ///< Reference to the render window handle.
    std::unique_ptr<VideoCore::RasterizerInterface> rasterizer;
};
