# One directory per architecture holds Aqsh plus the renamed vendor libraries
# and phonts. The loader opens them from the directory that contains the shim.
set(_aqtk_arch_choices win64 win32 linux64 linux32 macos)
if(NOT AQTK_ARCH IN_LIST _aqtk_arch_choices)
    message(FATAL_ERROR "AQTK_ARCH must be one of: ${_aqtk_arch_choices}")
endif()

message(STATUS "aqtk-shim stages Release into ${CMAKE_INSTALL_PREFIX}/${AQTK_ARCH}")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Aqsh.h" DESTINATION ".")

install(TARGETS Aqsh
    RUNTIME DESTINATION "${AQTK_ARCH}"
    LIBRARY DESTINATION "${AQTK_ARCH}"
    ARCHIVE DESTINATION "${AQTK_ARCH}"
)

if(WIN32)
    foreach(_voice IN LISTS AQTK1_VOICES)
        install(FILES "${AQTK_ROOT}/tk1/aqtk1_win/${AQTK_LIBDIR}/${_voice}/AquesTalk.dll"
            DESTINATION "${AQTK_ARCH}"
            RENAME "AquesTalk1_${_voice}.dll")
    endforeach()
    install(FILES "${AQTK2_VENDOR_DLL}"
        DESTINATION "${AQTK_ARCH}"
        RENAME "AquesTalk2.dll")
    install(FILES "${AQTK10_VENDOR_DLL}"
        DESTINATION "${AQTK_ARCH}"
        RENAME "AquesTalk10.dll")
else()
    foreach(_src _name IN ZIP_LISTS AQTK1_SOURCES AQTK1_STAGED)
        install(FILES "${_src}"
            DESTINATION "${AQTK_ARCH}"
            RENAME "${_name}")
    endforeach()
    install(FILES "${AQTK2_LIB}"
        DESTINATION "${AQTK_ARCH}"
        RENAME "${AQTK2_STAGED}")
    install(FILES "${AQTK10_LIB}"
        DESTINATION "${AQTK_ARCH}"
        RENAME "${AQTK10_STAGED}")
endif()

if(AQTK_STAGE_PHONTS)
    foreach(_phont IN LISTS AQTK2_PHONTS_PRESENT)
        install(FILES "${AQTK2_PHONTDIR}/${_phont}.phont"
            DESTINATION "${AQTK_ARCH}/phont")
    endforeach()
endif()
