#include "aqtk2.h"

#include "Aqsh.h"
#include "module_dir.h"
#include "synth_util.h"
#include "wav_track.h"

#include <filesystem>
#include <fstream>
#include <vector>

namespace {
    // Same order as Aq2Preset. Staged in phont/ next to the shim library.
    constexpr const char* kPhontNames[] = {
        "aq_yukkuri.phont",
        "aq_defo1.phont",
        "aq_f1c.phont",
        "aq_f3a.phont",
        "aq_huskey.phont",
        "aq_m4b.phont",
        "aq_mf1.phont",
        "aq_rb2.phont",
        "aq_rb3.phont",
        "aq_rm.phont",
        "aq_robo.phont",
        "aq_teto1.phont",
        "ar_f4.phont",
        "ar_m5.phont",
        "ar_mf2.phont",
        "ar_rm3.phont",
    };
    static_assert(sizeof(kPhontNames) / sizeof(kPhontNames[0]) == AQ2_PRESET_COUNT);

    std::filesystem::path phont_file(const AqSynthParam* param) {
        if (param->phont && param->phont[0]) {
            return path_from_utf8(param->phont);
        }
        if (param->preset < 0 || param->preset >= AQ2_PRESET_COUNT) {
            return {};
        }
        const auto dir = module_dir();
        if (dir.empty()) {
            return {};
        }
        return dir / "phont" / kPhontNames[param->preset];
    }

    bool read_file(const std::filesystem::path& path, std::vector<unsigned char>* out) {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            return false;
        }
        in.seekg(0, std::ios::end);
        const auto n = in.tellg();
        if (n <= 0 || n > 1024 * 1024) {
            return false;
        }
        in.seekg(0, std::ios::beg);
        out->resize(static_cast<size_t>(n));
        in.read(reinterpret_cast<char*>(out->data()), n);
        return static_cast<std::streamsize>(out->size()) == in.gcount();
    }
}

extern "C" {

AQTK_IMPORT unsigned char* AQTK_CALL AquesTalk2_Synthe_Utf8(
    const char* koe, int speed, int* size, void* phont);

AQTK_IMPORT void AQTK_CALL AquesTalk2_FreeWave(unsigned char* wav);

}

unsigned char* aqtk2_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    std::vector<unsigned char> phont;
    const auto path = phont_file(param);
    if (path.empty() || !read_file(path, &phont)) {
        return fail_wav(size);
    }

    unsigned char* wav = AquesTalk2_Synthe_Utf8(text, param->speed, size, phont.data());
    if (wav) {
        wav_track(wav, AquesTalk2_FreeWave);
    }
    return wav;
}
