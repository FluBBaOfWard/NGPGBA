//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#include <gba.h>
#include <string.h>

#include "Memory.h"
#include "bios.h"
#include "K2GE/K2GE.h"
#include "Shared/EmuMenu.h"
#include "FileHandling.h"
#include "TLCS900H/TLCS900H.h"

//=============================================================================

extern u32 gRomSize;			// From Cart.s
extern u8 gLang;				// From Cart.s
extern u8 gMachine;				// From Cart.s
extern u8 gPaletteBank;			// From Cart.s
extern u32 sngBIOSHLE;

//=============================================================================

const u8 font[0x800] = {

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,
	0x00,0x00,0x00,0xF8,0xF8,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
	0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,
	0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
	0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

	0x20,0x30,0x38,0x3C,0x38,0x30,0x20,0x00,0x04,0x0C,0x1C,0x3C,0x1C,0x0C,0x04,0x00,
	0x00,0x00,0xFE,0x7C,0x38,0x10,0x00,0x00,0x00,0x00,0x10,0x38,0x7C,0xFE,0x00,0x00,
	0x20,0x3C,0x08,0x3C,0x28,0x7E,0x08,0x00,0x3C,0x24,0x3C,0x24,0x3C,0x24,0x24,0x00,
	0x3C,0x24,0x24,0x3C,0x24,0x24,0x3C,0x00,0x10,0x10,0x54,0x54,0x10,0x28,0xC6,0x00,
	0x10,0x12,0xD4,0x58,0x54,0x92,0x10,0x00,0x10,0x10,0x7C,0x10,0x38,0x54,0x92,0x00,
	0x10,0x28,0x7C,0x92,0x38,0x54,0xFE,0x00,0x10,0x10,0x10,0x7C,0x10,0x10,0xFE,0x00,
	0x7F,0xFF,0xE0,0xFF,0x7F,0x01,0xFF,0xFF,0xDC,0xDE,0x1F,0x9F,0xDF,0xDD,0xDC,0x9C,
	0x3B,0x3B,0x3B,0xBB,0xFB,0xFB,0xFB,0x7B,0x8F,0x9E,0xBC,0xF8,0xF8,0xBC,0x9E,0x8F,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x08,0x10,0x00,0x18,0x00,
	0x6C,0x6C,0x24,0x48,0x00,0x00,0x00,0x00,0x14,0x14,0xFE,0x28,0xFE,0x50,0x50,0x00,
	0x10,0x7C,0x90,0x7C,0x12,0xFC,0x10,0x00,0x42,0xA4,0xA8,0x54,0x2A,0x4A,0x84,0x00,
	0x30,0x48,0x38,0x62,0x94,0x88,0x76,0x00,0x18,0x18,0x08,0x10,0x00,0x00,0x00,0x00,
	0x08,0x10,0x20,0x20,0x20,0x10,0x08,0x00,0x20,0x10,0x08,0x08,0x08,0x10,0x20,0x00,
	0x10,0x92,0x54,0x38,0x38,0x54,0x92,0x00,0x10,0x10,0x10,0xFE,0x10,0x10,0x10,0x00,
	0x00,0x00,0x00,0x30,0x30,0x10,0x20,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00,

	0x3C,0x42,0x46,0x5A,0x62,0x42,0x3C,0x00,0x08,0x38,0x08,0x08,0x08,0x08,0x08,0x00,
	0x3C,0x42,0x42,0x0C,0x30,0x40,0x7E,0x00,0x3C,0x42,0x02,0x1C,0x02,0x42,0x3C,0x00,
	0x0C,0x14,0x24,0x44,0x7E,0x04,0x04,0x00,0x7E,0x40,0x7C,0x02,0x02,0x42,0x3C,0x00,
	0x3C,0x40,0x7C,0x42,0x42,0x42,0x3C,0x00,0x7E,0x02,0x04,0x08,0x08,0x10,0x10,0x00,
	0x3C,0x42,0x42,0x3C,0x42,0x42,0x3C,0x00,0x3C,0x42,0x42,0x42,0x3E,0x02,0x3C,0x00,
	0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x18,0x08,0x10,0x00,
	0x00,0x08,0x10,0x20,0x10,0x08,0x00,0x00,0x00,0x00,0x3C,0x00,0x3C,0x00,0x00,0x00,
	0x00,0x10,0x08,0x04,0x08,0x10,0x00,0x00,0x3C,0x62,0x62,0x0C,0x18,0x00,0x18,0x00,

	0x7C,0x82,0xBA,0xA2,0xBA,0x82,0x7C,0x00,0x10,0x28,0x28,0x44,0x7C,0x82,0x82,0x00,
	0x7C,0x42,0x42,0x7C,0x42,0x42,0x7C,0x00,0x1C,0x22,0x40,0x40,0x40,0x22,0x1C,0x00,
	0x78,0x44,0x42,0x42,0x42,0x44,0x78,0x00,0x7E,0x40,0x40,0x7E,0x40,0x40,0x7E,0x00,
	0x7E,0x40,0x40,0x7C,0x40,0x40,0x40,0x00,0x3C,0x42,0x80,0x9E,0x82,0x46,0x3A,0x00,
	0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,
	0x02,0x02,0x02,0x02,0x42,0x42,0x3C,0x00,0x42,0x44,0x48,0x50,0x68,0x44,0x42,0x00,
	0x40,0x40,0x40,0x40,0x40,0x40,0x7E,0x00,0x82,0xC6,0xAA,0x92,0x82,0x82,0x82,0x00,
	0x42,0x62,0x52,0x4A,0x46,0x42,0x42,0x00,0x38,0x44,0x82,0x82,0x82,0x44,0x38,0x00,

	0x7C,0x42,0x42,0x7C,0x40,0x40,0x40,0x00,0x38,0x44,0x82,0x82,0x8A,0x44,0x3A,0x00,
	0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x00,0x3C,0x42,0x40,0x3C,0x02,0x42,0x3C,0x00,
	0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,
	0x82,0x82,0x44,0x44,0x28,0x28,0x10,0x00,0x82,0x92,0x92,0xAA,0xAA,0x44,0x44,0x00,
	0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,0x82,0x44,0x28,0x10,0x10,0x10,0x10,0x00,
	0x7E,0x04,0x08,0x10,0x20,0x40,0x7E,0x00,0x18,0x10,0x10,0x10,0x10,0x10,0x18,0x00,
	0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x00,0x18,0x08,0x08,0x08,0x08,0x08,0x18,0x00,
	0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,

	0x08,0x10,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x78,0x04,0x7C,0x84,0x84,0x7E,0x00,
	0x40,0x40,0x7C,0x42,0x42,0x42,0x3C,0x00,0x00,0x00,0x3C,0x42,0x40,0x42,0x3C,0x00,
	0x02,0x02,0x3E,0x42,0x42,0x42,0x3C,0x00,0x00,0x00,0x3C,0x42,0x7E,0x40,0x3E,0x00,
	0x0C,0x10,0x3E,0x10,0x10,0x10,0x10,0x00,0x00,0x3C,0x42,0x42,0x3E,0x02,0x7C,0x00,
	0x40,0x40,0x7C,0x42,0x42,0x42,0x42,0x00,0x18,0x18,0x00,0x08,0x08,0x08,0x08,0x00,
	0x06,0x06,0x00,0x02,0x42,0x42,0x3C,0x00,0x20,0x20,0x26,0x28,0x30,0x28,0x26,0x00,
	0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x80,0xEC,0x92,0x92,0x92,0x92,0x00,
	0x00,0x40,0x78,0x44,0x44,0x44,0x44,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00,

	0x00,0x3C,0x42,0x42,0x7C,0x40,0x40,0x00,0x00,0x78,0x84,0x84,0x7C,0x04,0x06,0x00,
	0x00,0x00,0x5C,0x62,0x40,0x40,0x40,0x00,0x00,0x00,0x3E,0x40,0x3C,0x02,0x7C,0x00,
	0x00,0x10,0x7C,0x10,0x10,0x10,0x0E,0x00,0x00,0x00,0x42,0x42,0x42,0x42,0x3F,0x00,
	0x00,0x00,0x42,0x42,0x24,0x24,0x18,0x00,0x00,0x00,0x92,0x92,0x92,0x92,0x6C,0x00,
	0x00,0x00,0x42,0x24,0x18,0x24,0x42,0x00,0x00,0x00,0x42,0x42,0x3E,0x02,0x7C,0x00,
	0x00,0x00,0x7E,0x02,0x3C,0x40,0x7E,0x00,0x08,0x10,0x10,0x20,0x10,0x10,0x08,0x00,
	0x10,0x10,0x10,0x00,0x10,0x10,0x10,0x00,0x20,0x10,0x10,0x08,0x10,0x10,0x20,0x00,
	0x00,0x00,0x60,0x92,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x90,0x90,0x60,0x00,
	0x1E,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0xF0,0x00,
	0x00,0x00,0x00,0x80,0x40,0x60,0x60,0x00,0x00,0x00,0x00,0x30,0x78,0x30,0x00,0x00,
	0x20,0xF8,0x26,0x78,0xD0,0x80,0x7C,0x00,0x00,0x10,0x3A,0x1C,0x36,0x5A,0x36,0x00,
	0x00,0x00,0x44,0x42,0x42,0x42,0x30,0x00,0x00,0x3C,0x00,0x3C,0x42,0x02,0x3C,0x00,
	0x00,0x3C,0x00,0x7C,0x08,0x38,0x66,0x00,0x00,0x14,0x72,0x1C,0x32,0x52,0x34,0x00,
	0x00,0x28,0x2C,0x3A,0x62,0x16,0x10,0x00,0x00,0x08,0x5C,0x6A,0x4A,0x0C,0x18,0x00,
	0x00,0x08,0x0C,0x38,0x4C,0x4A,0x38,0x00,0x00,0x00,0x00,0x1C,0x62,0x02,0x1C,0x00,

	0x00,0x00,0x80,0x7E,0x00,0x00,0x00,0x00,0x28,0xF2,0x3C,0x6A,0xAA,0xB6,0xEC,0x00,
	0x80,0x88,0x84,0x84,0x82,0x92,0x70,0x00,0x78,0x00,0x3C,0xC2,0x02,0x04,0x78,0x00,
	0x78,0x00,0xFC,0x08,0x30,0x50,0x9E,0x00,0x2C,0xF2,0x20,0x7C,0xA2,0xA2,0xE4,0x00,
	0x28,0xF4,0x2A,0x4A,0x4A,0x88,0xB0,0x00,0x20,0xFC,0x12,0xFC,0x08,0xC2,0x7C,0x00,
	0x04,0x18,0x60,0x80,0xC0,0x30,0x0E,0x00,0x84,0xBE,0x84,0x84,0x84,0x84,0x58,0x00,
	0x00,0x7C,0x02,0x00,0x80,0x82,0x7E,0x00,0x20,0xFE,0x10,0x78,0x8C,0xC0,0x7C,0x00,
	0x80,0x80,0x80,0x80,0x82,0x84,0x78,0x00,0x04,0xFE,0x3C,0x44,0x7C,0x04,0x78,0x00,
	0x44,0x5E,0xF4,0x44,0x48,0x40,0x3E,0x00,0x44,0x58,0xE0,0x3E,0xC0,0x40,0x3C,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x90,0x90,0x60,0x00,
	0x1E,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0xF0,0x00,
	0x00,0x00,0x00,0x80,0x40,0x60,0x60,0x00,0x00,0x00,0x00,0x30,0x78,0x30,0x00,0x00,
	0x1E,0x62,0x1E,0x62,0x04,0x0C,0x30,0x00,0x00,0x00,0x1E,0x6A,0x0E,0x08,0x18,0x00,
	0x00,0x00,0x06,0x18,0x68,0x08,0x08,0x00,0x00,0x10,0x1C,0x72,0x42,0x04,0x38,0x00,
	0x00,0x00,0x0C,0x78,0x10,0x10,0x7E,0x00,0x00,0x08,0x08,0x7C,0x18,0x28,0x48,0x00,
	0x00,0x00,0x26,0x1A,0x72,0x10,0x08,0x00,0x00,0x00,0x0C,0x34,0x04,0x08,0x7E,0x00,
	0x00,0x00,0x78,0x04,0x3C,0x04,0x78,0x00,0x00,0x00,0x52,0x4A,0x22,0x04,0x38,0x00,

	0x00,0x00,0x80,0x7E,0x00,0x00,0x00,0x00,0x3C,0xC2,0x14,0x1C,0x10,0x30,0x60,0x00,
	0x02,0x0C,0x38,0xC8,0x08,0x08,0x08,0x00,0x60,0x3C,0xE2,0x82,0x82,0x04,0x38,0x00,
	0x00,0x1C,0x70,0x10,0x10,0x1C,0xE2,0x00,0x08,0xFE,0x18,0x38,0x68,0xC8,0x18,0x00,
	0x10,0x3E,0xD2,0x12,0x22,0x62,0xCC,0x00,0x20,0x3C,0xF0,0x1E,0xF0,0x08,0x08,0x00,
	0x10,0x3E,0x62,0xC2,0x04,0x0C,0x70,0x00,0x40,0x7E,0x44,0x84,0x84,0x08,0x30,0x00,
	0x3E,0xC2,0x02,0x02,0x02,0x02,0xFC,0x00,0x44,0x5E,0xE4,0x44,0x44,0x08,0xF0,0x00,
	0x60,0x12,0xC2,0x22,0x04,0x04,0xF8,0x00,0x3C,0xC6,0x0C,0x08,0x38,0x6C,0xC6,0x00,
	0x40,0x4E,0x72,0xC4,0x4C,0x40,0x3E,0x00,0x82,0x42,0x62,0x04,0x04,0x08,0x70,0x00,

	0x3C,0x42,0x72,0x8A,0x04,0x0C,0x70,0x00,0x0C,0xF8,0x10,0xFE,0x10,0x10,0x60,0x00,
	0x22,0xA2,0x92,0x42,0x04,0x08,0x70,0x00,0x3C,0x40,0x1E,0xE8,0x08,0x10,0x60,0x00,
	0x40,0x40,0x70,0x4C,0x42,0x40,0x40,0x00,0x08,0x3E,0xC8,0x08,0x08,0x18,0x70,0x00,
	0x00,0x1C,0x60,0x00,0x00,0x3C,0xC2,0x00,0x3C,0xC2,0x26,0x38,0x1C,0x36,0xE2,0x00,
	0x10,0x3C,0xC6,0x1C,0x38,0xD6,0x12,0x00,0x02,0x02,0x02,0x06,0x04,0x1C,0xF0,0x00,
	0x18,0x4C,0x44,0x46,0x42,0x82,0x82,0x00,0x80,0x86,0xBC,0xE0,0x80,0x80,0x7E,0x00,
	0x3C,0xC2,0x02,0x02,0x04,0x08,0x30,0x00,0x30,0x48,0x4C,0x84,0x86,0x02,0x02,0x00,
	0x10,0xFE,0x10,0x54,0x52,0x92,0x92,0x00,0x3C,0xC2,0x02,0x44,0x28,0x10,0x0C,0x00,

	0x70,0x0C,0x60,0x18,0xC4,0x30,0x0E,0x00,0x30,0x40,0x4C,0x84,0x8E,0xBA,0x62,0x00,
	0x04,0x04,0x64,0x18,0x0C,0x16,0xE2,0x00,0x1C,0xE0,0x3E,0xE0,0x20,0x20,0x1E,0x00,
	0x4E,0x52,0x62,0xE4,0x20,0x10,0x18,0x00,0x1C,0x64,0x04,0x08,0x08,0x10,0xFE,0x00,
	0x1C,0x62,0x02,0x3E,0x02,0x02,0x7C,0x00,0x3C,0xC0,0x3E,0xC2,0x02,0x04,0x78,0x00,
	0x44,0x42,0x42,0x42,0x22,0x04,0x78,0x00,0x50,0x50,0x52,0x52,0x52,0x54,0x88,0x00,
	0x80,0x80,0x82,0x82,0x84,0x88,0xF0,0x00,0x1C,0xE2,0x82,0x82,0x82,0x4C,0x74,0x00,
	0x3C,0xC2,0x82,0x82,0x02,0x04,0x38,0x00,0xC0,0x62,0x22,0x02,0x04,0x08,0xF0,0x00,
	0x00,0x00,0x00,0x00,0x0A,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x02,0x00,

	0x20,0xF8,0x40,0x5E,0x80,0xA0,0x9E,0x00,0x20,0xFE,0x40,0x7C,0xC2,0x86,0x3C,0x00,
	0x00,0x3C,0xC6,0x02,0x02,0x0C,0x38,0x00,0x0E,0xF8,0x10,0x20,0x20,0x10,0x0E,0x00,
	0x40,0x4C,0x30,0x40,0x80,0x80,0x7E,0x00,0x44,0xF2,0x4A,0x9C,0xA4,0xA6,0x3A,0x00,
	0x40,0x5C,0x82,0x80,0xA0,0xA0,0x9E,0x00,0x48,0x7C,0x52,0xB2,0xBE,0xAA,0x4C,0x00,
	0x20,0xFC,0x32,0x62,0xEE,0xAA,0x2C,0x00,0x38,0x54,0x92,0x92,0xB2,0xA2,0x4C,0x00,
	0x44,0x5E,0x84,0x9C,0xA4,0xA6,0x9C,0x00,0x28,0xEE,0x44,0x84,0x84,0x44,0x38,0x00,
	0x78,0x10,0x64,0x34,0x8A,0x8A,0x30,0x00,0x30,0x58,0x48,0x84,0x84,0x02,0x02,0x00,
	0xBC,0x88,0xBE,0x84,0xBC,0xA6,0x9C,0x00,0x68,0x1E,0x68,0x1E,0x78,0x8C,0x7A,0x00,

	0x70,0x14,0x7C,0x96,0x94,0x94,0x68,0x00,0x2C,0xF2,0x60,0xA0,0xA2,0xC2,0x7C,0x00,
	0x48,0x7C,0x6A,0xAA,0xB2,0xB2,0x6C,0x00,0x10,0xF8,0x20,0xF8,0x22,0x22,0x1C,0x00,
	0x48,0x5C,0x6A,0xC2,0x64,0x20,0x18,0x00,0x10,0xBC,0xD6,0xCA,0xAA,0x1C,0x70,0x00,
	0x10,0x1C,0x12,0x70,0x9C,0x92,0x70,0x00,0xE0,0x18,0x40,0x7C,0xC2,0x82,0x3C,0x00,
	0x44,0x42,0x82,0xA2,0x62,0x04,0x78,0x00,0x7C,0x38,0x7C,0xC2,0xBA,0x26,0x3C,0x00,
	0x48,0xD4,0x64,0x64,0xC4,0xC4,0x46,0x00,0x7C,0x30,0x7C,0xC2,0x82,0x06,0x3C,0x00,
	0x20,0xFC,0x32,0x62,0xE2,0xA2,0x2C,0x00,0x10,0x30,0x60,0x72,0xD2,0x92,0x9C,0x00,
	0x00,0x00,0x00,0x00,0x0A,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x02,0x05,0x02,0x00
};

