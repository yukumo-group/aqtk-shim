#include "Aqsh.h"

#include <list>
#include <mutex>
#include <string>

namespace {
    struct ErrorRow {
        int version;
        int code;
        const char* name;
    };

    // version 0 is shared by every engine.
    constexpr ErrorRow kErrorRows[] = {
        {0, 100, "OTHER_ERROR"},
        {0, 101, "INSUFFICIENT_MEMORY"},
        {0, 105, "UNDEFINED_READING_SYMBOL"},
        {0, 106, "INVALID_TAG"},
        {0, 107, "TAG_LENGTH_EXCEEDED"},
        {0, 108, "INVALID_TAG_VALUE"},

        {AQ_VER_1, 200, "PHONETIC_TOO_LONG"},
        {AQ_VER_1, 201, "TOO_MANY_READING_SYMBOLS"},
        {AQ_VER_1, 202, "PHONETIC_TOO_LONG"},
        {AQ_VER_1, 203, "INSUFFICIENT_HEAP_MEMORY"},
        {AQ_VER_1, 204, "PHONETIC_TOO_LONG"},

        {AQ_VER_2, 102, "UNDEFINED_READING_SYMBOL"},
        {AQ_VER_2, 103, "NEGATIVE_PROSODY_DURATION"},
        {AQ_VER_2, 104, "UNDEFINED_DELIMITER"},
        {AQ_VER_2, 111, "NO_SPEECH_DATA"},
        {AQ_VER_2, 200, "PHONETIC_TOO_LONG"},
        {AQ_VER_2, 201, "TOO_MANY_READING_SYMBOLS"},
        {AQ_VER_2, 202, "PHONETIC_BUFFER_OVERFLOW"},
        {AQ_VER_2, 203, "INSUFFICIENT_HEAP_MEMORY"},
        {AQ_VER_2, 204, "PHONETIC_BUFFER_OVERFLOW"},

        {AQ_VER_10, 103, "INVALID_PHONETIC_SPEC"},
        {AQ_VER_10, 104, "NO_VALID_READING"},
        {AQ_VER_10, 120, "PHONETIC_TOO_LONG"},
        {AQ_VER_10, 121, "TOO_MANY_READING_SYMBOLS"},
        {AQ_VER_10, 122, "PHONETIC_BUFFER_OVERFLOW"},
    };

    const char* engine_prefix(AqVersion version) {
        switch (version) {
        case AQ_VER_1: return "AQ1_";
        case AQ_VER_2: return "AQ2_";
        case AQ_VER_10: return "AQ10_";
        default: return nullptr;
        }
    }

    const char* error_name(AqVersion version, int code) {
        if (version == AQ_VER_2 && code >= 1000 && code <= 1008) {
            return "INVALID_PHONT";
        }
        for (const ErrorRow& row : kErrorRows) {
            if (row.code == code && (row.version == 0 || row.version == version)) {
                return row.name;
            }
        }
        return nullptr;
    }

    // Prefix and name are literals, so pointer identity is a stable cache key.
    const char* join(const char* prefix, const char* name) {
        struct Item {
            const char* prefix;
            const char* name;
            std::string text;
        };
        static std::mutex mu;
        static std::list<Item> items;
        std::lock_guard lock(mu);
        for (const Item& item : items) {
            if (item.prefix == prefix && item.name == name) {
                return item.text.c_str();
            }
        }
        items.push_back({prefix, name, std::string(prefix) + name});
        return items.back().text.c_str();
    }
}

extern "C" {

AQSH_API const char* AqShim_ErrorMessage(AqVersion version, int code) {
    if (code < 0) {
        code = -code;
    }
    const char* prefix = engine_prefix(version);
    const char* name = error_name(version, code);
    if (!prefix || !name) {
        return nullptr;
    }
    return join(prefix, name);
}

}
