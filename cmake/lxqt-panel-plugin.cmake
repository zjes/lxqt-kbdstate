include(GNUInstallDirs)

find_package(Qt5 COMPONENTS Core Widgets)
find_package(KF5GuiAddons REQUIRED)
find_package(PkgConfig    REQUIRED QUIET)
find_package(lxqt         REQUIRED)

include(${LXQT_USE_FILE})

macro(init_plugin PROJECT)
    cmake_parse_arguments(ARG "" "VERSION;CHANGELOG" "" ${ARGN})
    project(${PROJECT})

    if(ARG_VERSION)
        if(ARG_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
            set(VERSION_MAJOR ${CMAKE_MATCH_1})
            set(VERSION_MINOR ${CMAKE_MATCH_2})
            set(VERSION_PATCH ${CMAKE_MATCH_3})
            message(STATUS "${PROJECT_NAME}: Plugin version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
        else()
            message(FATAL_ERROR "${PROJECT_NAME}: Wrong format of the version '${ARG_VERSION}' (must be X.X.X)")
        endif()
    elseif(ARG_CHANGELOG)
        if(IS_ABSOLUTE "${ARG_CHANGELOG}")
            set(ABSCHANGELOG "${ARG_CHANGELOG}")
        else()
            set(ABSCHANGELOG "${CMAKE_SOURCE_DIR}/${ARG_CHANGELOG}")
        endif()

        if(NOT EXISTS ${ABSCHANGELOG})
            message(FATAL_ERROR "${PROJECT_NAME}: Changelog at '${ABSCHANGELOG}' doesn't exists.")
        endif()

        file(STRINGS ${ABSCHANGELOG} LINE LIMIT_COUNT 1)
        if(LINE MATCHES "\\(([0-9]+)\\.([0-9]+)\\.([0-9]+)\\)")
            set(VERSION_MAJOR ${CMAKE_MATCH_1})
            set(VERSION_MINOR ${CMAKE_MATCH_2})
            set(VERSION_PATCH ${CMAKE_MATCH_3})
            message(STATUS "${PROJECT_NAME}: Plugin version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
        else()
            message(FATAL_ERROR "${PROJECT_NAME}: Wrong format of the version from changelog")
        endif()
    else()
        message(FATAL_ERROR "Version or change log must be specified")
    endif()

    lxqt_set_compiler_flags("-g -fPIC -Wall -Wextra -pedantic -Wformat=2 -Wnon-virtual-dtor -std=c++11")
    lxqt_set_linker_flags("-Wl,--as-needed -z relro -z defs" SHARED MODULE)

    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)

    set(PLUGIN_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/lxqt-panel")
    message(STATUS "${PROJECT_NAME}: Plugin location: ${PLUGIN_DIR}")
endmacro()

macro(add_plugin TARGET)
    cmake_parse_arguments(ARGS "" "" "SOURCES;LIBS;QRC;UI" ${ARGN})

    if(ARGS_QRC)
        qt5_add_resources(QRC ${ARGS_QRC})
    endif()

    if(ARGS_UI)
        qt5_wrap_ui(UIC ${ARGS_UI})
    endif()

    add_library(${TARGET}
        SHARED
        ${ARGS_SOURCES}
        ${QRC}
        ${UIC}
    )

    if(ARGS_LIBS)
        target_link_libraries(${TARGET} ${ARGS_LIBS})
    endif()
endmacro()

macro(install_plugin TARGET)
    install(
        TARGETS     ${TARGET}
        DESTINATION ${PLUGIN_DIR}
    )

    if(EXISTS "${CMAKE_SOURCE_DIR}/${TARGET}.desktop.in")
        configure_file("${CMAKE_SOURCE_DIR}/${TARGET}.desktop.in" "${CMAKE_BINARY_DIR}/${TARGET}.desktop" @ONLY)
        install(
            FILES       "${CMAKE_BINARY_DIR}/${TARGET}.desktop"
            DESTINATION "${LXQT_SHARE_DIR}/lxqt-panel"
        )
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/translation")
        message(${LXQT_TRANSLATIONS_DIR})
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/themes/")
        install(
            DIRECTORY   "${CMAKE_SOURCE_DIR}/themes/"
            DESTINATION "${LXQT_SHARE_DIR}/themes/"
        )
    endif()
endmacro()

macro(lxqt_set_linker_flags FLAGS)
    cmake_parse_arguments(ARG "SHARED;EXE;MODULE;ALL" "" "" ${ARGN})
    if(ARG_SHARED)
        set(CMAKE_SHARED_LINKER_FLAGS "${FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
    endif()

    if(ARG_EXE)
        set(CMAKE_EXE_LINKER_FLAGS    "${FLAGS} ${CMAKE_EXE_LINKER_FLAGS}")
    endif()

    if(ARG_MODULE)
        set(CMAKE_MODULE_LINKER_FLAGS "${FLAGS} ${CMAKE_MODULE_LINKER_FLAGS}")
    endif()

    if(NOT ARG_SHARED AND NOT ARG_EXE AND NOT ARG_MODULE)
        set(CMAKE_SHARED_LINKER_FLAGS "${FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS    "${FLAGS} ${CMAKE_EXE_LINKER_FLAGS}")
        set(CMAKE_MODULE_LINKER_FLAGS "${FLAGS} ${CMAKE_MODULE_LINKER_FLAGS}")
    endif()
endmacro()

macro(lxqt_set_compiler_flags FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
endmacro()