const u8 snkLogo[0x40] = {
	0xFF,0x3F,0xFF,0xFF,0x00,0xFC,0xFF,0xFF,0xFF,0x3F,0x03,0x00,0xFF,0xFF,0xFF,0xFF,
	0xF0,0xF3,0xFC,0xF3,0xFF,0x03,0xFF,0xC3,0xFF,0xF3,0xF3,0xF3,0xF0,0xF3,0xF0,0xC3,
	0xCF,0x0F,0xCF,0x0F,0xCF,0x0F,0xCF,0xCF,0xCF,0xFF,0xCF,0xFF,0xCF,0xFF,0xCF,0x3F,
	0xFF,0xC0,0xFC,0xC3,0xF0,0xCF,0xC0,0xFF,0xC0,0xFF,0xF0,0xCF,0xFC,0xC3,0xFF,0xC0
};

bool installHleBios(u8 *ngpBios)
{
	// === Install the reverse engineered bios
	int i;

	u32 callTable[] =
	{
		0xFF27A2,		//0x00		VECT_SHUTDOWN
		0xFF1034,		//0x01		VECT_CLOCKGEARSET
		0xFF1440,		//0x02		VECT_RTCGET
		0xFF12B4,		//0x03		VECT_RTCSET
		0xFF1222,		//0x04		VECT_INTLVSET
		0xFF8D8A,		//0x05		VECT_SYSFONTSET
		0xFF6FD8,		//0x06		VECT_FLASHWRITE
		0xFF7042,		//0x07		VECT_FLASHALLERS
		0xFF7082,		//0x08		VECT_FLASHERS
		0xFF149B,		//0x09		VECT_ALARMSET
		0xFF1033,		//0x0A		Reserved (just RET)
		0xFF1487,		//0x0B		VECT_ALARMDOWNSET
		0xFF731F,		//0x0C		Another FLASHPROTECT?
		0xFF70CA,		//0x0D		VECT_FLASHPROTECT
		0xFF17C4,		//0x0E		VECT_GEMODESET
		0xFF1032,		//0x0F		Reserved (just RET)

		0xFF2BBD,		//0x10		VECT_COMINIT
		0xFF2C0C,		//0x11		VECT_COMSENDSTART
		0xFF2C44,		//0x12		VECT_COMRECIVESTART
		0xFF2C86,		//0x13		VECT_COMCREATEDATA
		0xFF2CB4,		//0x14		VECT_COMGETDATA
		0xFF2D27,		//0x15		VECT_COMONRTS
		0xFF2D33,		//0x16		VECT_COMOFFRTS
		0xFF2D3A,		//0x17		VECT_COMSENDSTATUS
		0xFF2D4E,		//0x18		VECT_COMRECIVESTATUS
		0xFF2D6C,		//0x19		VECT_COMCREATEBUFDATA
		0xFF2D85,		//0x1A		VECT_COMGETBUFDATA
	};

	u32 irqTable[] =
	{
		0xFF204A,		//0x00		SWI_0, Reset
		0xFF2772,		//0x01		SWI_1, Call handler
		0xFF2305,		//0x02		SWI_2, Illegal instruction interrupt
		0xFF2202,		//0x03		SWI_3, User controlled
		0xFF220B,		//0x04		SWI_4, User controlled
		0xFF2214,		//0x05		SWI_5, User controlled
		0xFF221D,		//0x06		SWI_6, User controlled
		0xFF2226,		//0x07		SWI_7, ?
		0xFF1898,		//0x08		NMI_PowerButton
		0xFF2D98,		//0x09		NMI_WatchDogTimer
		0xFF2856,		//0x0A		IRQ_RTCAlarm
		0xFF2163,		//0x0B		IRQ_VerticalBlanking
		0xFF2282,		//0x0C		IRQ_Z80
		0xFF2B25,		//0x0D		IRQ_INT6? unused
		0xFF2DB6,		//0x0E		IRQ_INT7? joypad interrupt?
		0xFF22A4,		//0x0F		IRQ_??? unused

		0xFF22A5,		//0x10		IRQ_Timer0
		0xFF22AE,		//0x11		IRQ_Timer1
		0xFF22B7,		//0x12		IRQ_Timer2
		0xFF22C0,		//0x13		IRQ_Timer3
		0xFF22C9,		//0x14		IRQ_Timer4, unused
		0xFF22CA,		//0x15		IRQ_Timer5, unused
		0xFF22CB,		//0x16		IRQ_Timer6, unused
		0xFF22CC,		//0x17		IRQ_Timer7, unused
		0xFF22CD,		//0x18		IRQ_SerialTX0
		0xFF22D6,		//0x19		IRQ_SerialRX0
		0xFF22DF,		//0x1A		IRQ_SerialTX1, unused
		0xFF22E0,		//0x1B		IRQ_SerialRX1, unused
		0xFF2DCE,		//0x1C		IRQ_INTAD, battery measure
		0xFF22E1,		//0x1D		IRQ_DMA0End
		0xFF22EA,		//0x1E		IRQ_DMA1End
		0xFF22F3,		//0x1F		IRQ_DMA2End
		0xFF22FC,		//0x20		IRQ_DMA3End
	};

	// System Call Table, install iBIOSHLE instructions
	for (i = 0; i < ARRSIZE(callTable); i++)
	{
		*(u32*)(ngpBios + 0xFE00 + (i * 4)) = callTable[i];
		ngpBios[callTable[i] & 0xFFFF] = 0x1F;			// iBIOSHLE
		ngpBios[(callTable[i] + 1) & 0xFFFF] = i;		// iBIOSHLE num
		ngpBios[(callTable[i] + 2) & 0xFFFF] = 0x0E;	// RET
	}

	// IRQ vector Table, install
	for (i = 0; i < ARRSIZE(irqTable); i++)
	{
		*(u32*)(ngpBios + 0xFF00 + (i * 4)) = irqTable[i];
		ngpBios[irqTable[i] & 0xFFFF] = 0x1F;			// iBIOSHLE
		ngpBios[(irqTable[i] + 1) & 0xFFFF] = i + 0x40;	// iBIOSHLE num
		ngpBios[(irqTable[i] + 2) & 0xFFFF] = 0x07;		// RETI
	}

	// System Font
	memcpy(ngpBios + 0x8DCF, font, 0x800);

	// Default Interrupt handler
	ngpBios[0x23DF] = 0x07;		// RETI
	// System calls patch
	ngpBios[0x1032] = 0x0E;		// RET
	ngpBios[0x1033] = 0x0E;		// RET
	ngpBios[0x27A4] = 0x68;		// - JR 0xFE (Infinite loop!) VECT_SHUTDOWN
	ngpBios[0x27A5] = 0xFE;
	// IRQ Vector patch
	ngpBios[0x2305] = 0x68;		// - JR 0xFE (Infinite loop!) SWI_2 Illegal instruction interrupt
	ngpBios[0x2306] = 0xFE;
	ngpBios[0x2226] = 0x68;		// - JR 0xFE (Infinite loop!) SWI_7 ???
	ngpBios[0x2227] = 0xFE;
	ngpBios[0x1898] = 0x07;		// RETI
	ngpBios[0x2D98] = 0x07;		// RETI
	ngpBios[0x2B25] = 0x07;		// RETI
	ngpBios[0x2DB6] = 0x07;		// RETI
	ngpBios[0x22A4] = 0x07;		// RETI
	ngpBios[0x22C9] = 0x07;		// RETI
	ngpBios[0x22CA] = 0x07;		// RETI
	ngpBios[0x22CB] = 0x07;		// RETI
	ngpBios[0x22CC] = 0x07;		// RETI
	ngpBios[0x22DF] = 0x07;		// RETI
	ngpBios[0x22E0] = 0x07;		// RETI
	ngpBios[0x2DCE] = 0x07;		// RETI

	// ==========

	tlcs900HRedirectOpcode(0x1F, &sngBIOSHLE);
	return true;				// Success
}

