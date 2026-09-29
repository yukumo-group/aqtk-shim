#pragma once

// Stand-in JNIEnv for the Android AquesTalk1 and AquesTalk2 libraries.
unsigned char* jni_synthe_wav(
    void* synthe_fn,
    const char* text,
    int speed,
    const unsigned char* phont,
    int phont_size,
    int* size);

void jni_free_wav(unsigned char* wav);

// No-op when set_key_fn or key is null or empty.
void jni_set_key(void* set_key_fn, const char* key);
