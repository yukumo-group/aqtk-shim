#include "Aqsh.h"

#include <cstdio>
#include <filesystem>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <cstdint>
#include <mach-o/dyld.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace {
    // ゆっくりしていってね
    constexpr char kText[] = u8"ゆっくりしていってね";

    struct Engine {
        const char* name;
        const char* file;
        AqVersion version;
        int preset;
    };

    std::filesystem::path exe_directory() {
#ifdef _WIN32
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
        return std::filesystem::path(path);
#elif defined(__APPLE__)
        std::string path(1024, '\0');
        uint32_t size = static_cast<uint32_t>(path.size());
        while (_NSGetExecutablePath(path.data(), &size) != 0) {
            path.resize(size);
        }
        return std::filesystem::weakly_canonical(path.c_str()).parent_path();
#else
        std::string path(PATH_MAX, '\0');
        const ssize_t n = ::readlink("/proc/self/exe", path.data(), path.size() - 1);
        if (n <= 0) {
            return {};
        }
        path.resize(static_cast<size_t>(n));
        return std::filesystem::path(path).parent_path();
#endif
    }

    bool write_wav(const std::filesystem::path& path, const unsigned char* data, int size) {
#ifdef _WIN32
        FILE* fp = nullptr;
        if (_wfopen_s(&fp, path.c_str(), L"wb") != 0 || !fp) {
            return false;
        }
#else
        FILE* fp = std::fopen(path.c_str(), "wb");
        if (!fp) {
            return false;
        }
#endif
        const auto written = std::fwrite(data, 1, static_cast<size_t>(size), fp);
        std::fclose(fp);
        return written == static_cast<size_t>(size);
    }
}

int main() {
    const Engine engines[] = {
        {"aq1", "aq1.wav", AQ_VER_1, AQ1_F1},
        {"aq2", "aq2.wav", AQ_VER_2, AQ2_AQ_YUKKURI},
        {"aq10", "aq10.wav", AQ_VER_10, AQ10_F1},
    };

    if (AqShim_Init() != 0) {
        std::fprintf(stderr, "AqShim_Init failed\n");
        return 1;
    }

    const std::filesystem::path dir = exe_directory();
    int failed = 0;
    for (const Engine& engine : engines) {
        AqSynthParam param{};
        param.version = engine.version;
        param.preset = engine.preset;
        param.speed = 100;

        unsigned char* wav = nullptr;
        int size = 0;
        const int rc = AqShim_Synthesize(kText, &param, &wav, &size);
        const std::filesystem::path path = dir / engine.file;
        if (rc != 0 || !wav || size <= 0 || !write_wav(path, wav, size)) {
            const char* message = AqShim_ErrorMessage(engine.version, rc);
            std::fprintf(stderr, "%s failed rc=%d size=%d%s%s\n",
                engine.name, rc, size, message ? " " : "", message ? message : "");
            ++failed;
        } else {
#ifdef _WIN32
            std::wprintf(L"%hs %d bytes -> %ls\n", engine.name, size, path.c_str());
#else
            std::printf("%s %d bytes -> %s\n", engine.name, size, path.c_str());
#endif
        }
        AqShim_FreeWav(wav);
    }

    AqShim_Shutdown();
    return failed == 0 ? 0 : 1;
}
