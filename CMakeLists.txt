cmake_minimum_required(VERSION 3.20)

project(aqtk-shim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AQTK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

include(cmake/voices.cmake)
include(cmake/sdk.cmake)
include(cmake/stage.cmake)

if(WIN32)
    include(cmake/platform/windows.cmake)
elseif(APPLE)
    include(cmake/platform/macos.cmake)
else()
    message(FATAL_ERROR "aqtk-shim builds on Windows (MSVC) and macOS.")
endif()

add_library(Aqsh SHARED
    src/Aqsh.cpp
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
