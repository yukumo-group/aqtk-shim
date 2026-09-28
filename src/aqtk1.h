#pragma once

struct AqSynthParam;

unsigned char* aqtk1_synthe_utf8(const AqSynthParam* param, const char* text, int* size);
void aqtk1_free(unsigned char* wav);
