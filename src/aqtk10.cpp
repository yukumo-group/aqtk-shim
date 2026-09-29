#include "aqtk10.h"

#include "Aqsh.h"
#include "synth_util.h"
#include "wav_track.h"

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

// Windows imports AquesTalk10_* by ordinal. macOS and Linux link AquesTalk10,
// which exports the same AquesTalk_* names as the AquesTalk1 voices.
extern "C" {

#ifdef _WIN32
AQTK_IMPORT unsigned char* AQTK_CALL AquesTalk10_Synthe_Utf8(
    const Aq10Voice* voice, const char* koe, int* size);
AQTK_IMPORT void AQTK_CALL AquesTalk10_FreeWave(unsigned char* wav);
AQTK_IMPORT int AQTK_CALL AquesTalk10_SetDevKey(const char* key);
AQTK_IMPORT int AQTK_CALL AquesTalk10_SetUsrKey(const char* key);
#define AQ10_SYNTHE AquesTalk10_Synthe_Utf8
#define AQ10_FREE AquesTalk10_FreeWave
#define AQ10_DEV_KEY AquesTalk10_SetDevKey
#define AQ10_USR_KEY AquesTalk10_SetUsrKey
#else
AQTK_IMPORT unsigned char* AQTK_CALL AquesTalk_Synthe_Utf8(
    const Aq10Voice* voice, const char* koe, int* size);
AQTK_IMPORT void AQTK_CALL AquesTalk_FreeWave(unsigned char* wav);
AQTK_IMPORT int AQTK_CALL AquesTalk_SetDevKey(const char* key);
AQTK_IMPORT int AQTK_CALL AquesTalk_SetUsrKey(const char* key);
#define AQ10_SYNTHE AquesTalk_Synthe_Utf8
#define AQ10_FREE AquesTalk_FreeWave
#define AQ10_DEV_KEY AquesTalk_SetDevKey
#define AQ10_USR_KEY AquesTalk_SetUsrKey
#endif

}

unsigned char* aqtk10_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    Aq10Voice voice{};
    if (!voice_from_param(param, &voice)) {
        return fail_wav(size);
    }

    apply_key(AQ10_DEV_KEY, param->DevKey);
    apply_key(AQ10_USR_KEY, param->UserKey);
    unsigned char* wav = AQ10_SYNTHE(&voice, text, size);
    if (wav) {
        wav_track(wav, AQ10_FREE);
    }
    return wav;
}
