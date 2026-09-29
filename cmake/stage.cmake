function(aqtk_stage_file target src dest_name)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${src}" "$<TARGET_FILE_DIR:${target}>/${dest_name}"
        VERBATIM
    )
endfunction()

function(aqtk_stage_phonts target)
    if(NOT AQTK_STAGE_PHONTS)
        return()
    endif()
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${target}>/phont"
        VERBATIM
    )
    foreach(_phont IN LISTS AQTK2_PHONTS_PRESENT)
        aqtk_stage_file("${target}"
            "${AQTK2_PHONTDIR}/${_phont}.phont"
            "phont/${_phont}.phont")
    endforeach()
endfunction()
