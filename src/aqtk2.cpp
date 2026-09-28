#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "aqtk2.h"

#include "Aqsh.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
    // Same order as Aq2Preset. Staged in phont/ next to Aqsh.dll.
    constexpr const wchar_t* kPhontNames[] = {
        L"aq_f1c.phont",
        L"aq_f3a.phont",
        L"aq_huskey.phont",
        L"aq_m4b.phont",
        L"aq_mf1.phont",
        L"aq_rb2.phont",
        L"aq_rb3.phont",
        L"aq_rm.phont",
        L"aq_robo.phont",
        L"aq_yukkuri.phont",
        L"ar_f4.phont",
        L"ar_m5.phont",
        L"ar_mf2.phont",
        L"ar_rm3.phont",
    };
    static_assert(sizeof(kPhontNames) / sizeof(kPhontNames[0]) == AQ2_PRESET_COUNT);

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

    std::wstring utf8_to_wide(const char* text) {
        const int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, nullptr, 0);
        if (n <= 1) {
            return {};
        }
        std::wstring out(static_cast<size_t>(n - 1), L'\0');
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, out.data(), n) == 0) {
            return {};
        }
        return out;
    }

    bool read_file(const std::wstring& path, std::vector<unsigned char>* out) {
        std::ifstream in(std::filesystem::path(path), std::ios::binary);
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

    std::wstring phont_path(const AqSynthParam* param) {
        if (param->phont && param->phont[0]) {
            return utf8_to_wide(param->phont);
        }
        if (param->preset < 0 || param->preset >= AQ2_PRESET_COUNT) {
            return {};
        }
        const std::wstring dir = aqsh_dir();
        if (dir.empty()) {
            return {};
        }
        return dir + L"phont\\" + kPhontNames[param->preset];
    }
}

extern "C" {

__declspec(dllimport) unsigned char* __stdcall AquesTalk2_Synthe_Utf8(
    const char* koe, int speed, int* size, void* phont);
__declspec(dllimport) void __stdcall AquesTalk2_FreeWave(unsigned char* wav);

}

unsigned char* aqtk2_synthe_utf8(const AqSynthParam* param, const char* text, int* size) {
    std::vector<unsigned char> phont;
    const std::wstring path = phont_path(param);
    if (path.empty() || !read_file(path, &phont)) {
        if (size) {
            *size = 0;
        }
        return nullptr;
    }
    return AquesTalk2_Synthe_Utf8(text, param->speed, size, phont.data());
}

void aqtk2_free(unsigned char* wav) {
    AquesTalk2_FreeWave(wav);
}
