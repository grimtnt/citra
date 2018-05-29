# Generate cpp with Git revision from template
# Also if this is a AppVeyor build, add the build number to the scm_rev file as well
set(BUILD_VERSION "0")
if ($ENV{APPVEYOR})
  set(BUILD_VERSION $ENV{APPVEYOR_BUILD_NUMBER})
  if (BUILD_VERSION)
    set(BUILD_FULLNAME "#${BUILD_VERSION} ")
  else()
    set(BUILD_FULLNAME "")
  endif()
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/scm_rev.cpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/scm_rev.cpp" @ONLY)

add_library(common STATIC
    alignment.h
    announce_multiplayer_room.h
    assert.h
    bit_field.h
    bit_set.h
    break_points.cpp
    break_points.h
    chunk_file.h
    cityhash.cpp
    cityhash.h
    code_block.h
    color.h
    common_funcs.h
    common_paths.h
    common_types.h
    file_util.cpp
    file_util.h
    hash.h
    linear_disk_cache.h
    logging/backend.cpp
    logging/backend.h
    logging/filter.cpp
    logging/filter.h
    logging/log.h
    logging/text_formatter.cpp
    logging/text_formatter.h
    math_util.h
    memory_util.cpp
    memory_util.h
    misc.cpp
    param_package.cpp
    param_package.h
    quaternion.h
    scm_rev.cpp
    scm_rev.h
    scope_exit.h
    string_util.cpp
    string_util.h
    swap.h
    synchronized_wrapper.h
    thread.cpp
    thread.h
    thread_queue_list.h
    threadsafe_queue.h
    timer.cpp
    timer.h
    vector_math.h
)

if(ARCHITECTURE_x86_64)
    target_sources(common
        PRIVATE
            x64/cpu_detect.cpp

            x64/cpu_detect.h
            x64/xbyak_abi.h
            x64/xbyak_util.h
    )
endif()

create_target_directory_groups(common)

target_link_libraries(common PUBLIC Boost::boost fmt)
if (ARCHITECTURE_x86_64)
    target_link_libraries(common PRIVATE xbyak)
endif()
