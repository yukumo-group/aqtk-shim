# AquesTalk1's macOS dylibs are arm64-only. AquesTalk2 and AquesTalk10 are universal.
if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "AquesTalk1 macOS libraries are arm64-only" FORCE)
endif()
if(NOT CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    message(FATAL_ERROR "AquesTalk1 macOS libraries are arm64-only (CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}).")
endif()

set(AQTK1_LIBDIR "${AQTK_ROOT}/tk1/aqtk1_mac/lib")
set(AQTK2_DYLIB "${AQTK_ROOT}/tk2/aqtk2_mac/lib/libAquesTalk2Eva.dylib")
set(AQTK10_DYLIB "${AQTK_ROOT}/tk10/aqtk10_mac/lib/libAquesTalk10.dylib")
set(AQTK2_PHONTDIR "${AQTK_ROOT}/tk2/aqtk2_mac/phont")

set(_aqtk1_dylibs "")
foreach(_voice IN LISTS AQTK1_VOICES)
    list(APPEND _aqtk1_dylibs "${AQTK1_LIBDIR}/libAquesTalk1-${_voice}.dylib")
endforeach()
aqtk_require_files(${_aqtk1_dylibs} "${AQTK2_DYLIB}" "${AQTK10_DYLIB}")
aqtk_collect_phonts()

function(aqtk_configure_aqsh target)
    # AquesTalk2 (AquesTalk2_*) and AquesTalk10 (AquesTalk_*) are linked.
    # Each AquesTalk1 voice also exports AquesTalk_*, so those are dlopen'd.
    target_link_libraries(${target} PRIVATE
        "${AQTK2_DYLIB}"
        "${AQTK10_DYLIB}"
    )
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
        MACOSX_RPATH ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    foreach(_voice IN LISTS AQTK1_VOICES)
        aqtk_stage_file("${target}"
            "${AQTK1_LIBDIR}/libAquesTalk1-${_voice}.dylib"
            "libAquesTalk1-${_voice}.dylib")
    endforeach()
    aqtk_stage_file("${target}" "${AQTK2_DYLIB}" "libAquesTalk2Eva.dylib")
    aqtk_stage_file("${target}" "${AQTK10_DYLIB}" "libAquesTalk10.dylib")
    aqtk_stage_phonts("${target}")
endfunction()

function(aqtk_configure_demo target)
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
endfunction()
