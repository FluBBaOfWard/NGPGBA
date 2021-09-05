#ifndef MEMORY_HEADER
#define MEMORY_HEADER

#ifdef __cplusplus
extern "C" {
#endif

u8  t9LoadB(u32 address);
u16 t9LoadW(u32 address);
u32 t9LoadL(u32 address);

void t9StoreB(u8 data, u32 address);
void t9StoreW(u16 data, u32 address);
void t9StoreL(u32 data, u32 address);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	// MEMORY_HEADER
