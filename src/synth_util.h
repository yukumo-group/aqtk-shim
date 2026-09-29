#pragma once

#ifdef _WIN32
#define AQTK_CALL __stdcall
#else
#define AQTK_CALL
#endif

template <typename SetKeyFn>
void apply_key(SetKeyFn set_key, const char* key) {
    if (key && key[0]) {
        set_key(key);
    }
}

inline unsigned char* fail_wav(int* size) {
    if (size) {
        *size = 0;
    }
    return nullptr;
}
