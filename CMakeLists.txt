cmake_minimum_required(VERSION 3.20)

project(aqtk-shim LANGUAGES CXX)

if(NOT MSVC)
    message(FATAL_ERROR "Aqsh embeds the MSVC AquesTalk import libraries and must be built with MSVC.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(_msvc_bin "${CMAKE_LINKER}" DIRECTORY)
find_program(AQTK_LIBTOOL NAMES lib.exe HINTS "${_msvc_bin}" REQUIRED)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(AQTK_MACHINE X64)
    set(AQTK_LIBDIR lib64)
else()
    set(AQTK_MACHINE X86)
    set(AQTK_LIBDIR lib)
endif()

set(AQTK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
# Same order as Aq1Preset.
set(AQTK1_VOICES f1 f2 f3 m1 m2 r1 dvd imd1 jgr)
set(AQTK2_VENDOR_LIB "${AQTK_ROOT}/tk2/aqtk2-win/${AQTK_LIBDIR}/AquesTalk2.lib")
set(AQTK2_VENDOR_DLL "${AQTK_ROOT}/tk2/aqtk2-win/${AQTK_LIBDIR}/AquesTalk2.dll")
set(AQTK10_VENDOR_DLL "${AQTK_ROOT}/tk10/aqtk10_win/${AQTK_LIBDIR}/AquesTalk.dll")

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

add_library(Aqsh SHARED
    src/Aqsh.cpp
    src/aqtk1.cpp
    src/aqtk2.cpp
    src/aqtk10.cpp
    src/wav_track.cpp
)
add_dependencies(Aqsh aqtk_implibs)
target_compile_definitions(Aqsh PRIVATE AQSWRAPPER_EXPORTS)
target_include_directories(Aqsh PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(Aqsh PRIVATE
    "${AQTK2_VENDOR_LIB}"
    "${AQTK10_IMPLIB}"
)

# Load those renamed DLLs from Aqsh.dll's directory (0x100) and still allow
# system DLLs (0x800).
target_link_options(Aqsh PRIVATE "/DEPENDENTLOADFLAG:0x900")

function(aqtk_stage_dll src dest_name)
    add_custom_command(TARGET Aqsh POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${src}" "$<TARGET_FILE_DIR:Aqsh>/${dest_name}"
        VERBATIM
    )
endfunction()

foreach(_voice IN LISTS AQTK1_VOICES)
    aqtk_stage_dll(
        "${AQTK_ROOT}/tk1/aqtk1_win/${AQTK_LIBDIR}/${_voice}/AquesTalk.dll"
        "AquesTalk1_${_voice}.dll")
endforeach()
aqtk_stage_dll("${AQTK2_VENDOR_DLL}" "AquesTalk2.dll")
aqtk_stage_dll("${AQTK10_VENDOR_DLL}" "AquesTalk10.dll")

add_executable(aqtk_demo demo/main.cpp)
target_link_libraries(aqtk_demo PRIVATE Aqsh)
target_compile_options(aqtk_demo PRIVATE /utf-8)
