#pragma once

#include <filesystem>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
using ModuleHandle = HMODULE;
#else
#include <dlfcn.h>
using ModuleHandle = void*;
#endif

// Directory containing this shared library.
std::filesystem::path module_dir();

// UTF-8 path. Empty when text is null, empty, or not valid UTF-8.
std::filesystem::path path_from_utf8(const char* text);

ModuleHandle module_open(const std::filesystem::path& path);
void module_close(ModuleHandle module);

template <typename Fn>
Fn module_symbol(ModuleHandle module, const char* name) {
#ifdef _WIN32
    return reinterpret_cast<Fn>(GetProcAddress(module, name));
#else
    return reinterpret_cast<Fn>(dlsym(module, name));
#endif
}
