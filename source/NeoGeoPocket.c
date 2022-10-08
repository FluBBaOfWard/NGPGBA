#include <gba.h>
#include <string.h>

#include "NeoGeoPocket.h"
#include "Cart.h"
#include "Gfx.h"
#include "Sound.h"
#include "io.h"
#include "TLCS900H/TLCS900H.h"
#include "ARMZ80/ARMZ80.h"


int packState(void *statePtr) {
	int size = 0;
	memcpy(statePtr+size, ngpRAM, sizeof(ngpRAM));
	size += sizeof(ngpRAM);
//	size += ioSaveState(statePtr+size);
	size += t6W28SaveState(statePtr+size, &k2Audio_0);
	size += k2GESaveState(statePtr+size, &k2GE_0);
	size += Z80SaveState(statePtr+size, &Z80OpTable);
	size += tlcs900HSaveState(statePtr+size, &tlcs900HState);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	memcpy(ngpRAM, statePtr+size, sizeof(ngpRAM));
	size += sizeof(ngpRAM);
//	size += ioLoadState(statePtr+size);
	size += t6W28LoadState(&k2Audio_0, statePtr+size);
	size += k2GELoadState(&k2GE_0, statePtr+size);
	size += Z80LoadState(&Z80OpTable, statePtr+size);
	tlcs900HLoadState(&tlcs900HState, statePtr+size);
}

int getStateSize() {
	int size = 0;
	size += sizeof(ngpRAM);
//	size += ioGetStateSize();
	size += t6W28GetStateSize();
	size += k2GEGetStateSize();
	size += Z80GetStateSize();
	size += tlcs900HGetStateSize();
	return size;
}
