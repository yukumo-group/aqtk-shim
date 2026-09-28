#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "Aqsh.h"

#include <cstdio>
#include <string>

namespace {
    // ゆっくりしていってね
    constexpr char kText[] = u8"ゆっくりしていってね";

    struct Engine {
        const char* name;
        const wchar_t* file;
        AqVersion version;
        int preset;
    };

    std::wstring exe_directory() {
        std::wstring path(MAX_PATH, L'\0');
        for (;;) {
            const DWORD n = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
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

    bool write_wav(const std::wstring& path, const unsigned char* data, int size) {
        FILE* fp = nullptr;
        if (_wfopen_s(&fp, path.c_str(), L"wb") != 0 || !fp) {
            return false;
        }
        const auto written = std::fwrite(data, 1, static_cast<size_t>(size), fp);
        std::fclose(fp);
        return written == static_cast<size_t>(size);
    }
}

int main() {
    const Engine engines[] = {
        {"aq1", L"aq1.wav", AQ_VER_1, AQ1_F1},
        {"aq2", L"aq2.wav", AQ_VER_2, AQ2_AQ_YUKKURI},
        {"aq10", L"aq10.wav", AQ_VER_10, AQ10_F1},
    };

    if (AqShim_Init() != 0) {
        std::fprintf(stderr, "AqShim_Init failed\n");
        return 1;
    }

    const std::wstring dir = exe_directory();
    int failed = 0;
    for (const Engine& engine : engines) {
        AqSynthParam param{};
        param.version = engine.version;
        param.preset = engine.preset;
        param.speed = 100;

        unsigned char* wav = nullptr;
        int size = 0;
        const int rc = AqShim_Synthesize(kText, &param, &wav, &size);
        const std::wstring path = dir + engine.file;
        if (rc != 0 || !wav || size <= 0 || !write_wav(path, wav, size)) {
            std::fprintf(stderr, "%s failed rc=%d size=%d\n", engine.name, rc, size);
            ++failed;
        } else {
            std::printf("%s %d bytes -> %ls\n", engine.name, size, path.c_str());
        }
        AqShim_FreeWav(wav);
    }

    AqShim_Shutdown();
    return failed == 0 ? 0 : 1;
}
