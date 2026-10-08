#pragma once

#include <string>

#ifdef _WIN32
#define AQTK_CALL __stdcall
#define AQTK_IMPORT __declspec(dllimport)
#else
#define AQTK_CALL
#define AQTK_IMPORT __attribute__((visibility("default")))
#endif

template <typename SetKeyFn>
void apply_key(SetKeyFn set_key, const char* key) {
    if (key && key[0]) {
        set_key(key);
    }
}

// The key from AqSynthParam wins. Falls back to the key AqShim_Init read from
// .env, and to nothing when that is empty too. apply_key skips the null.
inline const char* pick_key(const char* param_key, const std::string& env_key) {
    if (param_key && param_key[0]) {
        return param_key;
    }
    return env_key.empty() ? nullptr : env_key.c_str();
}

inline unsigned char* fail_wav(int* size) {
    if (size) {
        *size = 0;
    }
    return nullptr;
}
