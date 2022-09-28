#ifndef CART_HEADER
#define CART_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 gRomSize;
extern u32 maxRomSize;
extern u32 emuFlags;
extern u8 gConfig;
extern u8 gMachineSet;
extern u8 gMachine;
extern u8 gSOC;
extern u8 gLang;
extern u8 gPaletteBank;

extern u8 ngpRAM[0x4000];
extern u8 biosSpace[0x10000];
extern u8 *romSpacePtr;
extern void *g_BIOSBASE_COLOR;
extern void *g_BIOSBASE_BW;

void machineInit(void);
void loadCart(int emuFlags);
void tlcs9000MemInit(u8 *romBase);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CART_HEADER
