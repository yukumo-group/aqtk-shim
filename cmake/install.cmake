# Each version directory is a runnable copy of Aqsh: the loader opens vendor
# libraries and phonts from the directory that contains the shim.
set(_aqtk_arch_choices win64 win32 linux64 linux32 macos)
if(NOT AQTK_ARCH IN_LIST _aqtk_arch_choices)
    message(FATAL_ERROR "AQTK_ARCH must be one of: ${_aqtk_arch_choices}")
endif()

message(STATUS "aqtk-shim stages Release into ${CMAKE_INSTALL_PREFIX}/aq{1,2,10}/${AQTK_ARCH}")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Aqsh.h" DESTINATION ".")

foreach(_ver IN ITEMS aq1 aq2 aq10)
    set(_dest "${_ver}/${AQTK_ARCH}")
    install(TARGETS Aqsh
        RUNTIME DESTINATION "${_dest}"
        LIBRARY DESTINATION "${_dest}"
        ARCHIVE DESTINATION "${_dest}"
    )
endforeach()

if(WIN32)
    foreach(_voice IN LISTS AQTK1_VOICES)
        install(FILES "${AQTK_ROOT}/tk1/aqtk1_win/${AQTK_LIBDIR}/${_voice}/AquesTalk.dll"
            DESTINATION "aq1/${AQTK_ARCH}"
            RENAME "AquesTalk1_${_voice}.dll")
    endforeach()
    install(FILES "${AQTK2_VENDOR_DLL}"
        DESTINATION "aq2/${AQTK_ARCH}"
        RENAME "AquesTalk2.dll")
    install(FILES "${AQTK10_VENDOR_DLL}"
        DESTINATION "aq10/${AQTK_ARCH}"
        RENAME "AquesTalk10.dll")
else()
    foreach(_src _name IN ZIP_LISTS AQTK1_SOURCES AQTK1_STAGED)
        install(FILES "${_src}"
            DESTINATION "aq1/${AQTK_ARCH}"
            RENAME "${_name}")
    endforeach()
    install(FILES "${AQTK2_LIB}"
        DESTINATION "aq2/${AQTK_ARCH}"
        RENAME "${AQTK2_STAGED}")
    install(FILES "${AQTK10_LIB}"
        DESTINATION "aq10/${AQTK_ARCH}"
        RENAME "${AQTK10_STAGED}")
endif()

if(AQTK_STAGE_PHONTS)
    foreach(_phont IN LISTS AQTK2_PHONTS_PRESENT)
        install(FILES "${AQTK2_PHONTDIR}/${_phont}.phont"
            DESTINATION "aq2/${AQTK_ARCH}/phont")
    endforeach()
endif()
