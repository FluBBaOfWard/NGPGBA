#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Shared/AsmExtra.h"
#include "Gui.h"
#include "Cart.h"
#include "cpu.h"
#include "Gfx.h"
#include "io.h"
#include "Memory.h"
#include "NGPHeader.h"

EWRAM_BSS int selectedGame = 0;
EWRAM_BSS ConfigData cfg;

//---------------------------------------------------------------------------------
int initSettings() {
	cfg.gammaValue = 0;
	cfg.config = 0;
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM;
	cfg.sleepTime = 60*60*5;
	cfg.controller = 0;					// Don't swap A/B
	cfg.alarmHour = 0;
	cfg.alarmMinute = 0;
	cfg.birthDay = 0;
	cfg.birthMonth = 0;
	cfg.birthYear = 99;
	cfg.language = 1;
	gLang        = 1;
	cfg.palette = 0;	// Black n White
	gPaletteBank = 0;
	return 0;
}

bool updateSettingsFromNGP() {
	if (g_BIOSBASE_COLOR == NULL && g_BIOSBASE_BNW == NULL) {
		return false;
	}

	bool changed = false;
	int val;
	val = t9LoadBX(0x6F8B);
	if (cfg.birthYear != val) {
		cfg.birthYear = val;
		changed = true;
	}
	val = t9LoadBX(0x6F8C);
	if (cfg.birthMonth != val) {
		cfg.birthMonth = val;
		changed = true;
	}
	val = t9LoadBX(0x6F8D);
	if (cfg.birthDay != val) {
		cfg.birthDay = val;
		changed = true;
	}

	val = t9LoadBX(0x6C34);
	if (cfg.alarmHour != val) {
		cfg.alarmHour = val;
		changed = true;
	}
	val = t9LoadBX(0x6C35);
	if (cfg.alarmMinute != val) {
		cfg.alarmMinute = val;
		changed = true;
	}

	val = t9LoadBX(0x6F87) & 1;
	if (cfg.language != val) {
		cfg.language = val;
		gLang = val;
		changed = true;
	}
	if (gMachine == HW_NGPCOLOR) {
		val = t9LoadBX(0x6F94) & 7;
		if (cfg.palette != val) {
			cfg.palette = val;
			gPaletteBank = val;
			changed = true;
		}
	}
	settingsChanged |= changed;

	return changed;
}

int loadSettings() {
	bytecopy_((u8 *)&cfg, (u8 *)0x0E000000, sizeof(ConfigData));
	if (!strstr(cfg.magic,"cfg")) {
		infoOutput("Error in settings file.");
		return 1;
	}

	gGammaValue  = cfg.gammaValue;
	gLang        = cfg.language;
	gPaletteBank = cfg.palette;
	gConfig      = cfg.config;
	int mach     = gConfig & 3;
	if (mach == 3) mach = 0;
	gMachineSet  = mach;
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	sleepTime    = cfg.sleepTime;
	joyCfg       = (joyCfg&~0x400)|((cfg.controller&1)<<10);

	infoOutput("Settings loaded.");
	return 0;
}
void saveSettings() {

	strcpy(cfg.magic,"cfg");
	cfg.gammaValue  = gGammaValue;
	cfg.language    = gLang;
	cfg.palette     = gPaletteBank;
	cfg.config      = (cfg.config & ~3)|gMachineSet;
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	cfg.sleepTime   = sleepTime;
	cfg.controller  = (joyCfg>>10) & 1;

	bytecopy_((u8 *)0x0E000000, (u8 *)&cfg, sizeof(ConfigData));
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
		for (i = 0; i < 100; i++) {
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
	else {
		pauseEmulation = false;
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
		g_BIOSBASE_BNW = biosSpace;
		return 1;
	}
	g_BIOSBASE_BNW = NULL;
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
