#include "aqtk1.h"

#include "Aqsh.h"

namespace {
    void apply_key(int (__stdcall* set_key)(const char*), const char* key) {
        if (key && key[0]) {
            set_key(key);
        }
    }
}

extern "C" {

__declspec(dllimport) unsigned char* __stdcall AquesTalk1_Synthe_Utf8(
    const char* koe, int speed, int* size);
__declspec(dllimport) void __stdcall AquesTalk1_FreeWave(unsigned char* wav);
__declspec(dllimport) int __stdcall AquesTalk1_SetDevKey(const char* key);
__declspec(dllimport) int __stdcall AquesTalk1_SetUsrKey(const char* key);

}

unsigned char* aqtk1_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    apply_key(AquesTalk1_SetDevKey, param->DevKey);
    apply_key(AquesTalk1_SetUsrKey, param->UserKey);
    return AquesTalk1_Synthe_Utf8(text, param->speed, size);
}

void aqtk1_free(unsigned char* wav) {
    AquesTalk1_FreeWave(wav);
}
