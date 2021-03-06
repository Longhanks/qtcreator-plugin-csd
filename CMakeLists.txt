cmake_minimum_required(VERSION 3.16)

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)
    find_program(BREW_EXECUTABLE brew HINTS /usr/local/bin)
    if ("${BREW_EXECUTABLE}" STREQUAL "BREW_EXECUTABLE-NOTFOUND")
        message(STATUS "macOS: Homebrew not found.")
    else ()
        message(STATUS "macOS: Homebrew found at ${BREW_EXECUTABLE}.")
        execute_process(COMMAND ${BREW_EXECUTABLE} --prefix qt OUTPUT_VARIABLE QT_PREFIX_PATH)
        string(REGEX REPLACE "\n$" "" QT_PREFIX_PATH_STRIPPED "${QT_PREFIX_PATH}")
        if ("${QT_PREFIX_PATH}" STREQUAL "")
            message(STATUS "macOS: Homebrew: Qt not installed.")
        else ()
            list(APPEND CMAKE_PREFIX_PATH "${QT_PREFIX_PATH_STRIPPED}")
            message(STATUS "macOS: Homebrew Qt at ${QT_PREFIX_PATH_STRIPPED}.")
        endif ()
    endif ()
endif ()

set(QTCREATOR_SRC "" CACHE PATH "Path to Qt Creator sources")
if (NOT WIN32)
    set(QTCREATOR_BIN "" CACHE FILEPATH "Path to Qt Creator binary")
endif ()
set(QTCREATOR_VERSION "4.11.0" CACHE STRING "Target version of Qt Creator")

if (NOT EXISTS "${QTCREATOR_SRC}/src/qtcreatorplugin.pri")
    message(FATAL_ERROR "QTCREATOR_SRC must point to Qt Creator sources.")
endif ()

if (NOT WIN32 AND NOT EXISTS "${QTCREATOR_BIN}")
    message(FATAL_ERROR "QTCREATOR_BIN must point to the Qt Creator executable.")
endif ()

get_filename_component(QTCREATOR_BIN_DIR "${QTCREATOR_BIN}" DIRECTORY)

enable_language(CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0071 NEW)

find_package(Qt5 COMPONENTS Network Widgets REQUIRED)

project(csd LANGUAGES CXX VERSION 0.2.0)

configure_file("${CMAKE_SOURCE_DIR}/csd.json.in" "${CMAKE_CURRENT_BINARY_DIR}/csd.json")

add_library(${PROJECT_NAME} SHARED
    "${CMAKE_SOURCE_DIR}/csd.qrc"
    "${CMAKE_SOURCE_DIR}/src/csdtitlebar.cpp"
    "${CMAKE_SOURCE_DIR}/src/csdtitlebarbutton.cpp"
    "${CMAKE_SOURCE_DIR}/src/optionsdialog.cpp"
    "${CMAKE_SOURCE_DIR}/src/optionspage.cpp"
    "${CMAKE_SOURCE_DIR}/src/plugin.cpp"
    "${CMAKE_SOURCE_DIR}/src/settings.cpp"
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" AND NOT MSVC)
    list(APPEND COMPILER_WARNINGS
        "-Weverything"
        "-Wno-c++98-compat"
        "-Wno-c++98-compat-pedantic"
        "-Wno-padded"
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND COMPILER_WARNINGS
        "-Wall"
        "-Wcast-align"
        "-Wcast-qual"
        "-Wconversion"
        "-Wctor-dtor-privacy"
        "-Wdisabled-optimization"
        "-Wdouble-promotion"
        "-Wduplicated-branches"
        "-Wduplicated-cond"
        "-Wextra"
        "-Wformat=2"
        "-Winvalid-pch"
        "-Wlogical-op"
        "-Wmisleading-indentation"
        "-Wmissing-include-dirs"
        "-Wnoexcept"
        "-Wnon-virtual-dtor"
        "-Wnull-dereference"
        "-Wold-style-cast"
        "-Woverloaded-virtual"
        "-Wpedantic"
        "-Wpointer-arith"
        "-Wredundant-decls"
        "-Wshadow"
        "-Wshift-overflow=2"
        "-Wsign-conversion"
        "-Wsized-deallocation"
        "-Wtrampolines"
        "-Wundef"
        "-Wunused"
        "-Wuseless-cast"
        "-Wvector-operation-performance"
        "-Wwrite-strings"
        "-Wzero-as-null-pointer-constant"
        "-pedantic-errors"
        )
else ()
    foreach (flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/W3" "" ${flag_var} "${${flag_var}}")
    endforeach()
    list(APPEND COMPILER_WARNINGS
        "/W4"
    )
endif ()
string(REPLACE ";" " " COMPILER_WARNINGS_STR "${COMPILER_WARNINGS}")

get_target_property(${PROJECT_NAME}_SOURCES ${PROJECT_NAME} SOURCES)

foreach (${PROJECT_NAME}_SOURCE ${${PROJECT_NAME}_SOURCES})
    set_source_files_properties(${${PROJECT_NAME}_SOURCE} PROPERTIES COMPILE_FLAGS "${COMPILER_WARNINGS_STR}")
endforeach ()

set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON AUTORCC ON)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${Qt5Network_INCLUDE_DIRS}"
    "${Qt5Widgets_INCLUDE_DIRS}"
    "${QTCREATOR_SRC}/src/libs"
    "${QTCREATOR_SRC}/src/plugins"
    )

