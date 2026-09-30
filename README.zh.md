# aqtk-shim

AquesTalk 1、2、10 的兼容层。一套 C 接口 `Aqsh`，按所选引擎加载对应的库。

[English](README.md) · [中文](README.zh.md)

## 依赖

CMake 3.20 或更高，以及支持 C++17 的编译器。

- Windows：Visual Studio 2022（MSVC）。导入库必须用 MSVC 链接。
- macOS：仅 arm64。
- Linux：x86_64 或 i386。32 位构建需要 multilib 工具链（`-m32`）。

把 AquesTalk SDK 放在 `third-party/`。缺少必需的库时，配置会失败。缺少某个 AquesTalk2 phont 时，会跳过该文件。

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

`<lib>`：64 位为 `lib64`，32 位为 `lib`。Linux 上 AquesTalk1 和 AquesTalk10 用 `lib64` / `lib32`，AquesTalk2 用 `lib64` / `lib`。

`<voice>` 为 `f1`、`f2`、`f3`、`m1`、`m2`、`r1`、`dvd`、`imd1`、`jgr`，只需放置要编译的平台的目录。

## 构建

Windows x64：

```sh
cmake --preset x64
cmake --build --preset x64-release
```

其他配置预设：`x86`、`macos`、`linux`、`linux32`。对应的构建预设在 `CMakePresets.json`。

构建时会把引擎库和 `phont/` 复制到 `Aqsh` 旁边。`aqtk_demo` 会在自身所在目录写出 `aq1.wav`、`aq2.wav`、`aq10.wav`：

```sh
build/x64/Release/aqtk_demo.exe
build/macos/aqtk_demo
build/linux/aqtk_demo
```

`AQTK_STAGE_PHONTS=OFF` 时不复制 `phont/`，此时需要自行传入 phont 路径。

安装会把 `Aqsh`、重命名后的引擎库、`phont/` 和 `Aqsh.h` 放到 `../third-party/<arch>`（`win64`、`win32`、`macos`、`linux64`、`linux32`）：

```sh
cmake --install build/x64 --config Release
```

## 使用

包含 `src/Aqsh.h` 并链接 `Aqsh`。接口是 C。

```c++
AqShim_Init();

AqSynthParam param = {};
param.version = AQ_VER_10;
param.preset = AQ10_F1;
param.speed = 100;

unsigned char *wav = NULL;
int size = 0;
int rc = AqShim_Synthesize("ゆっくりしていってね", &param, &wav, &size);
if (rc == 0) {
    /* wav[0..size) 是 WAV 文件 */
    AqShim_FreeWav(wav);
}

AqShim_Shutdown();
```

### 可配置参数

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

`version` 是 `AQ_VER_1`、`AQ_VER_2` 或 `AQ_VER_10`。

`preset` 是该引擎的预设枚举。AquesTalk1 的 0 是 `AQ1_F1`，AquesTalk2 的 0 是 `AQ2_AQ_YUKKURI`，AquesTalk10 的 0 是 `AQ10_CUSTOM`。

`speed` 是语速，三个引擎都读这个字段。AquesTalk10 使用非 0 预设时，语速来自预设。

`phont` 只用于 AquesTalk2，内容是 UTF-8 路径。空指针或空字符串时使用 `preset` 对应的文件，文件在库旁边的 `phont/` 里。路径非空时覆盖预设。

`base`、`volume`、`pitch`、`accent`、`lmd`、`fsc` 只在 `AQ10_CUSTOM` 时生效。`base` 是基本素片 F1E/F2E/M1E（0/1/2）。`volume` 为 0–300，默认 100。`pitch` 为 20–200，默认随素片。`accent` 为 0–200，默认随素片。`lmd` 为 0–200，默认 100。`fsc` 为 50–200，默认 100。

`DevKey` 和 `UserKey` 是可选的许可密钥。空指针或空字符串时不设置。

引擎库必须和 `Aqsh` 放在同一目录，加载器从那里打开它们。

`AqShim_Synthesize` 接收 UTF-8 语音符号文本。成功时返回 0 和一块 WAV 缓冲，用 `AqShim_FreeWav` 释放。

## 错误

`AqShim_Synthesize` 成功时返回 0。`-1` 表示参数为空，`-2` 表示未知的 `version`，`-3` 表示打不开音色或 phont。其他非 0 值是引擎错误码，为负数。

`AqShim_ErrorMessage(version, code)` 可以把引擎错误码转成字符串，格式为 `AQ1_<NAME>`、`AQ2_<NAME>` 或 `AQ10_<NAME>`。同一引擎上，同一种失败对应同一个字符串，即使两个引擎给它的编号不同。未知错误码返回 null。传入合成函数的返回值，或 AquesTalk 手册中的正数错误码，都可以。
