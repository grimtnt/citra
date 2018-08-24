// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include "common/common_types.h"
#include "core/core.h"
#include "video_core/rasterizer_interface.h"

class EmuWindow;

class RendererBase : NonCopyable {
public:
    /// Used to reference a framebuffer
    enum kFramebuffer { kFramebuffer_VirtualXFB = 0, kFramebuffer_EFB, kFramebuffer_Texture };

    explicit RendererBase(EmuWindow& window);
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

    void RefreshRasterizerSetting();

protected:
    EmuWindow& render_window; ///< Reference to the render window handle.
    std::unique_ptr<VideoCore::RasterizerInterface> rasterizer;

private:
    bool opengl_rasterizer_active = false;
};
