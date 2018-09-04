# Enable modules to include each other's files
include_directories(.)

if ($ENV{APPVEYOR})
include_directories("C:/projects/citra/WpdPack/Include")
endif()

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(video_core)
add_subdirectory(audio_core)
add_subdirectory(input_common)
if (ENABLE_QT)
    add_subdirectory(citra_qt)
endif()