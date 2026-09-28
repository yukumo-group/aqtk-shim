#include "aqtk10.h"

#include "Aqsh.h"

namespace {
    struct Aq10Voice {
        int bas;
        int spd;
        int vol;
        int pit;
        int acc;
        int lmd;
        int fsc;
    };

    void apply_key(int (__stdcall* set_key)(const char*), const char* key) {
        if (key && key[0]) {
            set_key(key);
        }
    }
}

extern "C" {

__declspec(dllimport) unsigned char* __stdcall AquesTalk10_Synthe_Utf8(
    const Aq10Voice* voice, const char* koe, int* size);
__declspec(dllimport) void __stdcall AquesTalk10_FreeWave(unsigned char* wav);
__declspec(dllimport) int __stdcall AquesTalk10_SetDevKey(const char* key);
__declspec(dllimport) int __stdcall AquesTalk10_SetUsrKey(const char* key);

}

unsigned char* aqtk10_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    apply_key(AquesTalk10_SetDevKey, param->DevKey);
    apply_key(AquesTalk10_SetUsrKey, param->UserKey);

    Aq10Voice voice{};
    voice.bas = param->base;
    voice.spd = param->speed;
    voice.vol = param->volume;
    voice.pit = param->pitch;
    voice.acc = param->accent;
    voice.lmd = param->lmd;
    voice.fsc = param->fsc;
    return AquesTalk10_Synthe_Utf8(&voice, text, size);
}

void aqtk10_free(unsigned char* wav) {
    AquesTalk10_FreeWave(wav);
}