if (APPLE)
    find_library(CORE_LIB NAMES Core PATHS "${QTCREATOR_BIN_DIR}/../PlugIns")
    find_library(EXTENSIONSYSTEM_LIB NAMES ExtensionSystem PATHS "${QTCREATOR_BIN_DIR}/../Frameworks")
    find_library(PROJECTEXPLORER_LIB NAMES ProjectExplorer PATHS "${QTCREATOR_BIN_DIR}/../PlugIns")
    find_library(UTILS_LIB NAMES Utils PATHS "${QTCREATOR_BIN_DIR}/../Frameworks")
    set(QTNETWORK_LIB "-framework QtNetwork")
    set(QTWIDGETS_LIB "-framework QtWidgets")
    set(QTGUI_LIB "-framework QtGui")
    set(QTCORE_LIB "-framework QtCore")
    target_link_options(${PROJECT_NAME} PRIVATE
        -Wl,-install_name,@rpath/PlugIns/
        -compatibility_version "${QTCREATOR_VERSION}"
        -iframework "${QTCREATOR_BIN_DIR}/../Frameworks"
    )
elseif (UNIX)
    find_package(Qt5X11Extras REQUIRED)
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Qt5X11Extras_INCLUDE_DIRS})
    find_library(CORE_LIB NAMES Core PATHS "${QTCREATOR_BIN_DIR}/../lib/qtcreator/plugins")
    find_library(EXTENSIONSYSTEM_LIB NAMES ExtensionSystem PATHS "${QTCREATOR_BIN_DIR}/../lib/qtcreator")
    find_library(PROJECTEXPLORER_LIB NAMES ProjectExplorer PATHS "${QTCREATOR_BIN_DIR}/../lib/qtcreator/plugins")
    find_library(UTILS_LIB NAMES Utils PATHS "${QTCREATOR_BIN_DIR}/../lib/qtcreator")
    if (EXISTS "${QTCREATOR_BIN_DIR}/../lib/libQt5Core.so.5")
        set(QTNETWORK_LIB "${QTCREATOR_BIN_DIR}/../lib/libQt5Network.so.5")
        set(QTWIDGETS_LIB "${QTCREATOR_BIN_DIR}/../lib/libQt5Widgets.so.5")
        set(QTGUI_LIB "${QTCREATOR_BIN_DIR}/../lib/libQt5Gui.so.5")
        set(QTCORE_LIB "${QTCREATOR_BIN_DIR}/../lib/libQt5Core.so.5")
    else ()
        set(QTNETWORK_LIB "${QTCREATOR_BIN_DIR}/../lib/Qt/lib/libQt5Network.so.5")
        set(QTWIDGETS_LIB "${QTCREATOR_BIN_DIR}/../lib/Qt/lib/libQt5Widgets.so.5")
        set(QTGUI_LIB "${QTCREATOR_BIN_DIR}/../lib/Qt/lib/libQt5Gui.so.5")
        set(QTCORE_LIB "${QTCREATOR_BIN_DIR}/../lib/Qt/lib/libQt5Core.so.5")
    endif ()
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH ON)

    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/src/linuxcsd.cpp"
    )
    if (EXISTS "${QTCREATOR_BIN_DIR}/../lib/libQt5Core.so.5")
    target_link_libraries(${PROJECT_NAME} PRIVATE "${QTCREATOR_BIN_DIR}/../lib/libQt5X11Extras.so.5")
    else ()
        target_link_libraries(${PROJECT_NAME} PRIVATE "${QTCREATOR_BIN_DIR}/../lib/Qt/lib/libQt5X11Extras.so.5")
    endif ()
