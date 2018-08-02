add_library(input_common STATIC
    analog_from_button.cpp
    analog_from_button.h
    keyboard.cpp
    keyboard.h
    main.cpp
    main.h
    motion_emu.cpp
    motion_emu.h
    udp/client.cpp
    udp/client.h
    udp/protocol.cpp
    udp/protocol.h
    udp/udp.cpp
    udp/udp.h

    $<$<BOOL:${SDL2_FOUND}>:sdl/sdl.cpp sdl/sdl.h>
)

create_target_directory_groups(input_common)
target_compile_definitions(input_common PRIVATE BOOST_ERROR_CODE_HEADER_ONLY BOOST_SYSTEM_NO_LIB BOOST_DATE_TIME_NO_LIB BOOST_REGEX_NO_LIB)
target_link_libraries(input_common PUBLIC core PRIVATE common ${Boost_LIBRARIES})

if(SDL2_FOUND)
    target_link_libraries(input_common PRIVATE SDL2)
    target_compile_definitions(input_common PRIVATE HAVE_SDL2)
endif()
