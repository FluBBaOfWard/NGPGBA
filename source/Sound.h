#ifndef SOUND_HEADER
#define SOUND_HEADER

#ifdef __cplusplus
extern "C" {
#endif

void soundInit(void);
void soundSetFrequency(void);
void setMuteSoundGUI(void);
void vblSound1(void);
void vblSound2(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SOUND_HEADER
