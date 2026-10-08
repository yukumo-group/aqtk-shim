cmake_minimum_required(VERSION 3.20)

project(aqtk-shim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AQTK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

# OFF skips copying phont/ next to Aqsh. The parent project passes paths itself.
option(AQTK_STAGE_PHONTS "Copy the AquesTalk2 phont directory next to Aqsh" ON)

include(cmake/voices.cmake)
include(cmake/sdk.cmake)
include(cmake/stage.cmake)

if(WIN32)
    include(cmake/platform/windows.cmake)
elseif(APPLE)
    include(cmake/platform/macos.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(cmake/platform/linux.cmake)
else()
    message(FATAL_ERROR "aqtk-shim builds on Windows (MSVC), macOS, and Linux.")
endif()

add_library(Aqsh SHARED
    src/Aqsh.cpp
    src/aq_error.cpp
    src/env_keys.cpp
    src/aqtk1.cpp
    src/aqtk2.cpp
    src/aqtk10.cpp
    src/wav_track.cpp
    src/module_dir.cpp
)
target_compile_definitions(Aqsh PRIVATE AQSWRAPPER_EXPORTS)
target_include_directories(Aqsh PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
aqtk_configure_aqsh(Aqsh)

add_executable(aqtk_demo demo/main.cpp)
target_link_libraries(aqtk_demo PRIVATE Aqsh)
aqtk_configure_demo(aqtk_demo)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../third-party" CACHE PATH "Staged AquesTalk runtime" FORCE)
endif()

# win64, win32, linux64, linux32, macos. Presets and `just init` set this.
if(NOT AQTK_ARCH)
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(AQTK_ARCH "win64")
        else()
            set(AQTK_ARCH "win32")
        endif()
    elseif(APPLE)
        set(AQTK_ARCH "macos")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(AQTK_ARCH "linux32")
    else()
        set(AQTK_ARCH "linux64")
    endif()
endif()

include(cmake/install.cmake)
