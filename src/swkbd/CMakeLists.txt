set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

add_executable(citra-kbd
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

set(UIS
    mainwindow.ui
)

qt5_wrap_ui(UI_HDRS ${UIS})

target_sources(citra-kbd
    PRIVATE
        ${UI_HDRS}
        ${UIS}
)

create_target_directory_groups(citra-kbd)

target_link_libraries(citra-kbd PRIVATE Qt5::Widgets)
target_link_libraries(citra-kbd PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

if(UNIX AND NOT APPLE)
    install(TARGETS citra-kbd RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
endif()

if (MSVC)
    include(CopyCitraQt5Deps)
    copy_citra_Qt5_deps(citra-kbd)
endif()
