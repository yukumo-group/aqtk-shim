#include "module_dir.h"

#include <string>

#ifdef __ANDROID__
#include <android/dlext.h>
#endif

std::filesystem::path module_dir() {
#ifdef _WIN32
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
    return std::filesystem::path(path).parent_path();
#else
    Dl_info info{};
    if (dladdr(reinterpret_cast<const void*>(&module_dir), &info) == 0 || !info.dli_fname) {
        return {};
    }
    return std::filesystem::path(info.dli_fname).parent_path();
#endif
}

std::filesystem::path path_from_utf8(const char* text) {
    if (!text || !text[0]) {
        return {};
    }
#ifdef _WIN32
    const int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, nullptr, 0);
    if (n <= 1) {
        return {};
    }
    std::wstring out(static_cast<size_t>(n - 1), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, out.data(), n) == 0) {
        return {};
    }
    return out;
#else
    return text;
#endif
}

ModuleHandle module_open(const std::filesystem::path& path) {
#ifdef _WIN32
    return LoadLibraryExW(
        path.c_str(),
        nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
#elif defined(__ANDROID__)
    // AquesTalk1 and AquesTalk10 share SONAME libAquesTalk.so. FORCE_LOAD maps
    // this file even when that name is already loaded for AquesTalk10.
    android_dlextinfo info{};
    info.flags = ANDROID_DLEXT_FORCE_LOAD;
    return android_dlopen_ext(path.c_str(), RTLD_NOW | RTLD_LOCAL, &info);
#else
    // RTLD_LOCAL: every AquesTalk1 voice exports AquesTalk_*, as does AquesTalk10.
    return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
}

void module_close(ModuleHandle module) {
#ifdef _WIN32
    FreeLibrary(module);
#else
    dlclose(module);
#endif
}
