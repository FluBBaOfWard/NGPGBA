#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Gui.h"
#include "Cart.h"
#include "cpu.h"
#include "Gfx.h"
#include "io.h"
#include "NGPHeader.h"

EWRAM_BSS int selectedGame = 0;
EWRAM_BSS ConfigData cfg;

//---------------------------------------------------------------------------------
int loadSettings() {
//	FILE *file;
/*
	if (findFolder(folderName)) {
		return 1;
	}
	if ( (file = fopen(settingName, "r")) ) {
		fread(&cfg, 1, sizeof(configdata), file);
		fclose(file);
		if (!strstr(cfg.magic,"cfg")) {
			infoOutput("Error in settings file.");
			return 1;
		}
	} else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
		return 1;
	}
*/
	gGammaValue = cfg.gammaValue;
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	sleepTime    = cfg.sleepTime;
	joyCfg       = (joyCfg&~0x400)|((cfg.controller&1)<<10);
//	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));

	infoOutput("Settings loaded.");
	return 0;
}
void saveSettings() {
//	FILE *file;

	strcpy(cfg.magic,"cfg");
	cfg.gammaValue  = gGammaValue;
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	cfg.sleepTime   = sleepTime;
	cfg.controller  = (joyCfg>>10)&1;
//	strlcpy(cfg.currentPath, currentDir, sizeof(currentDir));
/*
	if (findFolder(folderName)) {
		return;
	}
	if ( (file = fopen(settingName, "w")) ) {
		fwrite(&cfg, 1, sizeof(configdata), file);
		fclose(file);
		infoOutput("Settings saved.");
	} else {
		infoOutput("Couldn't open file:");
		infoOutput(settingName);
	}*/
	infoOutput("Settings saved.");
}

int loadNVRAM() {
	return 0;
}

void saveNVRAM() {
}

void loadState(void) {
//	unpackState(testState);
	infoOutput("Loaded state.");
}
void saveState(void) {
//	packState(testState);
	infoOutput("Saved state.");
}

/// Hold down the power button for ~40 frames.
static void turnPowerOff(void) {
	int i;
	if (g_BIOSBASE_COLOR != NULL) {
		EMUinput &= ~4;
		for (i = 0; i < 100; i++ ) {
			run();
			EMUinput |= 4;
			if (isConsoleSleeping()) {
				break;
			}
		}
		// Run a few more frames to turn off LED.
		for (i = 0; i < 10; i++) {
			run();
		}
		if (!isConsoleSleeping()) {
			machineInit();
		}
	}
}

/// Hold down the power button for ~40 frames.
static void turnPowerOn(void) {
	int i;
	if (g_BIOSBASE_COLOR != NULL) {
		EMUinput &= ~4;
		for (i = 0; i < 100; i++) {
			run();
			EMUinput |= 4;
			if (isConsoleRunning()) {
				break;
			}
		}
	}
}

//---------------------------------------------------------------------------------
bool loadGame(const RomHeader *rh) {
	if (rh) {
		selectedGame = selected;
		cls(0);
		if (isConsoleRunning()) {
			drawText("     Please wait, power off.", 9);
			turnPowerOff();
		}
		gRomSize = rh->filesize;
		romSpacePtr = (u8 *)rh + sizeof(RomHeader);
		tlcs9000MemInit(romSpacePtr);
		checkMachine();
		setEmuSpeed(0);
		loadCart(0);
		gameInserted = true;
		if (emuSettings & AUTOLOAD_NVRAM) {
			loadNVRAM();
		}
		if (emuSettings & AUTOLOAD_STATE) {
			loadState();
		}
		drawText("     Please wait, power on.", 9);
		turnPowerOn();
		closeMenu();
		return false;
	}
	return true;
}

void selectGame() {
	pauseEmulation = true;
	ui9();
	const RomHeader *rh = browseForFile();
	if (loadGame(rh)) {
		backOutOfMenu();
	}
}

//---------------------------------------------------------------------------------
static int loadBIOS(void *dest) {

	int i;
	const RomHeader *rh = NULL;
	for (i=0; i<3; i++) {
		rh = findRom(i);
		if ((rh->bios & 1) != 0 && (rh->filesize == 0x10000)) {
			memcpy(dest, (u8 *)rh + sizeof(RomHeader), 0x10000);
			return 1;
		}
	}
//	memcpy(dest, (u8 *)rawBios, 0x10000);
	return 0;
}

int loadColorBIOS(void) {
	if (loadBIOS(biosSpace)) {
		g_BIOSBASE_COLOR = biosSpace;
		return 1;
	}
	g_BIOSBASE_COLOR = NULL;
	return 0;
}

int loadBWBIOS(void) {
	if (loadBIOS(biosSpace)) {
		g_BIOSBASE_BW = biosSpace;
		return 1;
	}
	g_BIOSBASE_BW = NULL;
	return 0;
}

//---------------------------------------------------------------------------------
void checkMachine() {
	u8 newMachine = gMachineSet;
	if (newMachine == HW_AUTO) {
		if (ngpHeader->mode != 0) {
			newMachine = HW_NGPCOLOR;
		}
		else {
			newMachine = HW_NGPMONO;
		}
	}
	if (gMachine != newMachine) {
		gMachine = newMachine;
		if (gMachine == HW_NGPMONO) {
			gSOC = SOC_K1GE;
		}
		else {
			gSOC = SOC_K2GE;
		}
		machineInit();
	}
}
