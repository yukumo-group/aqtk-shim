#pragma once

#include "synth_util.h"

using WavFreeFn = void (AQTK_CALL*)(unsigned char* wav);

void wav_track(unsigned char* wav, WavFreeFn free_fn);
void wav_release(unsigned char* wav);
void wav_release_all();
