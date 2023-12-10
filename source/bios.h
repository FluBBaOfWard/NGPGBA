
#ifndef __BIOS__
#define __BIOS__

#include "NGPHeader.h"

/** Fill the bios rom area with a HLE BIOS. Call once at program start */
bool installHleBios(u8 *ngpBios);

void resetHleBios(NgpHeader *cartHeader);

void fixBiosSettings(void);

#endif
