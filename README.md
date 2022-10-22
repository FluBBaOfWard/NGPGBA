# NGPGBA V0.5.5

<img align="right" width="220" src="./logo.png" />

This is a SNK Neogeo Pocket (Color) emulator for the Nintendo GBA.

## How to use:

There is no builder included in the release yet.
The header is defined in Emubase.h, it's 64 bytes long, the size field is in
little endian, the 32bit id is 0x1A50474E (LE).
The name field can be 31 bytes plus a terminating zero.
There is an example header file included, "ngp.header".

When the emulator starts, you press L+R to open up the menu.
Now you can use the cross to navigate the menus, A to select an option, B to
go back a step.
To get maximum speed turn off sound and set Z80 CPU to 1/16 speed, maybe even
turn on overclock EWRAM.

## Menu:

### File:
	Load Game: Select a game to load.
	Load State: Load a previously saved state of the currently running game.
	Save State: Save a state of the currently running game.
	Load Flash RAM: Load flash ram for the currently running game.
	Save Flash RAM: Save flash ram for the currently running game.
	Save Settings: Save the current settings.
	Reset Game: Reset the currently running game.

### Controller:
	Autofire: Select if you want autofire.
	Controller: 2P start a 2 player game.
	Swap A/B: Swap which GBA button is mapped to which NGP button.

### Display:
	Mono Palette: Here you can select the palette for B & W games.
	Gamma: Lets you change the gamma ("brightness").
### Machine Settings:
	Language: Select between Japanese and English.
	Machine: Select the emulated machine.
	Change Batteries: Change to new main batteries (AA/LR6).
	Change Sub Battery: Change to a new sub battery (CR2032).
	Cpu Speed Hacks: Allow speed hacks.
	Half Cpu Speed: This halves the emulated cpu speed.
		Can make games faster.
	Z80 Clock: Down shift the clock of the Z80.
		This can make a huge impact to the speed but can also screw up sound.
	Sound: On / Off.
### Settings:
	Speed: Switch between speed modes.
		Normal: Game runs at it's normal speed.
		200%: Game runs at double speed.
		Max: Games can run up to 4 times normal speed (might change).
		50%: Game runs at half speed.
	Autoload State: Toggle Savestate autoloading.
		Automagically load the savestate associated with the current game.
	Autoload Flash RAM: Toggle flash/save ram autoloading.
		Automagically load the flash ram associated with the current game.
	Autosave Settings: This will save settings when
		leaving menu if any changes are made.
	Autopause Game: Toggle if the game should pause when opening the menu.
	Overclock EWRAM: Changes the waitstates on EWRAM between 2 and 1,
		might damage your GBA and uses more power, around 10% speedgain.
		Use at your own risk!
	Autosleep: Change the autosleep time, also see Sleep.
### Debug:
	Debug Output: Show FPS and logged text.
	Disable Foreground: Turn on/off foreground rendering.
	Disable Background: Turn on/off background rendering.
	Disable Sprites: Turn on/off sprite rendering.
	Step Frame: Emulate one frame.

### About:
	Some dumb info about the game and emulator...

### Sleep:
	Put the GBA into sleepmode.
	START+SELECT wakes up from sleep mode (activated from this menu or from	5/10/30	minutes of inactivity).

### Quit Emulator:
	Tries to reset the Flashcart and reboots the GBA.

## Controls:
	GBA A & B buttons are mapped to NeoGeo Pocket B & A.
	GBA Start is mapped to NeoGeo Pocket Option.
	GBA Select is mapped to NeoGeo Pocket Power.
	GBA d-pad is mapped to NeoGeo Pocket d-pad.
	GBA L button is mapped to NeoGeo Pocket D (debug).

## Games:
	Memories of - Pure: Glitches everywhere.
	Sonic The Hedgehog: First boss sometimes disappear.
	Super Real Mahjong - Premium Collection: Graphic bugs on intro.

## Credits:

Huge thanks to Loopy for the incredible PocketNES, without it this emu would
probably never have been made.
Thanks to:
Dwedit for help and inspiration with a lot of things.


Fredrik Ahlström

Twitter @TheRealFluBBa

http://www.github.com/FluBBaOfWard
