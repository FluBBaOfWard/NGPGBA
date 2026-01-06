#include <gba.h>
#include <string.h>

#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Main.h"
#include "FileHandling.h"
#include "Cart.h"
#include "Gfx.h"
#include "Sound.h"
#include "io.h"
#include "cpu.h"
#include "bios.h"
#include "TLCS900H/Version.h"
#include "ARMZ80/Version.h"
#include "K2GE/Version.h"

#define EMUVERSION "V0.5.8 2026-01-06"

#define HALF_CPU_SPEED		(1<<16)
#define ALLOW_SPEED_HACKS	(1<<17)

void hacksInit(void);

static void gammaChange(void);
static void paletteChange(void);
static const char *getPaletteText(void);
static void languageSet(void);
static const char *getLanguageText(void);
static void machineSet(void);
static const char *getMachineText(void);
static void batteryChange(void);
static void subBatteryChange(void);
static void speedHackSet(void);
static const char *getSpeedHackText(void);
static void cpuHalfSet(void);
static const char *getCPUHalfText(void);
static void z80SpeedSet(void);
static const char *getZ80SpeedText(void);
static void soundSet(void);
static const char *getSoundText(void);
static void stepFrameUI(void);
static void swapABSet(void);
static const char *getSwapABText(void);
static void fgrLayerSet(void);
static const char *getFgrLayerText(void);
static void bgrLayerSet(void);
static const char *getBgrLayerText(void);
static void sprLayerSet(void);
static const char *getSprLayerText(void);

static void updateGameInfo(void);

const MItem dummyItems[] = {
	{"", uiDummy},
};
const MItem mainItems[] = {
	{"File->", ui2},
	{"Controller->", ui3},
	{"Display->", ui4},
	{"Machine->", ui5},
	{"Settings->", ui6},
	{"Debug->", ui7},
	{"About->", ui8},
	{"Sleep", gbaSleep},
	{"Reset Console", resetConsole},
	{"Quit Emulator", ui10},
};
const MItem fileItems[] = {
	{"Load Game->", selectGame},
	{"Load State", loadState},
	{"Save State", saveState},
	{"Save Settings", saveSettings},
	{"Reset Game", resetConsole},
};
const MItem ctrlItems[] = {
	{"B Autofire: ", autoBSet, getAutoBText},
	{"A Autofire: ", autoASet, getAutoAText},
	{"Swap A-B:   ", swapABSet, getSwapABText},
};
const MItem displayItems[] = {
	{"Gamma: ", gammaChange, getGammaText},
	{"B&W Palette: ", paletteChange, getPaletteText},
};
const MItem machineItems[] = {
	{"Language: ", languageSet, getLanguageText},
	{"Machine: ", machineSet, getMachineText},
	{"Change Batteries", batteryChange},
	{"Change Sub Battery", subBatteryChange},
	{"Cpu Speed Hacks: ", speedHackSet, getSpeedHackText},
	{"Half Cpu Speed: ", cpuHalfSet, getCPUHalfText},
	{"Z80 Clock: ", z80SpeedSet, getZ80SpeedText},
	{"Sound: ", soundSet, getSoundText},
};
const MItem setItems[] = {
	{"Speed: ", speedSet, getSpeedText},
	{"Autoload State: ", autoStateSet, getAutoStateText},
	{"Autosave Settings: ", autoSettingsSet, getAutoSettingsText},
	{"Autopause Game: ", autoPauseGameSet, getAutoPauseGameText},
	{"EWRAM Overclock: ", ewramSet, getEWRAMText},
	{"Autosleep: ", sleepSet, getSleepText},
};
const MItem debugItems[] = {
	{"Debug Output: ", debugTextSet, getDebugText},
	{"Disable Foreground: ", fgrLayerSet, getFgrLayerText},
	{"Disable Background: ", bgrLayerSet, getBgrLayerText},
	{"Disable Sprites: ", sprLayerSet, getSprLayerText},
	{"Step Frame", stepFrameUI},
};
const MItem fnList9[] = {
	{"", quickSelectGame},
};
const MItem quitItems[] = {
	{"Yes", exitEmulator},
	{"No", backOutOfMenu},
};

