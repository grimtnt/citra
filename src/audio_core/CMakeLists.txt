add_library(audio_core STATIC
    audio_types.h
    codec.cpp
    codec.h
    hle/common.h
    hle/filter.cpp
    hle/filter.h
    hle/hle.cpp
    hle/hle.h
    hle/mixers.cpp
    hle/mixers.h
    hle/shared_memory.h
    hle/source.cpp
    hle/source.h
    interpolate.cpp
    interpolate.h
    sink.cpp
    sink.h
    time_stretch.cpp
    time_stretch.h
)

create_target_directory_groups(audio_core)

target_link_libraries(audio_core PUBLIC common core)
target_link_libraries(audio_core PRIVATE SoundTouch cubeb)
