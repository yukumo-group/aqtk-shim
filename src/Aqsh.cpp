#ifndef AQSWRAPPER_EXPORTS
#define AQSWRAPPER_EXPORTS
#endif
#include "Aqsh.h"

#include "aqtk1.h"
#include "aqtk2.h"
#include "aqtk10.h"
#include "wav_track.h"

namespace {
    enum {
        AQSH_OK = 0,
        AQSH_ERR_ARG = -1,
        AQSH_ERR_VERSION = -2,
        AQSH_ERR_SYNTH = -3
    };

    int finish_synth(unsigned char* wav, int size, unsigned char** out_wav_data, int* out_wav_size) {
        if (!wav) {
            *out_wav_data = nullptr;
            *out_wav_size = 0;
            return size != 0 ? size : AQSH_ERR_SYNTH;
        }
        *out_wav_data = wav;
        *out_wav_size = size;
        return AQSH_OK;
    }
}

extern "C" {

AQSH_API int AqShim_Init(void) {
    return AQSH_OK;
}

AQSH_API void AqShim_Shutdown(void) {
    wav_release_all();
}

AQSH_API int AqShim_Synthesize(
    const char* text,
    const AqSynthParam* param,
    unsigned char** out_wav_data,
    int* out_wav_size) {
    if (!text || !param || !out_wav_data || !out_wav_size) {
        return AQSH_ERR_ARG;
    }

    int size = 0;
    unsigned char* wav = nullptr;
    switch (param->version) {
    case AQ_VER_1:
        wav = aqtk1_synthe_utf8(param, text, &size);
        break;
    case AQ_VER_2:
        wav = aqtk2_synthe_utf8(param, text, &size);
        break;
    case AQ_VER_10:
        wav = aqtk10_synthe_utf8(param, text, &size);
        break;
    default:
        return AQSH_ERR_VERSION;
    }
    return finish_synth(wav, size, out_wav_data, out_wav_size);
}

AQSH_API void AqShim_FreeWav(unsigned char* wav_data) {
    wav_release(wav_data);
}

}
