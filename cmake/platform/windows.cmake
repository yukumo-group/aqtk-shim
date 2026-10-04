if(NOT MSVC)
    message(FATAL_ERROR "Aqsh embeds the MSVC AquesTalk import libraries and must be built with MSVC.")
endif()

# Release is size-optimized and emits no debug symbols.
set(_aqtk_release_compile "/O1 /Ob1 /GF /Gy /Gw /DNDEBUG")
set(_aqtk_release_link "/INCREMENTAL:NO /DEBUG:NONE /OPT:REF /OPT:ICF")
set(CMAKE_CXX_FLAGS_RELEASE "${_aqtk_release_compile}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "${_aqtk_release_compile}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${_aqtk_release_link}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${_aqtk_release_link}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${_aqtk_release_link}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${_aqtk_release_link}" CACHE STRING "" FORCE)

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

set(_aqtk1_dlls "")
foreach(_voice IN LISTS AQTK1_VOICES)
    list(APPEND _aqtk1_dlls "${AQTK_ROOT}/tk1/aqtk1_win/${AQTK_LIBDIR}/${_voice}/AquesTalk.dll")
endforeach()
aqtk_require_files(
    ${_aqtk1_dlls}
    "${AQTK2_VENDOR_LIB}"
    "${AQTK2_VENDOR_DLL}"
    "${AQTK10_VENDOR_DLL}")
aqtk_collect_phonts()

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

function(aqtk_configure_aqsh target)
    # Aqsh.h comments are UTF-8. MSVC otherwise uses the ANSI code page and warns C4819.
    target_compile_options(${target} PRIVATE /utf-8)
    add_dependencies(${target} aqtk_implibs)
    target_link_libraries(${target} PRIVATE
        "${AQTK2_VENDOR_LIB}"
        "${AQTK10_IMPLIB}"
    )
    # Load those renamed DLLs from Aqsh.dll's directory (0x100) and still allow
    # system DLLs (0x800).
    target_link_options(${target} PRIVATE "/DEPENDENTLOADFLAG:0x900")

    foreach(_voice IN LISTS AQTK1_VOICES)
        aqtk_stage_file("${target}"
            "${AQTK_ROOT}/tk1/aqtk1_win/${AQTK_LIBDIR}/${_voice}/AquesTalk.dll"
            "AquesTalk1_${_voice}.dll")
    endforeach()
    aqtk_stage_file("${target}" "${AQTK2_VENDOR_DLL}" "AquesTalk2.dll")
    aqtk_stage_file("${target}" "${AQTK10_VENDOR_DLL}" "AquesTalk10.dll")
    aqtk_stage_phonts("${target}")
endfunction()

function(aqtk_configure_demo target)
    target_compile_options(${target} PRIVATE /utf-8)
endfunction()
