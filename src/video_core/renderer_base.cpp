// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <utility>
#include "core/frontend/emu_window.h"
#include "video_core/renderer_base.h"

RendererBase::RendererBase(EmuWindow& window,
                           std::unique_ptr<VideoCore::RasterizerInterface> rasterizer)
    : render_window{window}, rasterizer{std::move(rasterizer)} {}
RendererBase::~RendererBase() = default;

void RendererBase::UpdateCurrentFramebufferLayout() {
    const Layout::FramebufferLayout& layout{render_window.GetFramebufferLayout()};
    render_window.UpdateCurrentFramebufferLayout(layout.width, layout.height);
}
