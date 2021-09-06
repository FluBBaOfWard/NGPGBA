#include <gba.h>

#include "GUI.h"
#include "Shared/EmuMenu.h"
#include "Shared/AsmExtra.h"
#include "Main.h"
#include "FileHandling.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"
#include "TLCS900H/Version.h"
#include "ARMZ80/Version.h"
#include "K2GE/Version.h"

#define EMUVERSION "V0.5.0 2021-09-06"

const fptr fnMain[] = {nullUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI};

const fptr fnList0[] = {uiDummy};
const fptr fnList1[] = {ui2, ui3, ui4, ui5, ui6, gbaSleep, resetGame};
const fptr fnList2[] = {ui8, loadState, saveState, saveSettings, resetGame};
const fptr fnList3[] = {autoBSet, autoASet, controllerSet, swapABSet};
const fptr fnList4[] = {scalingSet, flickSet, gammaSet, fgrLayerSet, bgrLayerSet, sprLayerSet};
const fptr fnList5[] = {speedSet, autoStateSet, autoSettingsSet, autoPauseGameSet, debugTextSet, sleepSet};
const fptr fnList6[] = {uiDummy};
const fptr fnList7[] = {quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame};
const fptr *const fnListX[] = {fnList0, fnList1, fnList2, fnList3, fnList4, fnList5, fnList6, fnList7};
const u8 menuXitems[] = {ARRSIZE(fnList0), ARRSIZE(fnList1), ARRSIZE(fnList2), ARRSIZE(fnList3), ARRSIZE(fnList4), ARRSIZE(fnList5), ARRSIZE(fnList6), ARRSIZE(fnList7)};
const fptr drawuiX[] = {uiNullNormal, uiMainMenu, uiFile, uiController, uiDisplay, uiSettings, uiAbout, uiLoadGame};
const u8 menuXback[] = {0,0,1,1,1,1,1,2};

int emuSettings = 1;
u8 g_gammaValue = 0;

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
//	keysSetRepeat(25, 4);	// Delay, repeat.
	closeMenu();
}

/// This is called when going from emu to ui.
void enterGUI() {
}

/// This is called going from ui to emu.
void exitGUI() {
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
	drawMenuItem("Help->");
	drawMenuItem("Sleep");
	drawMenuItem("Restart");
	if (enableExit) {
		drawMenuItem("Exit");
	}
}

void uiAbout() {
	setupSubMenu("Help");
	drawText("Select:   Power Button",3);
	drawText("Start:    Option Button",4);
	drawText("DPad:     Joystick",5);
	drawText("B:        A Button",6);
	drawText("A:        B Button",7);

	drawText(" NGPDS        " EMUVERSION, 15);
	drawText(" ARMZ80       " ARMZ80VERSION, 16);
	drawText(" ARMTLCS-900H " TLCS900VERSION, 17);
	drawText(" ARMK2GE      " K2GEVERSION, 18);
	drawText(" ARMK2Audio   2" , 19);
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
	drawSubItem("Display: ", dispTxt[g_scaling]);
	drawSubItem("Scaling: ", flickTxt[g_flicker]);
	drawSubItem("Gamma: ", brighTxt[g_gammaValue]);
	drawSubItem("Disable Foreground: ", autoTxt[g_gfxMask&1]);
	drawSubItem("Disable Background: ", autoTxt[(g_gfxMask>>1)&1]);
	drawSubItem("Disable Sprites: ", autoTxt[(g_gfxMask>>4)&1]);
}

void uiSettings() {
	setupSubMenu("Other Settings");
	drawSubItem("Speed: ", speedTxt[(emuSettings>>6)&3]);
	drawSubItem("Autoload State: ", autoTxt[(emuSettings>>2)&1]);
	drawSubItem("Autosave Settings: ", autoTxt[(emuSettings>>9)&1]);
	drawSubItem("Autopause Game: ", autoTxt[emuSettings&1]);
	drawSubItem("Debug Output: ", autoTxt[g_debugSet&1]);
	drawSubItem("Autosleep: ", sleepTxt[(emuSettings>>4)&3]);
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
	g_scaling ^= 0x01;
	refreshGfx();
}

/// Change gamma (brightness)
void gammaSet() {
	g_gammaValue++;
	if (g_gammaValue > 4) g_gammaValue=0;
	paletteInit(g_gammaValue);
	paletteTxAll();					// Make new palette visible
	setupMenuPalette();
}

/// Turn on/off rendering of foreground
void fgrLayerSet() {
	g_gfxMask ^= 0x01;
}
/// Turn on/off rendering of background
void bgrLayerSet() {
	g_gfxMask ^= 0x02;
}
/// Turn on/off rendering of sprites
void sprLayerSet() {
	g_gfxMask ^= 0x10;
}
