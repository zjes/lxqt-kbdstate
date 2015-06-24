cmake_minimum_required(VERSION 2.8.11)
cmake_policy(VERSION 2.8)

include(cmake/lxqt-panel-plugin.cmake)

init_plugin("lxqt-kbdstate" VERSION 0.0.1)

find_package(KF5WindowSystem REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XKB_COMMON_X11 QUIET xkbcommon-x11)

if(NOT XKB_COMMON_X11_FOUND)
    message(STATUS "XkbCommon X11 was not found")
else()
    message(STATUS "XkbCommon X11 was found")
    find_package(Qt5 COMPONENTS X11Extras Xml)
    pkg_check_modules(XCB_XCB xcb-xkb)

    set(ADDSRC
        src/x11/kbdlayout.h
        src/x11/kbdlayout.cpp
    )

    set(ADDLIBS
        ${XKB_COMMON_X11_LIBRARIES}
        ${XCB_XCB_LIBRARIES}
        Qt5::Xml
    )

    add_definitions(-DX11_ENABLED)
endif()

add_plugin(${PROJECT_NAME}
    SOURCES
        kbdstate-plugin.cpp
        src/kbdstate.h
        src/kbdstate.cpp
        src/settings.h
        src/settings.cpp
        src/content.h
        src/content.cpp
        src/kbdlayout.h
        src/kbdinfo.h
        src/kbdkeeper.h
        src/kbdkeeper.cpp
        src/kbdwatcher.h
        src/kbdwatcher.cpp
        src/controls.h
        src/kbdstateconfig.h
        src/kbdstateconfig.cpp
        ${ADDSRC}
    UI  src/kbdstateconfig.ui
    LIBS
        ${ADDLIBS}
        ${LXQT_LIBRARIES}
        Qt5::Widgets
        KF5::WindowSystem
)

install_plugin(${PROJECT_NAME})
