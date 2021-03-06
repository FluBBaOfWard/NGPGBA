#ifndef CART_HEADER
#define CART_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 g_romSize;
extern u32 maxRomSize;
extern u32 emuFlags;
extern u8 g_cartFlags;
extern u8 g_configSet;
extern u8 g_config;
extern u8 gMachine;
extern u8 gLang;
extern u8 gPaletteBank;

extern u8 biosSpace[0x10000];
extern void *romSpacePtr;
extern void *g_BIOSBASE_COLOR;
extern void *g_BIOSBASE_BW;

void machineInit(void);
void loadCart(int emuFlags);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CART_HEADER
