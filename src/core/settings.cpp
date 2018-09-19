// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "audio_core/hle/hle.h"
#include "core/core.h"
#include "core/hle/service/hid/hid.h"
#include "core/hle/service/ir/ir.h"
#include "core/hle/service/mic/mic_u.h"
#include "core/settings.h"
#include "video_core/renderer/renderer.h"
#include "video_core/video_core.h"

namespace Settings {

Values values = {};

void Apply() {
    VideoCore::g_hw_shaders_enabled = values.use_hw_shaders;
    VideoCore::g_accurate_shaders_enabled = values.accurate_shaders;

    if (VideoCore::g_renderer) {
        VideoCore::g_renderer->UpdateCurrentFramebufferLayout();
    }

    if (Core::System::GetInstance().IsPoweredOn()) {
        Core::DSP().UpdateSink();
    }

    Service::HID::ReloadInputDevices();
    Service::IR::ReloadInputDevices();
    Service::CAM::ReloadCameraDevices();
    Service::MIC::ReloadDevice();
}

template <typename T>
void LogSetting(const std::string& name, const T& value) {
    LOG_INFO(Config, "{}: {}", name, value);
}

void LogSettings() {
    LOG_INFO(Config, "Citra Configuration:");
    LogSetting("ControlPanel_Factor3d", Settings::values.factor_3d);
    LogSetting("Renderer_UseHwShaders", Settings::values.use_hw_shaders);
    LogSetting("Renderer_AccurateShaders", Settings::values.accurate_shaders);
    LogSetting("Audio_OutputDevice", Settings::values.output_device);
    LogSetting("Audio_InputDevice", Settings::values.input_device);
    using namespace Service::CAM;
    LogSetting("Camera_OuterRightName", Settings::values.camera_name[OuterRightCamera]);
    LogSetting("Camera_OuterRightConfig", Settings::values.camera_config[OuterRightCamera]);
    LogSetting("Camera_OuterRightFlip", Settings::values.camera_flip[OuterRightCamera]);
    LogSetting("Camera_InnerName", Settings::values.camera_name[InnerCamera]);
    LogSetting("Camera_InnerConfig", Settings::values.camera_config[InnerCamera]);
    LogSetting("Camera_InnerFlip", Settings::values.camera_flip[InnerCamera]);
    LogSetting("Camera_OuterLeftName", Settings::values.camera_name[OuterLeftCamera]);
    LogSetting("Camera_OuterLeftConfig", Settings::values.camera_config[OuterLeftCamera]);
    LogSetting("Camera_OuterLeftFlip", Settings::values.camera_flip[OuterLeftCamera]);
    LogSetting("System_RegionValue", Settings::values.region_value);
    LogSetting("Hacks_PriorityBoost", Settings::values.priority_boost);
    LogSetting("Hacks_Ticks", Settings::values.ticks);
    LogSetting("Hacks_TicksMode", static_cast<int>(Settings::values.ticks_mode));
    LogSetting("Hacks_UseBos", Settings::values.use_bos);
}

} // namespace Settings
