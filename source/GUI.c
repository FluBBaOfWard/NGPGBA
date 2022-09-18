#include <gba.h>
#include <string.h>

#include "GUI.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Main.h"
#include "FileHandling.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"
#include "cpu.h"
#include "bios.h"
#include "TLCS900H/Version.h"
#include "ARMZ80/Version.h"
#include "K2GE/Version.h"

#define EMUVERSION "V0.5.0 2022-09-18"

#define HALF_CPU_SPEED		(1<<16)
#define ALLOW_SPEED_HACKS	(1<<17)

void hacksInit(void);

static void paletteChange(void);
static void languageSet(void);
static void machineSet(void);
static void batteryChange(void);
static void subBatteryChange(void);
static void speedHackSet(void);
static void cpuHalfSet(void);
static void uiMachine(void);

const fptr fnMain[] = {nullUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI};

const fptr fnList0[] = {uiDummy};
const fptr fnList1[] = {ui2, ui3, ui4, ui5, ui6, ui7, gbaSleep, resetGame};
const fptr fnList2[] = {ui8, loadState, saveState, saveSettings, resetGame};
const fptr fnList3[] = {autoBSet, autoASet, controllerSet, swapABSet};
const fptr fnList4[] = {/*scalingSet, flickSet,*/ gammaSet, paletteChange, fgrLayerSet, bgrLayerSet, sprLayerSet};
const fptr fnList5[] = {speedSet, autoStateSet, autoSettingsSet, autoPauseGameSet, debugTextSet, sleepSet};
const fptr fnList6[] = {languageSet, machineSet, batteryChange, subBatteryChange, speedHackSet, cpuHalfSet};
const fptr fnList7[] = {uiDummy};
const fptr fnList8[] = {quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame};
const fptr *const fnListX[] = {fnList0, fnList1, fnList2, fnList3, fnList4, fnList5, fnList6, fnList7, fnList8};
const u8 menuXItems[] = {ARRSIZE(fnList0), ARRSIZE(fnList1), ARRSIZE(fnList2), ARRSIZE(fnList3), ARRSIZE(fnList4), ARRSIZE(fnList5), ARRSIZE(fnList6), ARRSIZE(fnList7), ARRSIZE(fnList8)};
const fptr drawUIX[] = {uiNullNormal, uiMainMenu, uiFile, uiController, uiDisplay, uiSettings, uiMachine, uiAbout, uiLoadGame};
const u8 menuXBack[] = {0,0,1,1,1,1,1,1,2};

u8 gGammaValue = 0;
char gameInfoString[32];

const char *const autoTxt[]  = {"Off","On","With R"};
const char *const speedTxt[] = {"Normal","200%","Max","50%"};
const char *const sleepTxt[] = {"5min","10min","30min","Off"};
const char *const brighTxt[] = {"I","II","III","IIII","IIIII"};
const char *const ctrlTxt[]  = {"1P","2P"};
const char *const dispTxt[]  = {"Unscaled","Scaled"};
const char *const flickTxt[] = {"No Flicker","Flicker"};
const char *const bordTxt[]  = {"Black", "Border Color", "None"};
const char *const palTxt[]   = {"Black & White", "Red", "Green", "Blue", "Classic"};
const char *const langTxt[]  = {"Japanese", "English"};
const char *const machTxt[]  = {"NeoGeo Pocket Color", "NeoGeo Pocket"};

/// This is called at the start of the emulator
void setupGUI() {
	emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM | ALLOW_SPEED_HACKS;
//	keysSetRepeat(25, 4);	// Delay, repeat.
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
	while (loadGame()) {
		redrawUI();
		return;
	}
	closeMenu();
}

void uiNullNormal() {
	uiNullDefault();
}

void uiFile() {
	setupSubMenu("File Handling");
	drawMenuItem("Load Game->");
	drawMenuItem("Load State");
	drawMenuItem("Save State");
	drawMenuItem("Save Settings");
	drawMenuItem("Reset Game");
}

void uiMainMenu() {
	setupSubMenu("Main Menu");
	drawMenuItem("File->");
	drawMenuItem("Controller->");
	drawMenuItem("Display->");
	drawMenuItem("Settings->");
	drawMenuItem("Machine->");
	drawMenuItem("Help->");
	drawMenuItem("Sleep");
	drawMenuItem("Restart");
	if (enableExit) {
		drawMenuItem("Exit");
	}
}

