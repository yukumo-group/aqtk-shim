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

    constexpr Aq10Voice kPresets[] = {
        {0, 100, 100, 100, 100, 100, 100}, // F1
        {1, 100, 100,  77, 150, 100, 100}, // F2
        {0,  80, 100, 100, 100,  61, 148}, // F3
        {2, 100, 100,  30, 100, 100, 100}, // M1
        {2, 105, 100,  45, 130, 120, 100}, // M2
        {2, 100, 100,  30,  20, 190, 100}, // R1
        {1,  70, 100,  50,  50,  50, 180}, // R2
    };
    static_assert(sizeof(kPresets) / sizeof(kPresets[0]) == AQ10_PRESET_COUNT - 1);

    bool voice_from_param(const AqSynthParam* param, Aq10Voice* voice) {
        if (param->preset == AQ10_CUSTOM) {
            voice->bas = param->base;
            voice->spd = param->speed;
            voice->vol = param->volume;
            voice->pit = param->pitch;
            voice->acc = param->accent;
            voice->lmd = param->lmd;
            voice->fsc = param->fsc;
            return true;
        }
        if (param->preset <= AQ10_CUSTOM || param->preset >= AQ10_PRESET_COUNT) {
            return false;
        }
        *voice = kPresets[param->preset - AQ10_F1];
        return true;
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
    Aq10Voice voice{};
    if (!voice_from_param(param, &voice)) {
        if (size) {
            *size = 0;
        }
        return nullptr;
    }

    apply_key(AquesTalk10_SetDevKey, param->DevKey);
    apply_key(AquesTalk10_SetUsrKey, param->UserKey);
    return AquesTalk10_Synthe_Utf8(&voice, text, size);
}

void aqtk10_free(unsigned char* wav) {
    AquesTalk10_FreeWave(wav);
}
