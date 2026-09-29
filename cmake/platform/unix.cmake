find_package(Threads REQUIRED)

function(aqtk_configure_aqsh target)
    target_link_libraries(${target} PRIVATE
        "${AQTK2_LIB}"
        "${AQTK10_LIB}"
        Threads::Threads
        ${CMAKE_DL_LIBS}
    )
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "${AQTK_RPATH}"
        INSTALL_RPATH "${AQTK_RPATH}"
        # Staged libraries sit next to this one. Skip the SDK directories CMake
        # would otherwise record from the absolute link paths.
        BUILD_WITH_INSTALL_RPATH ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
    if(APPLE)
        set_target_properties(${target} PROPERTIES MACOSX_RPATH ON)
    endif()

    foreach(_src _name IN ZIP_LISTS AQTK1_SOURCES AQTK1_STAGED)
        aqtk_stage_file("${target}" "${_src}" "${_name}")
    endforeach()
    aqtk_stage_file("${target}" "${AQTK2_LIB}" "${AQTK2_STAGED}")
    aqtk_stage_file("${target}" "${AQTK10_LIB}" "${AQTK10_STAGED}")
    aqtk_stage_phonts("${target}")
endfunction()

function(aqtk_configure_demo target)
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH "${AQTK_RPATH}"
        INSTALL_RPATH "${AQTK_RPATH}"
        BUILD_WITH_INSTALL_RPATH ON
    )
endfunction()
