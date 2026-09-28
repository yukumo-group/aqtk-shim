#include "Aqsh.h"

#include <mutex>
#include <unordered_map>

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

    enum {
        AQSH_OK = 0,
        AQSH_ERR_ARG = -1,
        AQSH_ERR_VERSION = -2,
        AQSH_ERR_SYNTH = -3
    };

    struct WavTable {
        std::mutex mu;
        std::unordered_map<unsigned char*, AqVersion> owners;
    };

    WavTable& wav_table() {
        static WavTable table;
        return table;
    }

    void apply_key(int (__stdcall* set_key)(const char*), const char* key) {
        if (key && key[0]) {
            set_key(key);
        }
    }

    void track(unsigned char* wav, AqVersion owner) {
        std::lock_guard lock(wav_table().mu);
        wav_table().owners.emplace(wav, owner);
    }

    int finish_synth(unsigned char* wav, const int size, const AqVersion owner,
                     unsigned char** out_wav_data, int* out_wav_size) {
        if (!wav) {
            *out_wav_data = nullptr;
            *out_wav_size = 0;
            return size != 0 ? size : AQSH_ERR_SYNTH;
        }
        track(wav, owner);
        *out_wav_data = wav;
        *out_wav_size = size;
        return AQSH_OK;
    }
}

extern "C" {

__declspec(dllimport) unsigned char* __stdcall AquesTalk1_Synthe_Utf8(
    const char* koe, int speed, int* size);
__declspec(dllimport) void __stdcall AquesTalk1_FreeWave(unsigned char* wav);
__declspec(dllimport) int __stdcall AquesTalk1_SetDevKey(const char* key);
__declspec(dllimport) int __stdcall AquesTalk1_SetUsrKey(const char* key);

__declspec(dllimport) unsigned char* __stdcall AquesTalk2_Synthe_Utf8(
    const char* koe, int speed, int* size, void* phont);
__declspec(dllimport) void __stdcall AquesTalk2_FreeWave(unsigned char* wav);

__declspec(dllimport) unsigned char* __stdcall AquesTalk10_Synthe_Utf8(
    const Aq10Voice* voice, const char* koe, int* size);
__declspec(dllimport) void __stdcall AquesTalk10_FreeWave(unsigned char* wav);
__declspec(dllimport) int __stdcall AquesTalk10_SetDevKey(const char* key);
__declspec(dllimport) int __stdcall AquesTalk10_SetUsrKey(const char* key);

AQSH_API int AqShim_Init(void) {
    return AQSH_OK;
}

AQSH_API void AqShim_Shutdown(void) {
    std::unordered_map<unsigned char*, AqVersion> pending;
    {
        std::lock_guard lock(wav_table().mu);
        pending.swap(wav_table().owners);
    }
    for (const auto&[fst, snd] : pending) {
        switch (snd) {
        case AQ_VER_1:
            AquesTalk1_FreeWave(fst);
            break;
        case AQ_VER_2:
            AquesTalk2_FreeWave(fst);
            break;
        case AQ_VER_10:
            AquesTalk10_FreeWave(fst);
            break;
        }
    }
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
        apply_key(AquesTalk1_SetDevKey, param->DevKey);
        apply_key(AquesTalk1_SetUsrKey, param->UserKey);
        wav = AquesTalk1_Synthe_Utf8(text, param->speed, &size);
    case AQ_VER_2:
        wav = AquesTalk2_Synthe_Utf8(text, param->speed, &size, nullptr);
    case AQ_VER_10: {
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
        wav = AquesTalk10_Synthe_Utf8(&voice, text, &size);
    }
    default:
        return AQSH_ERR_VERSION;
    }
    return finish_synth(wav, size, param->version, out_wav_data, out_wav_size);
}

AQSH_API void AqShim_FreeWav(unsigned char* wav_data) {
    if (!wav_data) {
        return;
    }

    AqVersion owner;
    {
        std::lock_guard lock(wav_table().mu);
        const auto it = wav_table().owners.find(wav_data);
        if (it == wav_table().owners.end()) {
            return;
        }
        owner = it->second;
        wav_table().owners.erase(it);
    }

    switch (owner) {
    case AQ_VER_1:
        AquesTalk1_FreeWave(wav_data);
        break;
    case AQ_VER_2:
        AquesTalk2_FreeWave(wav_data);
        break;
    case AQ_VER_10:
        AquesTalk10_FreeWave(wav_data);
        break;
    }
}

}