void uiAbout() {
	setupSubMenu("Help");
	updateGameInfo();
	drawText("Select:   Power Button",3);
	drawText("Start:    Option Button",4);
	drawText("DPad:     Joystick",5);
	drawText("B:        A Button",6);
	drawText("A:        B Button",7);

	drawText(gameInfoString, 9);

	drawText("NGPGBA      " EMUVERSION, 15);
	drawText("ARMZ80      " ARMZ80VERSION, 16);
	drawText("ARMTLCS900H " TLCS900VERSION, 17);
	drawText("ARMK2GE     " K2GEVERSION, 18);
	drawText("ARMK2Audio  2" , 19);
}

void uiController() {
	setupSubMenu("Controller Settings");
	drawSubItem("B Autofire: ", autoTxt[autoB]);
	drawSubItem("A Autofire: ", autoTxt[autoA]);
	drawSubItem("Controller: ", ctrlTxt[(joyCfg>>29)&1]);
	drawSubItem("Swap A-B:   ", autoTxt[(joyCfg>>10)&1]);
}

void uiDisplay() {
	setupSubMenu("Display Settings");
	drawSubItem("Gamma: ", brighTxt[gGammaValue]);
	drawSubItem("B&W Palette: ", palTxt[gPaletteBank]);
	drawSubItem("Disable Foreground: ", autoTxt[gGfxMask&1]);
	drawSubItem("Disable Background: ", autoTxt[(gGfxMask>>1)&1]);
	drawSubItem("Disable Sprites: ", autoTxt[(gGfxMask>>4)&1]);
}

static void uiMachine() {
	setupSubMenu("Machine Settings");
	drawSubItem("Language: ",langTxt[gLang]);
	drawSubItem("Machine: ",machTxt[gMachine]);
	drawMenuItem(" Change Batteries");
	drawMenuItem(" Change Sub Battery");
	drawSubItem("Cpu speed hacks: ",autoTxt[(emuSettings&ALLOW_SPEED_HACKS)>>17]);
	drawSubItem("Half cpu speed: ",autoTxt[(emuSettings&HALF_CPU_SPEED)>>16]);
}

void uiSettings() {
	setupSubMenu("Other Settings");
	drawSubItem("Speed: ", speedTxt[(emuSettings>>6)&3]);
	drawSubItem("Autoload State: ", autoTxt[(emuSettings>>1)&1]);
	drawSubItem("Autosave Settings: ", autoTxt[(emuSettings>>4)&1]);
	drawSubItem("Autopause Game: ", autoTxt[emuSettings&1]);
	drawSubItem("Debug Output: ", autoTxt[gDebugSet&1]);
	drawSubItem("Autosleep: ", sleepTxt[(emuSettings>>8)&3]);
}

void uiLoadGame() {
	setupSubMenu("Load game");
}

void nullUINormal(int key) {
}

void nullUIDebug(int key) {
}

void resetGame() {
	loadCart(0);
}

void updateGameInfo() {
	NgpHeader *header = romSpacePtr;
	strlMerge(gameInfoString, "Game name: ", header->name, sizeof(gameInfoString));
	int len = strlen(gameInfoString);
	int2HexStr(&gameInfoString[len], header->catalog);
}

//---------------------------------------------------------------------------------
/// Switch between Player 1 & Player 2 controls
void controllerSet() {					// See io.s: refreshEMUjoypads
	joyCfg ^= 0x20000000;
}

/// Swap A & B buttons
void swapABSet() {
	joyCfg ^= 0x400;
}

/// Turn on/off scaling
void scalingSet(){
	gScaling ^= 0x01;
	refreshGfx();
}

/// Change gamma (brightness)
void gammaSet() {
	gGammaValue++;
	if (gGammaValue > 4) gGammaValue=0;
	paletteInit(gGammaValue);
	paletteTxAll();					// Make new palette visible
	setupMenuPalette();
}

/// Turn on/off rendering of foreground
void fgrLayerSet() {
	gGfxMask ^= 0x01;
}
/// Turn on/off rendering of background
void bgrLayerSet() {
	gGfxMask ^= 0x02;
}
/// Turn on/off rendering of sprites
void sprLayerSet() {
	gGfxMask ^= 0x10;
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

void machineSet() {
	gMachine ^= 0x01;
}

void speedHackSet() {
	emuSettings ^= ALLOW_SPEED_HACKS;
	emuSettings &= ~HALF_CPU_SPEED;
	hacksInit();
}
void cpuHalfSet() {
	emuSettings ^= HALF_CPU_SPEED;
	emuSettings &= ~ALLOW_SPEED_HACKS;
	tweakCpuSpeed(emuSettings & HALF_CPU_SPEED);
}

void batteryChange() {
	batteryLevel = 0xFFFF;				// 0xFFFF for 2 days battery?
}

void subBatteryChange() {
	g_subBatteryLevel = 0x3FFFFFF;		// 0x3FFFFFF for 2 years battery?
}
