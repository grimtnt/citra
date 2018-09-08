// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstddef>
#include "audio_core/dsp_interface.h"
#include "audio_core/sink.h"
#include "audio_core/sink_details.h"
#include "common/assert.h"
#include "core/settings.h"

namespace AudioCore {

DspInterface::DspInterface() = default;
DspInterface::~DspInterface() = default;

void DspInterface::SetSink(const std::string& sink_id, const std::string& audio_device) {
    const SinkDetails& sink_details = GetSinkDetails(sink_id);
    sink = sink_details.factory(audio_device);
    sink->SetCallback(
        [this](s16* buffer, std::size_t num_frames) { OutputCallback(buffer, num_frames); });
    time_stretcher.SetOutputSampleRate(sink->GetNativeSampleRate());
}

Sink& DspInterface::GetSink() {
    ASSERT(sink);
    return *sink.get();
}

void DspInterface::EnableStretching(bool enable) {
    if (perform_time_stretching == enable)
        return;

    if (!enable) {
        FlushResidualStretcherAudio();
    }
    perform_time_stretching = enable;
}

void DspInterface::OutputFrame(StereoFrame16& frame) {
    if (!sink)
        return;

    // Implementation of the hardware volume slider with a dynamic range of 60 dB
    double volume_scale_factor{std::exp(6.90775 * Settings::values.volume) * 0.001};
    for (std::size_t i{}; i < frame.size(); i++) {
        frame[i][0] = static_cast<s16>(frame[i][0] * volume_scale_factor);
        frame[i][1] = static_cast<s16>(frame[i][1] * volume_scale_factor);
    }

    fifo.Push(frame.data(), frame.size());
}

void DspInterface::FlushResidualStretcherAudio() {}

void DspInterface::OutputCallback(s16* buffer, size_t num_frames) {
    const size_t frames_written{fifo.Pop(buffer, num_frames)};

    if (frames_written > 0) {
        std::memcpy(&last_frame[0], buffer + 2 * (frames_written - 1), 2 * sizeof(s16));
    }

    // Hold last emitted frame; this prevents popping.
    for (size_t i{frames_written}; i < num_frames; i++) {
        std::memcpy(buffer + 2 * i, &last_frame[0], 2 * sizeof(s16));
    }
}

} // namespace AudioCore
