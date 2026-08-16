#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Shared/SRAMHandler.h"
#include "Shared/AsmExtra.h"
#include "Gui.h"
#include "Cart.h"
#include "cpu.h"
#include "Gfx.h"
#include "io.h"
#include "bios.h"
#include "Memory.h"
#include "NGPHeader.h"

/// Used for emulators or flashcarts to choose save type.
const char *const sramTag = "SRAM_Vnnn";

EWRAM_BSS int selectedGame = 0;
EWRAM_BSS ConfigData cfg;

//---------------------------------------------------------------------------------
void applyConfigData(void) {
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	gGammaValue  = cfg.gammaValue;
	gLang        = cfg.language;
	gPaletteBank = cfg.palette;
	gConfig      = cfg.config;
	int mach     = gConfig & 3;
	if (mach == 3) mach = 0;
	gMachineSet  = mach;
	joyCfg       = (joyCfg & ~0x400) | ((cfg.controller & 1) << 10);
	setSleepValue(emuSettings & AUTOSLEEP_MASK);
}

void updateConfigData(void) {
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	cfg.gammaValue  = gGammaValue;
	cfg.language    = gLang;
	cfg.palette     = gPaletteBank;
	cfg.config      = (cfg.config & ~3)|gMachineSet;
	cfg.controller  = (joyCfg >> 10) & 1;
}

void initSettings(void) {
	memset(&cfg, 0, sizeof(ConfigData));
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | AUTOSLEEP_OFF | ALLOW_SPEED_HACKS;
	cfg.birthYear = 99;
	cfg.language = 1;

	applyConfigData();
}

bool updateSettingsFromNGP() {
	if (isRealBios == 0) {
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
	if (readFile((u8 *)&cfg, sizeof(cfg), NGPID)) {
		applyConfigData();
		settingsChanged = false;
		infoOutput("Settings loaded.");
		installBIOS(biosSpace);
		return 0;
	}
	else {
		updateConfigData();
		infoOutput("No settings file found.");
	}
	return 1;
}
void saveSettings() {
	updateConfigData();

	if (writeFile((u8 *)&cfg, sizeof(cfg), NGPID, "Config")) {
		settingsChanged = false;
		infoOutput("Settings saved.");
	}
	else {
		infoOutput("Could not save settings file.");
	}
	installBIOS(biosSpace);
}

int loadNVRAM() {
	return 0;
}

void saveNVRAM() {
}

int loadStateChk() {
	if (getStateSize() < 0x10000
		&& quickLoad()) {
		infoOutput("Loaded State.");
		return loadNVRAM();
	}
	return 0;
}
int saveStateChk() {
	if (getStateSize() < 0x10000
		&& quickSave()) {
		infoOutput("Saved State.");
		return 1;
	}
	return 0;
}

void loadState(void) {
	loadStateChk();
	installBIOS(biosSpace);
}
void saveState(void) {
	saveStateChk();
	installBIOS(biosSpace);
}

/// Hold down the power button for ~40 frames.
static void turnPowerOff(void) {
	int i;
	if (isRealBios != 0) {
		EMUinput = 0;
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
	if (isRealBios != 0) {
		EMUinput = 0;
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
		return loadROM(rh->romData, rh->filesize);
	}
	return true;
}

//---------------------------------------------------------------------------------
bool loadROM(const u8 *rom, int size) {
	selectedGame = selected;
	cls(0);
	if (isConsoleRunning()) {
		drawText("     Please wait, power off.", 9);
		turnPowerOff();
	}
	gRomSize = size;
	romSpacePtr = rom;
	tlcs9000MemInit(romSpacePtr);
	checkMachine();
	setEmuSpeed(0);
	loadCart();
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

void viewSStates() {
	pauseEmulation = true;
	ui13();
	skipScroll();
	loadStateMenu();
	backOutOfMenu();
	installBIOS(biosSpace);
}

//---------------------------------------------------------------------------------
void ejectCart() {
	gRomSize = 0x200000;
	romSpacePtr = (u8 *)ejectCart;
	tlcs9000MemInit(romSpacePtr);
	gameInserted = false;
}

//---------------------------------------------------------------------------------
void installBIOS(void *dest) {
	const void *src = NULL;
	if (g_BIOSBASE_COLOR != NULL && gMachine == HW_NGPCOLOR) {
		src = g_BIOSBASE_COLOR;
	}
	else if (g_BIOSBASE_BNW != NULL && gMachine == HW_NGPMONO) {
		src = g_BIOSBASE_BNW;
	}
	else {
		isRealBios = 0;
		installHleBios(dest);
	}
	if (src != NULL) {
		isRealBios = 1;
		memcpy(dest, src, 0x10000);
		if (gMachine == HW_NGPCOLOR) {
			patchColorBios(dest);
		}
	}
}

void loadBioses(void) {

	int i;
	for (i=0; i<3; i++) {
		const RomHeader *rh = findBios(i);
		if (rh != NULL && (rh->filesize == 0x10000)) {
			if (rh->flags & 0x04) {
				g_BIOSBASE_COLOR = rh->romData;
			}
			else {
				g_BIOSBASE_BNW = rh->romData;
			}
		}
	}
	installBIOS(biosSpace);
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
		installBIOS(biosSpace);
		machineInit();
	}
}
