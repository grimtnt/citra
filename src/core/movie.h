// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <string>
#include <tuple>
#include "common/common_types.h"

namespace Service {
namespace HID {
struct AccelerometerDataEntry;
struct GyroscopeDataEntry;
struct PadState;
struct TouchDataEntry;
} // namespace HID
namespace IR {
struct ExtraHIDResponse;
union PadState;
} // namespace IR
} // namespace Service

namespace Core {
struct CTMHeader;
struct ControllerState;
enum class PlayMode;

class Movie {
public:
    /**
     * Gets the instance of the Movie singleton class.
     * @returns Reference to the instance of the Movie singleton class.
     */
    static Movie& GetInstance() {
        return s_instance;
    }

    /**
     * Sets the file to play and restarts the movie system if initialized
     * @param path the path of the file
     */
    void SetPlayFile(const std::string& path) {
        play_file = path;

        if (IsInitialized()) {
            Shutdown();
            Init();
        }
    }

    /**
     * Sets the file to record and restarts the movie system if initialized
     * @param path the path of the file
     */
    void SetRecordFile(const std::string& path) {
        record_file = path;

        if (IsInitialized()) {
            Shutdown();
            Init();
        }
    }

    /**
     * Gets a tuple of play and record file
     * @returns Tuple of play file and record file
     */
    std::tuple<std::string, std::string> GetFiles() {
        return std::make_tuple(play_file, record_file);
    }

    void Init();

    void Shutdown();

    bool IsInitialized() {
        return initialized;
    }

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandlePadAndCircleStatus(Service::HID::PadState& pad_state, s16& circle_pad_x,
                                  s16& circle_pad_y);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleTouchStatus(Service::HID::TouchDataEntry& touch_data);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleAccelerometerStatus(Service::HID::AccelerometerDataEntry& accelerometer_data);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleGyroscopeStatus(Service::HID::GyroscopeDataEntry& gyroscope_data);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleIrRst(Service::IR::PadState& pad_state, s16& c_stick_x, s16& c_stick_y);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleExtraHidResponse(Service::IR::ExtraHIDResponse& extra_hid_response);

    bool IsPlayingInput();

    bool IsRecordingInput();

private:
    static Movie s_instance;

    void CheckInputEnd();

    template <typename... Targs>
    void Handle(Targs&... Fargs);

    void Play(Service::HID::PadState& pad_state, s16& circle_pad_x, s16& circle_pad_y);
    void Play(Service::HID::TouchDataEntry& touch_data);
    void Play(Service::HID::AccelerometerDataEntry& accelerometer_data);
    void Play(Service::HID::GyroscopeDataEntry& gyroscope_data);
    void Play(Service::IR::PadState& pad_state, s16& c_stick_x, s16& c_stick_y);
    void Play(Service::IR::ExtraHIDResponse& extra_hid_response);

    void Record(const ControllerState& controller_state);
    void Record(const Service::HID::PadState& pad_state, const s16& circle_pad_x,
                const s16& circle_pad_y);
    void Record(const Service::HID::TouchDataEntry& touch_data);
    void Record(const Service::HID::AccelerometerDataEntry& accelerometer_data);
    void Record(const Service::HID::GyroscopeDataEntry& gyroscope_data);
    void Record(const Service::IR::PadState& pad_state, const s16& c_stick_x, const s16& c_stick_y);
    void Record(const Service::IR::ExtraHIDResponse& extra_hid_response);

    bool ValidateHeader(const CTMHeader& header);

    void SaveMovie();

    bool initialized = false;

    PlayMode play_mode;
    std::vector<u8> recorded_input;
    size_t current_byte = 0;
    std::string play_file;
    std::string record_file;
};
} // namespace Core
