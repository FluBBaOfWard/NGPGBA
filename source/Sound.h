#ifndef SOUND_HEADER
#define SOUND_HEADER

#ifdef __cplusplus
extern "C" {
#endif

//#include "t6w28.i"

//extern SN76496 k2Audio_0;

void soundInit(void);
void soundSetFrequency(void);
void setMuteSoundGUI(void);
void vblSound1(void);
void vblSound2(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOUND_HEADER
