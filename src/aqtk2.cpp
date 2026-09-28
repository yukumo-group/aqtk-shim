#include "aqtk2.h"

#include "Aqsh.h"

extern "C" {

__declspec(dllimport) unsigned char* __stdcall AquesTalk2_Synthe_Utf8(
    const char* koe, int speed, int* size, void* phont);
__declspec(dllimport) void __stdcall AquesTalk2_FreeWave(unsigned char* wav);

}

unsigned char* aqtk2_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    return AquesTalk2_Synthe_Utf8(text, param->speed, size, nullptr);
}

void aqtk2_free(unsigned char* wav) {
    AquesTalk2_FreeWave(wav);
}
