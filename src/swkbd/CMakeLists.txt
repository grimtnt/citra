set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

add_executable(citra-kbd
    Info.plist
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

set(UIS
    mainwindow.ui
)

file(GLOB_RECURSE ICONS ${CMAKE_SOURCE_DIR}/dist/icons/*)

qt5_wrap_ui(UI_HDRS ${UIS})

target_sources(citra-kbd
    PRIVATE
        ${ICONS}
        ${UI_HDRS}
        ${UIS}
)

if (APPLE)
    set(MACOSX_ICON "../../dist/citra.icns")
    set_source_files_properties(${MACOSX_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    target_sources(citra-kbd PRIVATE ${MACOSX_ICON})
    set_target_properties(citra-kbd PROPERTIES MACOSX_BUNDLE TRUE)
    set_target_properties(citra-kbd PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif()

create_target_directory_groups(citra-kbd)

target_link_libraries(citra-kbd PRIVATE Qt5::Widgets)
target_link_libraries(citra-kbd PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

if(UNIX AND NOT APPLE)
    install(TARGETS citra-kbd RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
endif()
