#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "aqtk1.h"

#include "Aqsh.h"

#include <array>
#include <mutex>
#include <string>
#include <unordered_map>

namespace {
    using SyntheUtf8Fn = unsigned char* (__stdcall*)(const char* koe, int speed, int* size);
    using FreeWaveFn = void (__stdcall*)(unsigned char* wav);
    using SetKeyFn = int (__stdcall*)(const char* key);

    struct VoiceLib {
        HMODULE module = nullptr;
        SyntheUtf8Fn synthe = nullptr;
        FreeWaveFn free_wave = nullptr;
        SetKeyFn set_dev_key = nullptr;
        SetKeyFn set_usr_key = nullptr;
    };

    struct Registry {
        std::mutex mu;
        std::wstring dir;
        std::array<VoiceLib, AQ1_PRESET_COUNT> voices{};
        std::unordered_map<unsigned char*, FreeWaveFn> frees;
    };

    // Same order as Aq1Preset. Staged next to Aqsh.dll; the SDK copies are all named AquesTalk.dll.
    constexpr std::array<const wchar_t*, AQ1_PRESET_COUNT> kDllNames = {
        L"AquesTalk1_f1.dll",
        L"AquesTalk1_f2.dll",
        L"AquesTalk1_f3.dll",
        L"AquesTalk1_m1.dll",
        L"AquesTalk1_m2.dll",
        L"AquesTalk1_r1.dll",
        L"AquesTalk1_dvd.dll",
        L"AquesTalk1_imd1.dll",
        L"AquesTalk1_jgr.dll",
    };

    Registry& registry() {
        static Registry reg;
        return reg;
    }

    std::wstring aqsh_dir() {
        HMODULE self = nullptr;
        static int anchor = 0;
        if (!GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(&anchor),
                &self)) {
            return {};
        }

        std::wstring path(MAX_PATH, L'\0');
        for (;;) {
            const DWORD n = GetModuleFileNameW(self, path.data(), static_cast<DWORD>(path.size()));
            if (n == 0) {
                return {};
            }
            if (n < path.size()) {
                path.resize(n);
                break;
            }
            path.resize(path.size() * 2);
        }

        const auto slash = path.find_last_of(L"\\/");
        if (slash == std::wstring::npos) {
            return {};
        }
        path.resize(slash + 1);
        return path;
    }

    void apply_key(SetKeyFn set_key, const char* key) {
        if (key && key[0]) {
            set_key(key);
        }
    }

    VoiceLib* open_voice(Registry& reg, int preset) {
        if (preset < 0 || preset >= AQ1_PRESET_COUNT) {
            return nullptr;
        }

        VoiceLib& voice = reg.voices[preset];
        if (voice.module) {
            return &voice;
        }

        if (reg.dir.empty()) {
            reg.dir = aqsh_dir();
            if (reg.dir.empty()) {
                return nullptr;
            }
        }

        const std::wstring path = reg.dir + kDllNames[preset];
        const HMODULE module = LoadLibraryExW(
            path.c_str(),
            nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!module) {
            return nullptr;
        }

        const auto synthe = reinterpret_cast<SyntheUtf8Fn>(GetProcAddress(module, "AquesTalk_Synthe_Utf8"));
        const auto free_wave = reinterpret_cast<FreeWaveFn>(GetProcAddress(module, "AquesTalk_FreeWave"));
        const auto set_dev_key = reinterpret_cast<SetKeyFn>(GetProcAddress(module, "AquesTalk_SetDevKey"));
        const auto set_usr_key = reinterpret_cast<SetKeyFn>(GetProcAddress(module, "AquesTalk_SetUsrKey"));
        if (!synthe || !free_wave || !set_dev_key || !set_usr_key) {
            FreeLibrary(module);
            return nullptr;
        }

        voice.module = module;
        voice.synthe = synthe;
        voice.free_wave = free_wave;
        voice.set_dev_key = set_dev_key;
        voice.set_usr_key = set_usr_key;
        return &voice;
    }
}

unsigned char* aqtk1_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    SyntheUtf8Fn synthe = nullptr;
    FreeWaveFn free_wave = nullptr;
    SetKeyFn set_dev_key = nullptr;
    SetKeyFn set_usr_key = nullptr;
    {
        auto& reg = registry();
        std::lock_guard lock(reg.mu);
        VoiceLib* voice = open_voice(reg, param->preset);
        if (!voice) {
            if (size) {
                *size = 0;
            }
            return nullptr;
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
        std::lock_guard lock(registry().mu);
        registry().frees.emplace(wav, free_wave);
    }
    return wav;
}

void aqtk1_free(unsigned char* wav) {
    if (!wav) {
        return;
    }

    FreeWaveFn free_wave = nullptr;
    {
        auto& reg = registry();
        std::lock_guard lock(reg.mu);
        const auto it = reg.frees.find(wav);
        if (it == reg.frees.end()) {
            return;
        }
        free_wave = it->second;
        reg.frees.erase(it);
    }
    free_wave(wav);
}