else ()
    find_library(CORE_LIB NAMES Core4 PATHS "${CMAKE_SOURCE_DIR}/lib/qtcreator/plugins")
    find_library(EXTENSIONSYSTEM_LIB NAMES ExtensionSystem4 PATHS "${CMAKE_SOURCE_DIR}/lib/qtcreator")
    find_library(PROJECTEXPLORER_LIB NAMES ProjectExplorer4 PATHS "${CMAKE_SOURCE_DIR}/lib/qtcreator/plugins")
    find_library(UTILS_LIB NAMES Utils4 PATHS "${CMAKE_SOURCE_DIR}/lib/qtcreator")
    set(QTNETWORK_LIB "${Qt5Network_LIBRARIES}")
    set(QTWIDGETS_LIB "${Qt5Widgets_LIBRARIES}")
    set(QTGUI_LIB "${Qt5Gui_LIBRARIES}")
    set(QTCORE_LIB "${Qt5Core_LIBRARIES}")

    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/src/qtwinbackports.cpp"
        "${CMAKE_SOURCE_DIR}/src/win32csd.cpp"
    )

    target_compile_definitions(${PROJECT_NAME} PUBLIC WIN32_LEAN_AND_MEAN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE)

    find_library(DWMAPI "dwmapi")
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${DWMAPI}
    )
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
    "${CORE_LIB}"
    "${EXTENSIONSYSTEM_LIB}"
    "${PROJECTEXPLORER_LIB}"
    "${UTILS_LIB}"
    "${QTNETWORK_LIB}"
    "${QTWIDGETS_LIB}"
    "${QTGUI_LIB}"
    "${QTCORE_LIB}"
)

if (APPLE)
    install(TARGETS ${PROJECT_NAME} DESTINATION "${QTCREATOR_BIN_DIR}/../PlugIns")
    install(CODE "execute_process(COMMAND \"sudo xattr -rd com.apple.quarantine ${QTCREATOR_BIN_DIR}/../../\")")
elseif (UNIX)
    install(TARGETS ${PROJECT_NAME} DESTINATION "${QTCREATOR_BIN_DIR}/../lib/qtcreator/plugins")
else ()
    file(TO_CMAKE_PATH $ENV{LOCALAPPDATA} LOCALAPPDATA_PATH)
    if ("${LOCALAPPDATA_PATH}" STREQUAL "")
        message(FATAL_ERROR "%LOCALAPPDATA% not set - plugin cannot be installed")
    endif ()
    set(PLUGINDIR_PATH "${LOCALAPPDATA_PATH}/QtProject/QtCreator/plugins/${QTCREATOR_VERSION}")
    if (NOT IS_DIRECTORY "${PLUGINDIR_PATH}")
        file(MAKE_DIRECTORY "${PLUGINDIR_PATH}")
    endif ()
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${PLUGINDIR_PATH}")
endif ()
