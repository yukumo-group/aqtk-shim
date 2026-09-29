#include "jni_bridge.h"

#include <cstdlib>

namespace {
    // Slots in JNINativeInterface. Same indexes on every ABI; the wrappers
    // scale them by sizeof(void*).
    constexpr int kGetStringUTFChars = 169;
    constexpr int kReleaseStringUTFChars = 170;
    constexpr int kNewByteArray = 176;
    constexpr int kGetByteArrayElements = 184;
    constexpr int kReleaseByteArrayElements = 192;

    struct JniRef {
        const char* chars = nullptr;
        unsigned char* bytes = nullptr;
        int size = 0;
        bool owned = false;
    };

    struct Env {
        void** table;
    };

    using Synthe1 = void* (*)(void* env, void* self, void* text, int speed);
    using Synthe2 = void* (*)(void* env, void* self, void* text, int speed, void* phont);
    using SetKey = int (*)(void* env, void* self, void* key);

    void release_ref(JniRef* ref) {
        if (!ref) {
            return;
        }
        if (ref->owned) {
            std::free(ref->bytes);
        }
        std::free(ref);
    }

    extern "C" {
        const char* aq_get_string_utf_chars(Env*, JniRef* str, unsigned char* is_copy) {
            if (is_copy) {
                *is_copy = 0;
            }
            return str ? str->chars : nullptr;
        }

        void aq_release_string_utf_chars(Env*, JniRef*, const char*) {}

        JniRef* aq_new_byte_array(Env*, int length) {
            auto* ref = static_cast<JniRef*>(std::calloc(1, sizeof(JniRef)));
            if (!ref) {
                return nullptr;
            }
            ref->size = length;
            if (length > 0) {
                ref->bytes = static_cast<unsigned char*>(std::malloc(static_cast<size_t>(length)));
                if (!ref->bytes) {
                    std::free(ref);
                    return nullptr;
                }
                ref->owned = true;
            }
            return ref;
        }

        unsigned char* aq_get_byte_array_elements(Env*, JniRef* array, unsigned char* is_copy) {
            if (is_copy) {
                *is_copy = 0;
            }
            return array ? array->bytes : nullptr;
        }

        void aq_release_byte_array_elements(Env*, JniRef*, unsigned char*, int) {}
    }

    void* jni_env() {
        static void* table[193];
        static Env env = [] {
            table[kGetStringUTFChars] = reinterpret_cast<void*>(aq_get_string_utf_chars);
            table[kReleaseStringUTFChars] = reinterpret_cast<void*>(aq_release_string_utf_chars);
            table[kNewByteArray] = reinterpret_cast<void*>(aq_new_byte_array);
            table[kGetByteArrayElements] = reinterpret_cast<void*>(aq_get_byte_array_elements);
            table[kReleaseByteArrayElements] = reinterpret_cast<void*>(aq_release_byte_array_elements);
            return Env{table};
        }();
        return &env;
    }
}

unsigned char* jni_synthe_wav(
    void* synthe_fn,
    const char* text,
    int speed,
    const unsigned char* phont,
    int phont_size,
    int* size) {
    if (!synthe_fn || !text || !size) {
        if (size) {
            *size = 0;
        }
        return nullptr;
    }

    JniRef text_ref{};
    text_ref.chars = text;
    JniRef phont_ref{};
    phont_ref.bytes = const_cast<unsigned char*>(phont);
    phont_ref.size = phont_size;

    void* env = jni_env();
    void* result = phont
        ? reinterpret_cast<Synthe2>(synthe_fn)(env, nullptr, &text_ref, speed, &phont_ref)
        : reinterpret_cast<Synthe1>(synthe_fn)(env, nullptr, &text_ref, speed);

    auto* out = static_cast<JniRef*>(result);
    if (!out || !out->owned || out->size <= 1 || !out->bytes) {
        const int err = (out && out->size == 1 && out->bytes)
            ? static_cast<unsigned char>(out->bytes[0])
            : 0;
        release_ref(out);
        *size = err;
        return nullptr;
    }

    unsigned char* wav = out->bytes;
    *size = out->size;
    out->owned = false;
    release_ref(out);
    return wav;
}

void jni_free_wav(unsigned char* wav) {
    std::free(wav);
}

void jni_set_key(void* set_key_fn, const char* key) {
    if (!set_key_fn || !key || !key[0]) {
        return;
    }
    JniRef key_ref{};
    key_ref.chars = key;
    reinterpret_cast<SetKey>(set_key_fn)(jni_env(), nullptr, &key_ref);
}