const Menu menu0 = MENU_M("", uiNullNormal, dummyItems);
Menu menu1 = MENU_M("Main Menu", uiAuto, mainItems);
const Menu menu2 = MENU_M("File Handling", uiAuto, fileItems);
const Menu menu3 = MENU_M("Controller Settings", uiAuto, ctrlItems);
const Menu menu4 = MENU_M("Display Settings", uiAuto, displayItems);
const Menu menu5 = MENU_M("Machine Settings", uiAuto, machineItems);
const Menu menu6 = MENU_M("Other Settings", uiAuto, setItems);
const Menu menu7 = MENU_M("Debug", uiAuto, debugItems);
const Menu menu8 = MENU_M("About", uiAbout, dummyItems);
const Menu menu9 = MENU_M("Load game", uiLoadGame, fnList9);
const Menu menu10 = MENU_M("Quit Emulator?", uiAuto, quitItems);

const Menu *const menus[] = {&menu0, &menu1, &menu2, &menu3, &menu4, &menu5, &menu6, &menu7, &menu8, &menu9, &menu10 };

EWRAM_BSS u8 gZ80Speed = 0;
EWRAM_BSS char gameInfoString[32];

const char *const machTxt[]  = {"Auto", "NeoGeo Pocket", "NeoGeo Pocket Color"};
const char *const bordTxt[]  = {"Black", "Border Color", "None"};
const char *const palTxt[]   = {"Black & White", "Red", "Green", "Blue", "Classic"};
const char *const langTxt[]  = {"Japanese", "English"};
const char *const cpuSpeedTxt[]  = {"Full Speed", "Half Speed", "1/4 Speed", "1/8 Speed", "1/16 Speed"};

/// This is called at the start of the emulator
void setupGUI() {
	emuSettings = AUTOPAUSE_EMULATION | ALLOW_SPEED_HACKS;
//	keysSetRepeat(25, 4);	// Delay, repeat.
	menu1.itemCount = ARRSIZE(mainItems) - (enableExit?0:1);
	closeMenu();
}

/// This is called when going from emu to ui.
void enterGUI() {
}

/// This is called going from ui to emu.
void exitGUI() {
	setupBorderPalette();
}

void quickSelectGame() {
	openMenu();
	selectGame();
	closeMenu();
}

void uiNullNormal() {
	uiNullDefault();
}

void uiAbout() {
	setupSubMenuText();
	updateGameInfo();
	drawText("B:        NGP A Button", 3);
	drawText("A:        NGP B Button", 4);
	drawText("L:        NGP D Button", 5);
	drawText("Start:    Option Button", 6);
	drawText("Select:   Power Button", 7);
	drawText("DPad:     Joystick", 8);

	drawText(gameInfoString, 10);

	drawText("NGPGBA      " EMUVERSION, 15);
	drawText("ARMZ80      " ARMZ80VERSION, 16);
	drawText("ARMTLCS900H " TLCS900VERSION, 17);
	drawText("ARMK2GE     " K2GEVERSION, 18);
	drawText("ARMK2Audio  " , 19);
}

void uiLoadGame() {
	setupSubMenuText();
}

void nullUINormal(int key) {
}

void nullUIDebug(int key) {
}

void resetConsole() {
	checkMachine();
	machineInit();
	loadCart(0);
}

void updateGameInfo() {
	char catalog[8];
	strlMerge(gameInfoString, "Game Name: ", ngpHeader->name, sizeof(gameInfoString));
	strlcat(gameInfoString, " #", sizeof(gameInfoString));
	short2HexStr(catalog, ngpHeader->catalog);
	strlcat(gameInfoString, catalog, sizeof(gameInfoString));
}
//---------------------------------------------------------------------------------
void debugIO(u16 port, u8 val, const char *message) {
	char debugString[32];

	debugString[0] = 0;
	strlcat(debugString, message, sizeof(debugString));
	short2HexStr(&debugString[strlen(debugString)], port);
	strlcat(debugString, " val:", sizeof(debugString));
	char2HexStr(&debugString[strlen(debugString)], val);
	debugOutput(debugString);
}
//---------------------------------------------------------------------------------
void debugIOUnimplR(u16 port) {
	debugIO(port, 0, "Unimpl R port:");
}
void debugIOUnimplW(u8 val, u16 port) {
	debugIO(port, val, "Unimpl W port:");
}
void debugDivideError() {
	debugOutput("Divide Error.");
}
void debugUndefinedInstruction() {
	debugOutput("Undefined Instruction.");
}
void debugCrashInstruction() {
	debugOutput("CPU Crash!");
}

