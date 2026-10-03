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

// AquesTalk1 voice libraries. 0 is f1 so a zeroed AqSynthParam keeps that voice.
enum Aq1Preset {
    AQ1_F1 = 0,
    AQ1_F2,
    AQ1_F3,
    AQ1_M1,
    AQ1_M2,
    AQ1_R1,
    AQ1_DVD,
    AQ1_IMD1,
    AQ1_JGR,
    AQ1_PRESET_COUNT
};

// AquesTalk2 phont files staged in phont/ next to Aqsh.dll. 0 is aq_yukkuri.
// A non-empty phont path overrides this preset. phont_data overrides both.
enum Aq2Preset {
    AQ2_AQ_YUKKURI = 0,
    AQ2_AQ_DEFO1,
    AQ2_AQ_F1C,
    AQ2_AQ_F3A,
    AQ2_AQ_HUSKEY,
    AQ2_AQ_M4B,
    AQ2_AQ_MF1,
    AQ2_AQ_RB2,
    AQ2_AQ_RB3,
    AQ2_AQ_RM,
    AQ2_AQ_ROBO,
    AQ2_AQ_TETO1,
    AQ2_AR_F4,
    AQ2_AR_M5,
    AQ2_AR_MF2,
    AQ2_AR_RM3,
    AQ2_PRESET_COUNT
};

// AquesTalk10 gVoice_* presets. 0 uses base/speed/volume/pitch/accent/lmd/fsc.
enum Aq10Preset {
    AQ10_CUSTOM = 0,
    AQ10_F1,
    AQ10_F2,
    AQ10_F3,
    AQ10_M1,
    AQ10_M2,
    AQ10_R1,
    AQ10_R2,
    AQ10_PRESET_COUNT
};

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

// Token for an aq1/aq2/aq10 error, prefixed AQ1_, AQ2_, or AQ10_.
// See aq_error.cpp for the list of error codes.
AQSH_API const char* AqShim_ErrorMessage(AqVersion version, int code);
#ifdef __cplusplus
}
#endif

#endif // AQTK_SHIM_AQSH_H
