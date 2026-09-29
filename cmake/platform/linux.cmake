# AquesTalk Linux libraries are x86_64 (lib64) and i386. AquesTalk2's 32-bit
# build lives in lib/, not lib32.
if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64|i[3-6]86|x86)$")
    message(FATAL_ERROR "AquesTalk Linux libraries are x86_64 and i386 only (CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}).")
endif()

# The dynamic loader opens these by SONAME, not by the SDK filename.
# libAquesTalk2Eva.so.2.3 -> libAquesTalk2.so.2
# libAquesTalk10.so.1.1   -> libAquesTalk10.so.1
set(AQTK_RPATH "\$ORIGIN")
set(AQTK2_STAGED "libAquesTalk2.so.2")
set(AQTK10_STAGED "libAquesTalk10.so.1")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_aqtk1_libdir "lib64")
    set(_aqtk2_libdir "lib64")
    set(_aqtk10_libdir "lib64")
else()
    set(_aqtk1_libdir "lib32")
    set(_aqtk2_libdir "lib")
    set(_aqtk10_libdir "lib32")
endif()

set(AQTK2_LIB "${AQTK_ROOT}/tk2/aqtk2-lnx/${_aqtk2_libdir}/libAquesTalk2Eva.so.2.3")
set(AQTK10_LIB "${AQTK_ROOT}/tk10/aqtk10_lnx/${_aqtk10_libdir}/libAquesTalk10.so.1.1")
set(AQTK2_PHONTDIR "${AQTK_ROOT}/tk2/aqtk2-lnx/phont")

set(AQTK1_SOURCES "")
set(AQTK1_STAGED "")
foreach(_voice IN LISTS AQTK1_VOICES)
    list(APPEND AQTK1_SOURCES "${AQTK_ROOT}/tk1/aqtk1_lnx/${_aqtk1_libdir}/${_voice}/libAquesTalk.so")
    list(APPEND AQTK1_STAGED "libAquesTalk1-${_voice}.so")
endforeach()
aqtk_require_files(${AQTK1_SOURCES} "${AQTK2_LIB}" "${AQTK10_LIB}")
aqtk_collect_phonts()

include("${CMAKE_CURRENT_LIST_DIR}/unix.cmake")
