#ifndef SOUND_HEADER
#define SOUND_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "SN76496/SN76496.h"

extern SN76496 k2Audio_0;
extern u8 soundMode;

void soundInit(void);
void soundSetFrequency(void);
void soundSetMuteGUI(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !SOUND_HEADER
