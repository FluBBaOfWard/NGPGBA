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

#include "io.h"
#include "bios.h"
#include "TLCS900H/TLCS900H_registers.h"
#include "TLCS900H/TLCS900H_int.h"
#include "Memory.h"

const u8 IndexConv[0x21] = {0,0,0,0,1,2,3,0,0,0,4,5,6,0,0,0, 7,8,9,10,0,0,0,0,11,12,0,0,0,14,15,16, 17};

static u8 CacheIntPrio[0xB]; // Interrupt prio registers at 0x0070-0x007a don't have priority readable.
	 		       // This should probably be stored in BIOS work RAM somewhere instead of a separate array! @6C24-6C2B


void BIOSHLE_Reset(void)
{
	int x;
	memset(CacheIntPrio, 0, sizeof(CacheIntPrio));
	CacheIntPrio[0] = 0x02;
	CacheIntPrio[1] = 0x32;

	for (x = 0; x < 0xB; x++) {
		t9StoreB(CacheIntPrio[x], 0x70 + x);
	}
}

/* This is the custom HLE instruction. I considered it was the fastest and
most streamlined way of intercepting a bios call. The operation performed
is dependant on the current program counter. */

void iBIOSHLE(int vector)
{
//	u32 a;
	switch (vector)
	{	

	// VECT_SHUTDOWN (0xFF27A2)
	case 0x00:
		// Cheap bit of code to stop the message appearing repeatedly.
//		a = pop32();
//		if (a != 0xBAADC0DE){
			//MDFN_printf("IDS_POWER");
//		}
//		push32(0xBAADC0DE); // Sure is!
		return;	// Don't pop a return address, stay here

	// VECT_CLOCKGEARSET (0xFF1034)
	case 0x01:
		t9StoreB(rCodeL(0x3C), 0x80);
		break;

	// VECT_RTCGET (0xFF1440)
	case 0x02:
		if (rCodeL(0x3C) < 0xC000)
		{
			// Copy data from hardware area
			t9StoreB(t9LoadB(0x91), rCodeL(0x3C) + 0);
			t9StoreW(t9LoadW(0x92), rCodeL(0x3C) + 1);
			t9StoreW(t9LoadW(0x94), rCodeL(0x3C) + 3);
			t9StoreW(t9LoadW(0x96), rCodeL(0x3C) + 5);
		}
		break; 

	// VECT_RTCSET (0xFF12B4)
	//case 0x03:
	//	break;

	// VECT_INTLVSET (0xFF1222)
	case 0x04:
	{

	u8 level = rCodeB(0x35) & 0x07; // RB3
	u8 interrupt = rCodeB(0x34);	// RC3

	switch(interrupt)
	{
		case 0x00:	// Interrupt from RTC alarm
			CacheIntPrio[0x0] = (CacheIntPrio[0x0] & 0xf0) |  level;
			t9StoreB(CacheIntPrio[0x0], 0x70);
			break;
		case 0x01:	// Interrupt from the Z80 CPU
			CacheIntPrio[0x1] = (CacheIntPrio[0x1] & 0x0f) | (level<<4);
			t9StoreB(CacheIntPrio[0x1], 0x71);
			break;
		case 0x02:	// Interrupt from the 8 bit timer 0
			CacheIntPrio[0x3] = (CacheIntPrio[0x3] & 0xf0) |  level;
			t9StoreB(CacheIntPrio[0x3], 0x73);
			break;
		case 0x03:	// Interrupt from the 8 bit timer 1
			CacheIntPrio[0x3] = (CacheIntPrio[0x3] & 0x0f) | (level<<4);
			t9StoreB(CacheIntPrio[0x3], 0x73);
			break;
		case 0x04:	// Interrupt from the 8 bit timer 2
			CacheIntPrio[0x4] = (CacheIntPrio[0x4] & 0xf0) |  level;
			t9StoreB(CacheIntPrio[0x4], 0x74);
			break;
		case 0x05:	// Interrupt from the 8 bit timer 3
			CacheIntPrio[0x4] = (CacheIntPrio[0x4] & 0x0f) | (level<<4);
			t9StoreB(CacheIntPrio[0x4], 0x74);
			break;
		case 0x06:	// End of transfer interrupt from DMA channel 0
			CacheIntPrio[0x9] = (CacheIntPrio[0x9] & 0xf0) |  level;
			t9StoreB(CacheIntPrio[0x9], 0x79);
			break;
		case 0x07:	// End of transfer interrupt from DMA channel 1
			CacheIntPrio[0x9] = (CacheIntPrio[0x9] & 0x0f) | (level<<4);
			t9StoreB(CacheIntPrio[0x9], 0x79);
			break;
		case 0x08:	// End of transfer interrupt from DMA channel 2
			CacheIntPrio[0xa] = (CacheIntPrio[0xa] & 0xf0) |  level;
			t9StoreB(CacheIntPrio[0xa], 0x7a);
			break;
		case 0x09:	// End of transfer interrupt from DMA channel 3
			CacheIntPrio[0xa] = (CacheIntPrio[0xa] & 0x0f) | (level<<4);
			t9StoreB(CacheIntPrio[0xa], 0x7a);
			break;
		default:
			//puts("DOH");
			break;
	}
	}
		break;

	// VECT_SYSFONTSET (0xFF8D8A)
	case 0x05:
	{
		u8 a,b,c, j;
		u16 i, dst = 0xA000;

		b = rCodeB(0x30) >> 4;
		a = rCodeB(0x30) & 3;

		for (i = 0; i < 0x800; i++) {
			c = ngpc_bios[0x8DCF + i];

			for (j = 0; j < 8; j++, c<<=1) {
				u16 data16;

				data16 = t9LoadW(dst);
				data16 <<= 2;
				t9StoreW(data16, dst);

				if (c & 0x80)	t9StoreB(t9LoadB(dst) | a, dst);
				else			t9StoreB(t9LoadB(dst) | b, dst);
			}
			dst += 2;
		}
	}
		break;

	// VECT_FLASHWRITE (0xFF6FD8)
	case 0x06:
	{
#ifdef FLASH_ENABLED
		u32 i, address, bank = 0x200000;

		// Select HI rom?
		if (rCodeB(0x30) == 1) {
			bank = 0x800000;
		}
		address = rCodeL(0x38) + bank;

		memory_flash_error = FALSE;
		// Copy as 32 bit values for speed
		for (i = 0; i < rCodeW(0x34) * 64ul; i++) {
			t9StoreL(t9LoadL(rCodeL(0x3C) + (i * 4)), address + (i * 4));
		}

		if (memory_flash_error) {
			rCodeB(0x30) = 0xFF;	// RA3 = SYS_FAILURE
		}
		else {
			// Save this data to an external file
			flash_write(address, rCodeW(0x34) * 256);
			rCodeB(0x30) = 0;		// RA3 = SYS_SUCCESS
		}
#endif
	}
		break;

	// VECT_FLASHALLERS (0xFF7042)
	case 0x07:
		// TODO
		rCodeB(0x30) = 0;	// RA3 = SYS_SUCCESS
		break;
	// VECT_FLASHERS (0xFF7082)
	case 0x08:
		// TODO
		rCodeB(0x30) = 0;	// RA3 = SYS_SUCCESS
		break;

	// VECT_ALARMSET (0xFF149B)
	case 0x09:
		// TODO
		rCodeB(0x30) = 0;	// RA3 = SYS_SUCCESS
		break;

	// Reserved (just RET) (0xFF1033)
	//case 0x0A: break;

	// VECT_ALARMDOWNSET (0xFF1487)
	case 0x0B:
		// TODO
		rCodeB(0x30) = 0;	// RA3 = SYS_SUCCESS
		break;

	// ? (0xFF731F)
	//case 0x0C: break;

	// VECT_FLASHPROTECT (0xFF70CA)
	case 0x0D:
		// TODO
		rCodeB(0x30) = 0;	// RA3 = SYS_SUCCESS
		break;

	// VECT_GEMODESET (0xFF17C4)
	case 0x0E:
		t9StoreB(0xAA, 0x87F0);		// Allow GE MODE registers to be written to
		if (rCodeB(0x30) < 0x10) {	// Current A register.
			// set B/W mode
			t9StoreB(0x80, 0x87E2);
			t9StoreB(0x00, 0x6F95);
		}
		else {
			// set color mode
			t9StoreB(0x00, 0x87E2);
			t9StoreB(0x10, 0x6F95);
		}
		t9StoreB(0x55, 0x87F0);		// Disallow GE MODE registers to be written to
		break;

	// Reserved (just RET) (0xFF1032)
	//case 0x0F: break;

	// VECT_COMINIT (0xFF2BBD)
	case 0x10:
		// Nothing to do.
		rCodeB(0x30) = 0;	// RA3 = COM_BUF_OK
		break;

	// VECT_COMSENDSTART (0xFF2C0C)
	case 0x11:
		// Nothing to do.
		break;

	// VECT_COMRECEIVESTART (0xFF2C44)
	case 0x12:
		// Nothing to do.
		break;

	// VECT_COMCREATEDATA (0xFF2C86)
	case 0x13:
	{
		// Write the byte
		u8 data = rCodeB(0x35);
		system_comms_write(data);
		// Always COM_BUF_OK because the write call always succeeds.
		rCodeB(0x30) = 0x0;			// RA3 = COM_BUF_OK
	}

		// Restore $PC after BIOS-HLE instruction
//		tlcs_pc = pop32();
		setInterruptExternal(0x18);
		return;

	// VECT_COMGETDATA (0xFF2CB4)
	case 0x14:
	{
		char data;

		if (system_comms_read(&data)) {
			rCodeB(0x30) = 0;	// COM_BUF_OK
			rCodeB(0x35) = data;
			// Comms. Read interrupt
			t9StoreB(data, 0x50);

//			tlcs_pc = pop32();
			setInterruptExternal(0x19);
			return;
		}
		else {
			rCodeB(0x30) = 1;	// COM_BUF_EMPTY
		}
	}
		break;
	
	// VECT_COMONRTS (0xFF2D27)
	case 0x15:
		t9StoreB(0, 0xB2);
		break;
	
	// VECT_COMOFFRTS (0xFF2D33)
	case 0x16:
		t9StoreB(1, 0xB2);
		break;	

	// VECT_COMSENDSTATUS (0xFF2D3A)
	case 0x17:
		// Nothing to do.
		rCodeW(0x30) = 0;	// Send Buffer Count, never any pending data!
		break;

	// VECT_COMRECEIVESTATUS (0xFF2D4E)
	case 0x18:
		// Receive Buffer Count
		rCodeW(0x30) = system_comms_read(NULL);
		break;

	// VECT_COMCREATEBUFDATA (0xFF2D6C)
	case 0x19:
		while(rCodeB(0x35) > 0)
		{
			u8 data;
			data = t9LoadB(rCodeL(0x3C));

			// Write data from (XHL3++)
			system_comms_write(data);
			rCodeL(0x3C)++; // Next data
			rCodeB(0x35)--;	// RB3 = Count Left
		}
//		tlcs_pc = pop32();
		setInterruptExternal(0x18);
		return;

	// VECT_COMGETBUFDATA (0xFF2D85)
	case 0x1A:
		while(rCodeB(0x35) > 0) {
			char data;
			if (system_comms_read(&data)) {
				// Read data into (XHL3++)
				t9StoreB(data, rCodeL(0x3C));
				rCodeL(0x3C)++; // Next data
				rCodeB(0x35)--;	// RB3 = Count Left

				// Comms. Read interrupt
				t9StoreB(data, 0x50);
//				tlcs_pc = pop32();
				setInterruptExternal(0x19);
				return;
			}
			else {
				break;
			}
		}
		break;
	}

}
