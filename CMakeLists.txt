cmake_minimum_required(VERSION 3.20)

project(aqtk-shim LANGUAGES CXX)

if(WIN32 AND NOT MSVC)
    message(FATAL_ERROR "Aqsh embeds the MSVC AquesTalk import libraries and must be built with MSVC.")
endif()
if(NOT WIN32 AND NOT APPLE)
    message(FATAL_ERROR "aqtk-shim builds on Windows (MSVC) and macOS.")
endif()

# AquesTalk1's macOS dylibs are arm64-only. AquesTalk2 and AquesTalk10 are universal.
if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "AquesTalk1 macOS libraries are arm64-only" FORCE)
    endif()
    if(NOT CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        message(FATAL_ERROR "AquesTalk1 macOS libraries are arm64-only (CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}).")
    endif()
endif()

if(MSVC)
    # Release is size-optimized and emits no debug symbols.
    set(_aqtk_release_compile "/O1 /Ob1 /GF /Gy /Gw /DNDEBUG")
    set(_aqtk_release_link "/INCREMENTAL:NO /DEBUG:NONE /OPT:REF /OPT:ICF")
    set(CMAKE_CXX_FLAGS_RELEASE "${_aqtk_release_compile}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${_aqtk_release_compile}" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${_aqtk_release_link}" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${_aqtk_release_link}" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${_aqtk_release_link}" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${_aqtk_release_link}" CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AQTK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
# Same order as Aq1Preset.
set(AQTK1_VOICES f1 f2 f3 m1 m2 r1 dvd imd1 jgr)
# Same order as Aq2Preset.
set(AQTK2_PHONTS
    aq_defo1 aq_f1c aq_f3a aq_huskey aq_m4b aq_mf1
    aq_rb2 aq_rb3 aq_rm aq_robo aq_teto1 aq_yukkuri
    ar_f4 ar_m5 ar_mf2 ar_rm3)

if(WIN32)
    get_filename_component(_msvc_bin "${CMAKE_LINKER}" DIRECTORY)
    find_program(AQTK_LIBTOOL NAMES lib.exe HINTS "${_msvc_bin}" REQUIRED)

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(AQTK_MACHINE X64)
        set(AQTK_LIBDIR lib64)
    else()
        set(AQTK_MACHINE X86)
        set(AQTK_LIBDIR lib)
    endif()

    set(AQTK2_VENDOR_LIB "${AQTK_ROOT}/tk2/aqtk2-win/${AQTK_LIBDIR}/AquesTalk2.lib")
    set(AQTK2_VENDOR_DLL "${AQTK_ROOT}/tk2/aqtk2-win/${AQTK_LIBDIR}/AquesTalk2.dll")
    set(AQTK10_VENDOR_DLL "${AQTK_ROOT}/tk10/aqtk10_win/${AQTK_LIBDIR}/AquesTalk.dll")
    set(AQTK2_PHONTDIR "${AQTK_ROOT}/tk2/aqtk2-win/phont")

    foreach(_voice IN LISTS AQTK1_VOICES)
        set(_aqtk1_dll "${AQTK_ROOT}/tk1/aqtk1_win/${AQTK_LIBDIR}/${_voice}/AquesTalk.dll")
        if(NOT EXISTS "${_aqtk1_dll}")
            message(FATAL_ERROR "Missing AquesTalk SDK file: ${_aqtk1_dll}")
        endif()
    endforeach()
    foreach(_required IN ITEMS "${AQTK2_VENDOR_LIB}" "${AQTK2_VENDOR_DLL}" "${AQTK10_VENDOR_DLL}")
        if(NOT EXISTS "${_required}")
            message(FATAL_ERROR "Missing AquesTalk SDK file: ${_required}")
        endif()
    endforeach()

    # stdcall argument bytes
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(_aq_free "@4")
        set(_aq_key "@4")
        set(_aq_synthe "@12")
    else()
        set(_aq_free "")
        set(_aq_key "")
        set(_aq_synthe "")
    endif()

    function(aqtk_write_ordinal_def def_path library_name symbol_prefix)
        file(WRITE "${def_path}"
"LIBRARY ${library_name}
EXPORTS
${symbol_prefix}FreeWave${_aq_free} @1 NONAME
${symbol_prefix}SetDevKey${_aq_key} @2 NONAME
${symbol_prefix}SetUsrKey${_aq_key} @3 NONAME
${symbol_prefix}Synthe${_aq_synthe} @4 NONAME
${symbol_prefix}Synthe_Utf16${_aq_synthe} @5 NONAME
${symbol_prefix}Synthe_Utf8${_aq_synthe} @6 NONAME
")
    endfunction()

    function(aqtk_renamed_implib symbol_prefix library_name out_lib)
        set(def_path "${CMAKE_CURRENT_BINARY_DIR}/${library_name}.def")
        aqtk_write_ordinal_def("${def_path}" "${library_name}" "${symbol_prefix}")
        add_custom_command(
            OUTPUT "${out_lib}"
            COMMAND "${AQTK_LIBTOOL}" /nologo "/MACHINE:${AQTK_MACHINE}" "/DEF:${def_path}" "/OUT:${out_lib}"
            DEPENDS "${def_path}"
            COMMENT "Import library ${library_name}.lib (${symbol_prefix}*)"
            VERBATIM
        )
        set_source_files_properties("${out_lib}" PROPERTIES GENERATED TRUE)
    endfunction()

    set(AQTK10_IMPLIB "${CMAKE_CURRENT_BINARY_DIR}/AquesTalk10.lib")
    aqtk_renamed_implib("AquesTalk10_" "AquesTalk10" "${AQTK10_IMPLIB}")
    add_custom_target(aqtk_implibs DEPENDS "${AQTK10_IMPLIB}")
else()
    set(AQTK1_LIBDIR "${AQTK_ROOT}/tk1/aqtk1_mac/lib")
    set(AQTK2_DYLIB "${AQTK_ROOT}/tk2/aqtk2_mac/lib/libAquesTalk2Eva.dylib")
    set(AQTK10_DYLIB "${AQTK_ROOT}/tk10/aqtk10_mac/lib/libAquesTalk10.dylib")
    set(AQTK2_PHONTDIR "${AQTK_ROOT}/tk2/aqtk2_mac/phont")

    foreach(_voice IN LISTS AQTK1_VOICES)
        set(_aqtk1_dylib "${AQTK1_LIBDIR}/libAquesTalk1-${_voice}.dylib")
        if(NOT EXISTS "${_aqtk1_dylib}")
            message(FATAL_ERROR "Missing AquesTalk SDK file: ${_aqtk1_dylib}")
        endif()
    endforeach()
    foreach(_required IN ITEMS "${AQTK2_DYLIB}" "${AQTK10_DYLIB}")
        if(NOT EXISTS "${_required}")
            message(FATAL_ERROR "Missing AquesTalk SDK file: ${_required}")
        endif()
    endforeach()
endif()

set(AQTK2_PHONTS_PRESENT "")
foreach(_phont IN LISTS AQTK2_PHONTS)
    set(_aqtk2_phont "${AQTK2_PHONTDIR}/${_phont}.phont")
    if(EXISTS "${_aqtk2_phont}")
        list(APPEND AQTK2_PHONTS_PRESENT "${_phont}")
    elseif(WIN32)
        message(FATAL_ERROR "Missing AquesTalk SDK file: ${_aqtk2_phont}")
    else()
        message(STATUS "AquesTalk2 phont not in the macOS SDK, skipping: ${_phont}")
    endif()
endforeach()

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

if(WIN32)
    add_dependencies(Aqsh aqtk_implibs)
    target_link_libraries(Aqsh PRIVATE
        "${AQTK2_VENDOR_LIB}"
        "${AQTK10_IMPLIB}"
    )
    # Load those renamed DLLs from Aqsh.dll's directory (0x100) and still allow
    # system DLLs (0x800).
    target_link_options(Aqsh PRIVATE "/DEPENDENTLOADFLAG:0x900")
else()
    # AquesTalk2 (AquesTalk2_*) and AquesTalk10 (AquesTalk_*) are linked.
    # Each AquesTalk1 voice also exports AquesTalk_*, so those are dlopen'd.
    target_link_libraries(Aqsh PRIVATE
        "${AQTK2_DYLIB}"
        "${AQTK10_DYLIB}"
    )
    set_target_properties(Aqsh PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
        MACOSX_RPATH ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

function(aqtk_stage_dll src dest_name)
    add_custom_command(TARGET Aqsh POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${src}" "$<TARGET_FILE_DIR:Aqsh>/${dest_name}"
        VERBATIM
    )
endfunction()

if(WIN32)
    foreach(_voice IN LISTS AQTK1_VOICES)
        aqtk_stage_dll(
            "${AQTK_ROOT}/tk1/aqtk1_win/${AQTK_LIBDIR}/${_voice}/AquesTalk.dll"
            "AquesTalk1_${_voice}.dll")
    endforeach()
    aqtk_stage_dll("${AQTK2_VENDOR_DLL}" "AquesTalk2.dll")
    aqtk_stage_dll("${AQTK10_VENDOR_DLL}" "AquesTalk10.dll")
else()
    foreach(_voice IN LISTS AQTK1_VOICES)
        aqtk_stage_dll(
            "${AQTK1_LIBDIR}/libAquesTalk1-${_voice}.dylib"
            "libAquesTalk1-${_voice}.dylib")
    endforeach()
    aqtk_stage_dll("${AQTK2_DYLIB}" "libAquesTalk2Eva.dylib")
    aqtk_stage_dll("${AQTK10_DYLIB}" "libAquesTalk10.dylib")
endif()

add_custom_command(TARGET Aqsh POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:Aqsh>/phont"
    VERBATIM
)
foreach(_phont IN LISTS AQTK2_PHONTS_PRESENT)
    aqtk_stage_dll(
        "${AQTK2_PHONTDIR}/${_phont}.phont"
        "phont/${_phont}.phont")
endforeach()

add_executable(aqtk_demo demo/main.cpp)
target_link_libraries(aqtk_demo PRIVATE Aqsh)
if(MSVC)
    target_compile_options(aqtk_demo PRIVATE /utf-8)
endif()
if(APPLE)
    set_target_properties(aqtk_demo PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
endif()
