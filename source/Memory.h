#ifndef MEMORY_HEADER
#define MEMORY_HEADER

#ifdef __cplusplus
extern "C" {
#endif

u8  t9LoadBX(u32 address);
u16 t9LoadWX(u32 address);
u32 t9LoadLX(u32 address);

void t9StoreBX(u8 data, u32 address);
void t9StoreWX(u16 data, u32 address);
void t9StoreLX(u32 data, u32 address);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// MEMORY_HEADER
