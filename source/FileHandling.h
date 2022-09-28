#ifndef FILEHANDLING_HEADER
#define FILEHANDLING_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "Emubase.h"

#define FILEEXTENSIONS ".ngp.ngc"

extern ConfigData cfg;

int loadSettings(void);
void saveSettings(void);
bool loadGame(const RomHeader *rh);
void checkMachine(void);
int loadNVRAM(void);
void saveNVRAM(void);
void loadState(void);
void saveState(void);
void selectGame(void);
int loadColorBIOS(void);
int loadBWBIOS(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHANDLING_HEADER
