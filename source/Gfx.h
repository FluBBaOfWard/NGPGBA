#ifndef GFX_HEADER
#define GFX_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "K2GE/K2GE.h"

extern u8 gFlicker;
extern u8 gTwitch;
extern u8 gScaling;
extern u8 gGfxMask;

extern K2GE k2GE_0;
extern u16 EMUPALBUFF[0x200];
extern u16 GFX_BG0CNT;
extern u16 GFX_BG1CNT;

void gfxInit(void);
void vblIrqHandler(void);
void monoPalInit(void);
void paletteInit(u8 gammaVal);
void paletteTxAll(void);
void refreshGfx(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GFX_HEADER
