# aqtk-shim

AquesTalk 1、2、10 の互換レイヤ。C API `Aqsh` が、選んだエンジンのライブラリを読み込む。

[English](README.md) · [中文](README.zh.md) · **<u>日本語</u>**

## 対応状況

| エンジン | Windows | macOS | Linux | Android | iOS |
|----------|---------|-------|-------|---------|-----|
| AquesTalk1 | ✅ | ⚠️ | ⚠️ | ❌ | ❌ |
| AquesTalk2 | ✅ | ⚠️ | ⚠️ | ❌ | ❌ |
| AquesTalk10 | ✅ | ⚠️ | ⚠️ | ❌ | ❌ |

## 依存関係

CMake 3.20 以上と、C++17 のコンパイラ。

- Windows: Visual Studio 2022（MSVC）。インポートライブラリは MSVC でしかリンクできない。
- macOS: arm64 のみ。
- Linux: x86_64 または i386。32 ビットビルドには multilib（`-m32`）が必要。

[AquesTalk 公式サイト](https://www.a-quest.com/download.html)から、対象プラットフォームの AquesTalk SDK をすべてダウンロードする。

AquesTalk SDK を `third-party/` に置く。必要なライブラリが無いと configure が失敗する。AquesTalk2 の phont が欠けている場合、そのファイルだけ飛ばす。

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

`<lib>` は 64 ビットが `lib64`、32 ビットが `lib`。Linux では AquesTalk1 と AquesTalk10 が `lib64` / `lib32`、AquesTalk2 が `lib64` / `lib`。

`<voice>` は `f1`、`f2`、`f3`、`m1`、`m2`、`r1`、`dvd`、`imd1`、`jgr`。9 つとも必要。ビルドするプラットフォームのツリーだけ置けばよい。

## ビルド

Windows x64:

```
cmake --preset x64
cmake --build --preset x64-release
```

他の configure プリセットは `x86`、`macos`、`linux`、`linux32`。対応する build プリセットは `CMakePresets.json` にある。

ビルドすると、エンジンのライブラリと `phont/` が `Aqsh` の横にコピーされる。`aqtk_demo` は同じディレクトリに `aq1.wav`、`aq2.wav`、`aq10.wav` を書く。

```
build/x64/Release/aqtk_demo.exe
build/macos/aqtk_demo
build/linux/aqtk_demo
```

`AQTK_STAGE_PHONTS=OFF` にすると `phont/` をコピーしない。その場合は phont のパスか、メモリ上の phont データを自分で渡す。

インストール先の既定は `../third-party/<arch>`（`win64`、`win32`、`macos`、`linux64`、`linux32`）。`Aqsh`、リネームしたエンジンライブラリ、`phont/`、`Aqsh.h` が入る。

```
cmake --install build/x64 --config Release
```

## 使い方

`src/Aqsh.h` をインクルードし、`Aqsh` をリンクする。API は C。

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
    /* wav[0..size) は WAV ファイル */
    AqShim_FreeWav(wav);
}

AqShim_Shutdown();
```

### パラメータ

```c
struct AqSynthParam {
    AqVersion version;

    // --- Aqtk1 + shared params ---
    // aq1: Aq1Preset; aq2: Aq2Preset; aq10: Aq10Preset.
    int preset;
    int speed;

    // aq2 custom phont path (UTF-8). Null or empty uses preset.
    // phont_data overrides this path.
    char *phont;
    // aq2 phont file contents already in memory. Non-null overrides phont and preset.
    // The buffer stays valid for the AqShim_Synthesize call.
    unsigned char *phont_data;

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

`version` は `AQ_VER_1`、`AQ_VER_2`、`AQ_VER_10`。

`preset` はそのエンジンのプリセット列挙。0 は AquesTalk1 が `AQ1_F1`、AquesTalk2 が `AQ2_AQ_YUKKURI`、AquesTalk10 が `AQ10_CUSTOM`。

`speed` は話速。3 つのエンジンともこの値を読む。AquesTalk10 で 0 以外のプリセットを使うと、話速はプリセット側の値になる。

`phont` は AquesTalk2 のみ。UTF-8 のパス。ヌルまたは空文字のときは、ライブラリ横の `phont/` にある `preset` のファイルを使う。パスが空でなければプリセットより優先される。

`phont_data` は AquesTalk2 のみ。ヌルでなければ、メモリ上の phont ファイル本体。`phont` とプリセットより優先される。長さは AquesTalk2 が phont 自身から読む。バッファは `AqShim_Synthesize` が戻るまで有効であること。

`base`、`volume`、`pitch`、`accent`、`lmd`、`fsc` は `AQ10_CUSTOM` のときだけ有効。`base` は基本素片 F1E/F2E/M1E（0/1/2）。`volume` は 0–300、既定 100。`pitch` は 20–200、既定は素片による。`accent` は 0–200、既定は素片による。`lmd` は 0–200、既定 100。`fsc` は 50–200、既定 100。

`DevKey` と `UserKey` は任意のライセンスキー。ヌルまたは空文字なら設定しない。

エンジンのライブラリは `Aqsh` と同じディレクトリに置く。ローダはそこから開く。

`AqShim_Synthesize` は UTF-8 の音声記号列を受け取る。成功すると 0 と WAV バッファを返す。バッファは `AqShim_FreeWav` で解放する。

## エラー

`AqShim_Synthesize` は成功で 0 を返す。`-1` は引数がヌル、`-2` は未知の `version`、`-3` は音声ライブラリまたは phont を開けなかった場合。それ以外の非 0 はエンジンのエラーコードで、負の値。

`AqShim_ErrorMessage(version, code)` はその失敗を `AQ1_<NAME>`、`AQ2_<NAME>`、`AQ10_<NAME>` の文字列で返す。同じエンジンでは、同じ失敗は同じ文字列になる。エンジン間で番号が違っても、意味が同じなら語幹は同じ。未知のコードは null。合成の戻り値か、AquesTalk のマニュアルにある正のエラーコードを渡す。
