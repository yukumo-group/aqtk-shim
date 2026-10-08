#ifndef AQSWRAPPER_EXPORTS
#define AQSWRAPPER_EXPORTS
#endif
#include "Aqsh.h"

#include "aqtk1.h"
#include "aqtk2.h"
#include "aqtk10.h"
#include "env_keys.h"
#include "module_dir.h"
#include "wav_track.h"
#include <include/dotenv.h>

#include <filesystem>
#include <string>
#include <system_error>

namespace {
    enum {
        AQSH_OK = 0,
        AQSH_ERR_ARG = -1,
        AQSH_ERR_VERSION = -2,
        AQSH_ERR_SYNTH = -3,
        AQSH_ERR_ENV = -4
    };

    // License keys AqShim_Init reads from .env, per engine.
    struct EnvRow {
        AqVersion version;
        const char* dev_key;
        const char* usr_key;
    };

    // AquesTalk2 has no key functions, so it has no row.
    constexpr EnvRow kEnvRows[] = {
        {AQ_VER_1, "AQ01_DEV_KEY", "AQ01_USR_KEY"},
        {AQ_VER_10, "AQ10_DEV_KEY", "AQ10_USR_KEY"},
    };

    // Used when AqShim_Init gets no path.
    constexpr const char* kEnvFile = ".env";

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

    int load_env(const char* envPath) {
        std::filesystem::path path;
        std::error_code ec;
        if (envPath && envPath[0]) {
            path = path_from_utf8(envPath);
            if (path.empty()) {
                return AQSH_ERR_ENV;
            }
            if (is_directory(path, ec)) {
                path /= kEnvFile;
            }
            if (!is_regular_file(path, ec)) {
                return AQSH_ERR_ENV;
            }
        } else {
            path = kEnvFile;
            if (!is_regular_file(path, ec)) {
                return AQSH_OK;
            }
        }

        // dotenv opens the path as a narrow string, so hand it UTF-8.
        const dotenv env(path.u8string());
        for (const EnvRow& row : kEnvRows) {
            set_env_keys(row.version, env.get(row.dev_key), env.get(row.usr_key));
        }
        return AQSH_OK;
    }
}

extern "C" {

AQSH_API int AqShim_Init(const char* envPath) {
    return load_env(envPath);
}

AQSH_API void AqShim_Shutdown(void) {
    wav_release_all();
    clear_env_keys();
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
