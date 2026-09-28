#ifndef AQTK_SHIM_AQSH_H
#define AQTK_SHIM_AQSH_H

#ifdef _WIN32
    #ifdef AQSWRAPPER_EXPORTS
        #define AQSH_API __declspec(dllexport)
    #else
        #define AQSH_API __declspec(dllimport)
    #endif
#else
    // Non-Windows OS symbols visibility
    #define AQSH_API __attribute__((visibility("default")))
#endif

// use C Name Mangling for external calls (like CGO)
#ifdef __cplusplus
extern "C" {
#endif

enum AqVersion {
    AQ_VER_1 = 1,
    AQ_VER_2 = 2,
    AQ_VER_10 = 10
};

struct AqSynthParam {
    AqVersion version;

    // --- Aqtk1 + shared params ---
    // aq1/aq10: presets; aq2: phont path / name;
    int preset;
    int speed;

    // --- Aqtk10 Extra Settings ---
    int base;	// 基本素片 F1E/F2E/M1E (0/1/2)
    int volume;	// 音量 	0-300 default:100
    int pitch;	// 高さ 	20-200 default:基本素片に依存
    int accent;	// アクセント 0-200 default:基本素片に依存
    int lmd;	// 音程１ 	0-200 default:100
    int fsc;	// 音程２(サンプリング周波数) 50-200 default:100

    // --- Licensing ---
    char *DevKey;
    char *UserKey;
};

AQSH_API int AqShim_Init(void);
AQSH_API void AqShim_Shutdown(void);
AQSH_API int AqShim_Synthesize(
    const char* text,
    const AqSynthParam* param,
    unsigned char** out_wav_data,
    int* out_wav_size
);
AQSH_API void AqShim_FreeWav(unsigned char* wav_data);
#ifdef __cplusplus
}
#endif

#endif // AQTK_SHIM_AQSH_H