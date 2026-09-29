#include "aqtk1.h"

#include "Aqsh.h"
#include "module_dir.h"
#include "synth_util.h"
#include "wav_track.h"

#include <array>
#include <mutex>
#include <string>

namespace {
    using SyntheUtf8Fn = unsigned char* (AQTK_CALL*)(const char* koe, int speed, int* size);
    using SetKeyFn = int (AQTK_CALL*)(const char* key);

    struct VoiceLib {
        ModuleHandle module = nullptr;
        SyntheUtf8Fn synthe = nullptr;
        WavFreeFn free_wave = nullptr;
        SetKeyFn set_dev_key = nullptr;
        SetKeyFn set_usr_key = nullptr;
    };

    struct Registry {
        std::mutex mu;
        std::array<VoiceLib, AQ1_PRESET_COUNT> voices{};
    };

    // Same order as Aq1Preset. Not linked: each voice exports AquesTalk_*,
    // which collides with the other voices and with AquesTalk10.
    constexpr std::array<const char*, AQ1_PRESET_COUNT> kVoices = {
        "f1", "f2", "f3", "m1", "m2", "r1", "dvd", "imd1", "jgr",
    };

    Registry& registry() {
        static Registry reg;
        return reg;
    }

    std::filesystem::path voice_library(const char* voice) {
        const auto dir = module_dir();
        if (dir.empty()) {
            return {};
        }
#ifdef _WIN32
        return dir / (std::string("AquesTalk1_") + voice + ".dll");
#else
        // Same staged names as cmake/platform/macos.cmake and linux.cmake.
        const char* ext =
#ifdef __APPLE__
            ".dylib";
#else
            ".so";
#endif
        return dir / (std::string("libAquesTalk1-") + voice + ext);
#endif
    }

    bool bind_voice(ModuleHandle module, VoiceLib* voice) {
        const auto synthe = module_symbol<SyntheUtf8Fn>(module, "AquesTalk_Synthe_Utf8");
        const auto free_wave = module_symbol<WavFreeFn>(module, "AquesTalk_FreeWave");
        const auto set_dev_key = module_symbol<SetKeyFn>(module, "AquesTalk_SetDevKey");
        const auto set_usr_key = module_symbol<SetKeyFn>(module, "AquesTalk_SetUsrKey");
        if (!synthe || !free_wave || !set_dev_key || !set_usr_key) {
            module_close(module);
            return false;
        }
        voice->module = module;
        voice->synthe = synthe;
        voice->free_wave = free_wave;
        voice->set_dev_key = set_dev_key;
        voice->set_usr_key = set_usr_key;
        return true;
    }

    VoiceLib* open_voice(Registry& reg, int preset) {
        if (preset < 0 || preset >= AQ1_PRESET_COUNT) {
            return nullptr;
        }

        VoiceLib& voice = reg.voices[preset];
        if (voice.module) {
            return &voice;
        }

        const auto path = voice_library(kVoices[preset]);
        if (path.empty()) {
            return nullptr;
        }
        const ModuleHandle module = module_open(path);
        if (!module || !bind_voice(module, &voice)) {
            return nullptr;
        }
        return &voice;
    }
}

unsigned char* aqtk1_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    SyntheUtf8Fn synthe = nullptr;
    WavFreeFn free_wave = nullptr;
    SetKeyFn set_dev_key = nullptr;
    SetKeyFn set_usr_key = nullptr;
    {
        auto& reg = registry();
        std::lock_guard lock(reg.mu);
        VoiceLib* voice = open_voice(reg, param->preset);
        if (!voice) {
            return fail_wav(size);
        }
        synthe = voice->synthe;
        free_wave = voice->free_wave;
        set_dev_key = voice->set_dev_key;
        set_usr_key = voice->set_usr_key;
    }

    apply_key(set_dev_key, param->DevKey);
    apply_key(set_usr_key, param->UserKey);
    unsigned char* wav = synthe(text, param->speed, size);
    if (wav) {
        wav_track(wav, free_wave);
    }
    return wav;
}
