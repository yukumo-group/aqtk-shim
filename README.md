# aqtk-shim

Compatibility layer for AquesTalk 1, 2, and 10. One C API, `Aqsh`, loads the library for the selected engine.

[English](README.md) · [中文](README.zh.md)

## Dependencies

CMake 3.20 or newer, and a C++17 compiler.

- Windows: Visual Studio 2022 (MSVC). The import libraries are MSVC-only.
- macOS: arm64.
- Linux: x86_64 or i386. The 32-bit build needs a multilib toolchain (`-m32`).

Put the AquesTalk SDKs in `third-party/`. Configure fails if a required library is missing. A missing AquesTalk2 phont is skipped.

```
third-party/
  tk1/aqtk1_win/<lib>/<voice>/AquesTalk.dll
  tk1/aqtk1_mac/lib/libAquesTalk1-<voice>.dylib
  tk1/aqtk1_lnx/<lib>/<voice>/libAquesTalk.so
  tk2/aqtk2-win/<lib>/AquesTalk2.dll
  tk2/aqtk2-win/<lib>/AquesTalk2.lib
  tk2/aqtk2-win/phont/
  tk2/aqtk2_mac/lib/libAquesTalk2Eva.dylib
  tk2/aqtk2_mac/phont/
  tk2/aqtk2-lnx/<lib>/libAquesTalk2Eva.so.2.3
  tk2/aqtk2-lnx/phont/
  tk10/aqtk10_win/<lib>/AquesTalk.dll
  tk10/aqtk10_mac/lib/libAquesTalk10.dylib
  tk10/aqtk10_lnx/<lib>/libAquesTalk10.so.1.1
```

`<lib>` is `lib64` for 64-bit and `lib` for 32-bit. On Linux, AquesTalk1 and AquesTalk10 use `lib64` / `lib32`; AquesTalk2 uses `lib64` / `lib`.

`<voice>` is `f1`, `f2`, `f3`, `m1`, `m2`, `r1`, `dvd`, `imd1`, `jgr`. All nine are required. Only the tree for the platform you build is required.

## Build

Windows x64:

```
cmake --preset x64
cmake --build --preset x64-release
```

Other configure presets: `x86`, `macos`, `linux`, `linux32`. Matching build presets are in `CMakePresets.json`.

The build copies the engine libraries and `phont/` next to `Aqsh`. `aqtk_demo` writes `aq1.wav`, `aq2.wav`, and `aq10.wav` beside itself:

```
build/x64/Release/aqtk_demo.exe
build/macos/aqtk_demo
build/linux/aqtk_demo
```

`AQTK_STAGE_PHONTS=OFF` skips copying `phont/`. Pass phont paths yourself in that case.

Install places `Aqsh`, the renamed engine libraries, `phont/`, and `Aqsh.h` in `../third-party/<arch>` (`win64`, `win32`, `macos`, `linux64`, `linux32`):

```
cmake --install build/x64 --config Release
```

## Use

Include `src/Aqsh.h` and link `Aqsh`. The API is C.

```c
AqShim_Init();

AqSynthParam param = {};
param.version = AQ_VER_10;
param.preset = AQ10_F1;
param.speed = 100;

unsigned char *wav = NULL;
int size = 0;
int rc = AqShim_Synthesize("ゆっくりしていってね", &param, &wav, &size);
if (rc == 0) {
    /* wav[0..size) is a WAV file */
    AqShim_FreeWav(wav);
}

AqShim_Shutdown();
```

### Parameters

```c
struct AqSynthParam {
    AqVersion version;

    // --- Aqtk1 + shared params ---
    // aq1: Aq1Preset; aq2: Aq2Preset; aq10: Aq10Preset.
    int preset;
    int speed;

    // aq2 custom phont path (UTF-8). Null or empty uses preset.
    char *phont;

    // --- Aqtk10 Extra Settings ---
    int base;   // 基本素片 F1E/F2E/M1E (0/1/2)
    int volume; // 音量 0-300 default:100
    int pitch;  // 高さ 20-200 default:基本素片に依存
    int accent; // アクセント 0-200 default:基本素片に依存
    int lmd;    // 音程１ 0-200 default:100
    int fsc;    // 音程２(サンプリング周波数) 50-200 default:100

    // --- Licensing ---
    char *DevKey;
    char *UserKey;
};
```

`version` is `AQ_VER_1`, `AQ_VER_2`, or `AQ_VER_10`.

`preset` is that engine's preset enum. 0 is `AQ1_F1` for AquesTalk1, `AQ2_AQ_YUKKURI` for AquesTalk2, and `AQ10_CUSTOM` for AquesTalk10.

`speed` is the speaking rate. All three engines read it. A non-zero AquesTalk10 preset uses the speed stored in that preset.

`phont` is AquesTalk2 only, a UTF-8 path. Null or empty uses the file for `preset` in `phont/` next to the library. A non-empty path overrides the preset.

`base`, `volume`, `pitch`, `accent`, `lmd`, and `fsc` apply only when the preset is `AQ10_CUSTOM`. `base` is the base fragment F1E/F2E/M1E (0/1/2). `volume` is 0–300, default 100. `pitch` is 20–200, default depends on the fragment. `accent` is 0–200, default depends on the fragment. `lmd` is 0–200, default 100. `fsc` is 50–200, default 100.

`DevKey` and `UserKey` are optional license keys. Null or empty leaves them unset.

The engine libraries must stay next to `Aqsh`. The loader opens them from that directory.

`AqShim_Synthesize` takes UTF-8 phonetic text. On success it returns 0 and a WAV buffer. Free that buffer with `AqShim_FreeWav`.

## Errors

`AqShim_Synthesize` returns 0 on success. `-1` is a null argument, `-2` is an unknown `version`, and `-3` means the voice or phont could not be opened. Any other non-zero value is the engine's error code, and it is negative.

`AqShim_ErrorMessage(version, code)` returns that failure as `AQ1_<NAME>`, `AQ2_<NAME>`, or `AQ10_<NAME>`. One engine uses one string for one failure, including when two engines number the same failure differently. An unknown code returns null. Pass the synthesize return value, or the positive code from the AquesTalk manual.