void stepFrameUI() {
	stepFrame();
	setupMenuPalette();
}
//---------------------------------------------------------------------------------
/// Swap A & B buttons
void swapABSet() {
	joyCfg ^= 0x400;
}
const char *getSwapABText() {
	return autoTxt[(joyCfg>>10)&1];
}

/// Turn on/off scaling
void scalingSet(){
	gScaling ^= 0x01;
	refreshGfx();
}

/// Change gamma (brightness)
void gammaChange() {
	gammaSet();
	paletteInit(gGammaValue);
	paletteTxAll();					// Make new palette visible
	setupMenuPalette();
}

/// Turn on/off rendering of foreground
void fgrLayerSet() {
	gGfxMask ^= 0x01;
}
const char *getFgrLayerText() {
	return autoTxt[gGfxMask&1];
}
/// Turn on/off rendering of background
void bgrLayerSet() {
	gGfxMask ^= 0x02;
}
const char *getBgrLayerText() {
	return autoTxt[(gGfxMask>>1)&1];
}
/// Turn on/off rendering of sprites
void sprLayerSet() {
	gGfxMask ^= 0x10;
}
const char *getSprLayerText() {
	return autoTxt[(gGfxMask>>4)&1];
}

void paletteChange() {
	gPaletteBank++;
	if (gPaletteBank > 4) {
		gPaletteBank = 0;
	}
	settingsChanged = true;
	monoPalInit();
	paletteTxAll();
	fixBiosSettings();
}
const char *getPaletteText() {
	return palTxt[gPaletteBank];
}
/*
void borderSet() {
	bcolor++;
	if (bcolor > 2) {
		bcolor = 0;
	}
	makeborder();
}
*/
void languageSet() {
	gLang ^= 0x01;
	fixBiosSettings();
}
const char *getLanguageText() {
	return langTxt[gLang];
}

void machineSet() {
	gMachineSet++;
	if (gMachineSet >= HW_SELECT_END) {
		gMachineSet = 0;
	}
}
const char *getMachineText() {
	return machTxt[gMachineSet];
}

void batteryChange() {
	batteryLevel = 0xFFFF;				// 0xFFFF for 2 days battery?
}

void subBatteryChange() {
	gSubBatteryLevel = 0x3FFFFFF;		// 0x3FFFFFF for 2 years battery?
}

void speedHackSet() {
	emuSettings ^= ALLOW_SPEED_HACKS;
	emuSettings &= ~HALF_CPU_SPEED;
	hacksInit();
	tweakCpuSpeed(emuSettings & HALF_CPU_SPEED);
}
const char *getSpeedHackText() {
	return autoTxt[(emuSettings & ALLOW_SPEED_HACKS)>>17];
}

void cpuHalfSet() {
	emuSettings ^= HALF_CPU_SPEED;
	emuSettings &= ~ALLOW_SPEED_HACKS;
	tweakCpuSpeed(emuSettings & HALF_CPU_SPEED);
	if ((emuSettings & HALF_CPU_SPEED) && gZ80Speed < 1) {
		gZ80Speed = 1;
	}
	tweakZ80Speed(gZ80Speed);
}
const char *getCPUHalfText() {
	return autoTxt[(emuSettings & HALF_CPU_SPEED)>>16];
}

void z80SpeedSet() {
	gZ80Speed++;
	if (gZ80Speed >= 5) {
		gZ80Speed = 0;
	}
	tweakZ80Speed(gZ80Speed);
}
const char *getZ80SpeedText() {
	return cpuSpeedTxt[gZ80Speed&7];
}

void soundSet() {
	soundMode++;
	if (soundMode > 1) {
		soundMode = 0;
	}
	soundInit();
}
const char *getSoundText() {
	return autoTxt[soundMode&1];
}