void resetBios(NgpHeader *ngpHeader)
{
	int i;

	// Clear all RAM first
	for (i = 0; i < 0x1000; i++) {
		t9StoreLX(0, 0x4000 + i*4);
	}

//=============================================================================
//006C00 -> 006FFF	BIOS Workspace
//==================================

	t9StoreLX(ngpHeader->startPC, 0x6C00);		// Start

	t9StoreWX(ngpHeader->catalog, 0x6C04);
	t9StoreWX(ngpHeader->catalog, 0x6E82);

	t9StoreBX(ngpHeader->subCatalog, 0x6C06);
	t9StoreBX(ngpHeader->subCatalog, 0x6E84);

	for (i = 0; i < 12; i++) {
		t9StoreBX(ngpHeader->name[i], 0x6C08 + i);
	}

	t9StoreBX(0x01, 0x6C58);
	t9StoreBX(0x00, 0x6C59);
	// 32MBit cart?
	if (gRomSize > 0x200000) {
		t9StoreBX(0x03, 0x6C58);
		t9StoreBX(0x03, 0x6C59);
	}
	else if (gRomSize > 0x100000) {
		t9StoreBX(0x03, 0x6C58);
	}
	else if (gRomSize > 0x080000) {
		t9StoreBX(0x02, 0x6C58);
	}

	t9StoreBX(0x01, 0x6C55);		// 0x01 = Commercial game

	t9StoreWX(0x03FF, 0x6F80);	// Lots of battery power!

	t9StoreBX(0x40, 0x6F84);		// "Power On" startup
	t9StoreBX(0x00, 0x6F85);		// No shutdown request
	t9StoreBX(0x00, 0x6F86);		// No user answer (?)

	t9StoreWX(0xA5A5, 0x6C7A);	// Running mode
	t9StoreWX(0x5AA5, 0x6C7C);	// Running mode

	// Language: 0 = Japanese, 1 = English
	if (gLang) {
		t9StoreBX(0x01, 0x6F87);
	}
	else {
		t9StoreBX(0x00, 0x6F87);
	}
	t9StoreBX(gPaletteBank, 0x6F94);

	// Color Mode Selection: 0x00 = B&W, 0x10 = Colour
	int color = ngpHeader->mode;
	if (gMachine == HW_NGPMONO) {
		color = 0;
	}
	t9StoreBX(color, 0x6F90);		// Game Displaymode
	t9StoreBX(color, 0x6F95);		// Current Displaymode
	t9StoreBX(color, 0x6F91);		// Machine
	if (gMachine == HW_NGPCOLOR) {
		t9StoreBX(0x10, 0x6F91);		// Machine
	}
	// User Interrupt table
	for (i = 0; i < 0x12; i++) {
		t9StoreLX(0x00FF23DF, 0x6FB8 + i * 4);
	}

//=============================================================================
//008000 -> 00BFFF	Video RAM
//=============================

	t9StoreBX(0xAA, 0x87F0);
	t9StoreBX(color ? 0x00 : 0x80, 0x87E2);
	t9StoreBX(0x55, 0x87F0);

	// Clear some tiles, (Cool Cool Jam in B&W)
	for (i = 0; i < 0x20; i++) {
		t9StoreBX(0, 0xA000 + i);
	}

	// Copy SNK logo to VRAM, Metal Slug 2 checks this unless run in debug mode.
	for (i = 0; i < 0x40; i++) {
		t9StoreBX(snkLogo[i], 0xA1C0 + i);
	}

	t9StoreBX(0xF3, 0x7000);	// Z80 DI
	t9StoreBX(0x31, 0x7001);	// Z80 LD SP 0x0FFF
	t9StoreBX(0xFF, 0x7002);
	t9StoreBX(0x0F, 0x7003);
	t9StoreBX(0x18, 0x7004);	// Z80 JR -2
	t9StoreBX(0xFE, 0x7005);

	// Enable sound
	t9StoreBX(0x55, 0xB9);

	// Turn on LED
	t9StoreBX(0xFF, 0x8400);

	// Do some setup for the interrupt priorities.
	BIOSHLE_Reset();
}

void fixBiosSettings(void)
{
	t9StoreBX(cfg.birthYear, 0x6F8B);
	t9StoreBX(cfg.birthMonth, 0x6F8C);
	t9StoreBX(cfg.birthDay, 0x6F8D);

	// Setup alarm from config
	t9StoreBX(cfg.alarmHour, 0x6C34);
	t9StoreBX(cfg.alarmMinute, 0x6C35);

	int check = gLang ? 0x01 : 0x00;
	// Language: 0 = Japanese, 1 = English
	t9StoreBX(check, 0x6F87);
	if (gMachine == HW_NGPCOLOR) {
		t9StoreBX(gPaletteBank, 0x6F94);
		check += gPaletteBank;
	}
	else {
		// Required by mono BIOS to skip setup.
		t9StoreBX(0xDC, 0x6C25);
	}
	check += 0xDC;		// Actualy addition of all IRQ priorities.
	t9StoreWX(check, 0x6C14);

	t9StoreBX(0x4E, 0x6E96);
	t9StoreBX(0x50, 0x6E95);
	t9StoreBX(0x00, 0x6F83);
}

//=============================================================================
