if(NOT ANDROID_ABI)
    message(FATAL_ERROR "ANDROID_ABI is not set. Configure with cmake/android.toolchain.cmake.")
endif()

set(_aqtk_abis armeabi-v7a arm64-v8a x86 x86_64 riscv64)
if(NOT ANDROID_ABI IN_LIST _aqtk_abis)
    message(FATAL_ERROR "AquesTalk Android libraries do not include ABI '${ANDROID_ABI}'.")
endif()

# The prebuilt libraries are aligned for 16 KB pages.
add_link_options(-Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384)

set(AQTK_RPATH "\$ORIGIN")
set(_jni "jniLibs/${ANDROID_ABI}")
set(AQTK2_LIB "${AQTK_ROOT}/tk2/aqtk2-adr/${_jni}/libAquesTalk2.so")
set(AQTK2_STAGED "libAquesTalk2.so")
# JNI entry only; opened by path from AQTK2_DLOPEN. AquesTalk10 is linked.
set(AQTK_DLOPEN_AQTK2 "${AQTK2_STAGED}")
set(AQTK10_LIB "${AQTK_ROOT}/tk10/aqtk10_adr/${_jni}/libAquesTalk.so")
set(AQTK10_STAGED "libAquesTalk.so")
set(AQTK2_PHONTDIR "${AQTK_ROOT}/tk2/aqtk2-adr/phont")

# This SDK ships voice F1 only. Other Aq1Preset values fail to open.
set(AQTK1_VOICES f1)
set(AQTK1_SOURCES "")
set(AQTK1_STAGED "")
foreach(_voice IN LISTS AQTK1_VOICES)
    list(APPEND AQTK1_SOURCES "${AQTK_ROOT}/tk1/aqtk1-adr/${_jni}/libAquesTalk.so")
    list(APPEND AQTK1_STAGED "libAquesTalk1-${_voice}.so")
endforeach()

aqtk_require_files(${AQTK1_SOURCES} "${AQTK2_LIB}" "${AQTK10_LIB}")
aqtk_collect_phonts()

set(AQTK_EXTRA_SOURCES src/jni_bridge.cpp)

include("${CMAKE_CURRENT_LIST_DIR}/unix.cmake")
