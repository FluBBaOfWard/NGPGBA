
#include <gba.h>

#include "io.h"
#include "cpu.h"
#include "Memory.h"
#include "NGPFlash/NGPFlash.h"

#define rCodeB(r)	(*((u8 *)getRegAdr(r)))
#define rCodeW(r)	(*((u16 *)getRegAdr((r) & ~1)))
#define rCodeL(r)	(*((u32 *)getRegAdr((r) & ~3)))

/**
 *
 */
void iBIOSHLE(int vector) {
	switch (vector) {

	// VECT_SHUTDOWN (0xFF27A2)
	case 0x00:
		return;

	// VECT_CLOCKGEARSET (0xFF1034)
	case 0x01:
		t9StoreBX(rCodeL(0x3C), 0x80);
		break;

	// VECT_RTCGET (0xFF1440)
	case 0x02:
	{
		// Copy time from hardware area
		u32 adr = rCodeL(0x3C);
		t9StoreBX(t9LoadBX(0x91), adr + 0);
		t9StoreWX(t9LoadWX(0x92), adr + 1);
		t9StoreWX(t9LoadWX(0x94), adr + 3);
		t9StoreWX(t9LoadWX(0x96), adr + 5);
		break;
	}
	// VECT_RTCSET (0xFF12B4)
	//case 0x03:
	//	break;

	// VECT_INTLVSET (0xFF1222)
	case 0x04:
	{

	u8 level = rCodeB(0x35) & 0x07; // RB
	u8 interrupt = rCodeB(0x34);	// RC

	switch(interrupt) {
		case 0x00:	// Interrupt from RTC alarm
			level |= t9LoadBX(0x6C24) & 0xf0;
			t9StoreBX(level, 0x6C24);
			t9StoreBX(level, 0x70);
			break;
		case 0x01:	// Interrupt from the Z80 CPU
			level = (level<<4) | (t9LoadBX(0x6C25) & 0x0f);
			t9StoreBX(level, 0x6C25);
			t9StoreBX(level, 0x71);
			break;
		case 0x02:	// Interrupt from Timer 0
			level |= t9LoadBX(0x6C27) & 0xf0;
			t9StoreBX(level, 0x6C27);
			t9StoreBX(level, 0x73);
			break;
		case 0x03:	// Interrupt from Timer 1
			level = (level<<4) | (t9LoadBX(0x6C27) & 0x0f);
			t9StoreBX(level, 0x6C27);
			t9StoreBX(level, 0x73);
			break;
		case 0x04:	// Interrupt from Timer 2
			level |= t9LoadBX(0x6C28) & 0xf0;
			t9StoreBX(level, 0x6C28);
			t9StoreBX(level, 0x74);
			break;
		case 0x05:	// Interrupt from Timer 3
			level = (level<<4) | (t9LoadBX(0x6C28) & 0x0f);
			t9StoreBX(level, 0x6C28);
			t9StoreBX(level, 0x74);
			break;
		case 0x06:	// End of transfer interrupt from DMA channel 0
			level |= t9LoadBX(0x6C2A) & 0xf0;
			t9StoreBX(level, 0x6C2A);
			t9StoreBX(level, 0x79);
			break;
		case 0x07:	// End of transfer interrupt from DMA channel 1
			level = (level<<4) | (t9LoadBX(0x6C2A) & 0x0f);
			t9StoreBX(level, 0x6C2A);
			t9StoreBX(level, 0x79);
			break;
		case 0x08:	// End of transfer interrupt from DMA channel 2
			level |= (t9LoadBX(0x6C2B) & 0xf0);
			t9StoreBX(level, 0x6C2B);
			t9StoreBX(level, 0x7A);
			break;
		case 0x09:	// End of transfer interrupt from DMA channel 3
			level = (level<<4) | (t9LoadBX(0x6C2B) & 0x0f);
			t9StoreBX(level, 0x6C2B);
			t9StoreBX(level, 0x7A);
			break;
		default:
			break;
	}
	}
		break;

	// VECT_SYSFONTSET (0xFF8D8A)
	case 0x05:
	{
		int j, i, dst = 0xA000;

		u8 a = rCodeB(0x30);
		u8 b = a >> 4;
		a &= 3;

		for (i = 0; i < 0x800; i++) {
			u8 c = t9LoadBX(0xFF8DCF + i);

			u16 data16 = t9LoadWX(dst);
			for (j = 0; j < 8; j++, c <<= 1) {
				data16 <<= 2;
				data16 |= (c & 0x80)?a:b;
			}
			t9StoreWX(data16, dst);
			dst += 2;
		}
	}
		break;

	// VECT_FLASHWRITE (0xFF6FD8)
	case 0x06:
	{
		int i, bank = 0x200000;

		// Select HI rom
		if (rCodeB(0x30) == 1) {
			bank = 0x800000;
		}
		int destination = rCodeL(0x38) + bank;
		int source = rCodeL(0x3C) + bank;

		int count = rCodeW(0x34) * 256;
		for (i = 0; i < count; i++) {
			t9StoreBX(0xAA, bank + 0x5555);
			t9StoreBX(0x55, bank + 0x2AAA);
			t9StoreBX(0xA0, bank + 0x5555);
			t9StoreBX(t9LoadBX(source + i), destination + i);
		}
		int block = getBlockFromAddress(destination);
		markBlockDirty(rCodeB(0x30), block);

		rCodeB(0x30) = 0;		// RA3 = SYS_SUCCESS
	}
		break;

	// VECT_FLASHALLERS (0xFF7042)
	case 0x07:
		// TODO
		rCodeB(0x30) = 0;	// RA3 = SYS_SUCCESS
		break;
	// VECT_FLASHERS (0xFF7082)
	case 0x08:
	{
		u32 address = 0x200000;
		// Select HI rom?
		if (rCodeB(0x30) == 1) {
			address = 0x800000;
		}
		address += getBlockOffset(rCodeB(0x34));

		t9StoreBX(0xAA, address + 0x5555);
		t9StoreBX(0x55, address + 0x2AAA);
		t9StoreBX(0x80, address + 0x5555);
		t9StoreBX(0xAA, address + 0x5555);
		t9StoreBX(0x55, address + 0x2AAA);
		rCodeB(0x30) = 0;	// RA3 = SYS_SUCCESS
	}
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
		t9StoreBX(0xAA, 0x87F0);		// Allow GE MODE registers to be written to
		if (rCodeB(0x30) < 0x10) {	// Current A register.
			// set B/W mode
			t9StoreBX(0x80, 0x87E2);
			t9StoreBX(0x00, 0x6F95);
		}
		else {
			// set color mode
			t9StoreBX(0x00, 0x87E2);
			t9StoreBX(0x10, 0x6F95);
		}
		t9StoreBX(0x55, 0x87F0);		// Disallow GE MODE registers to be written to
		break;

	// Reserved (just RET) (0xFF1032)
	//case 0x0F: break;

	// VECT_COMINIT (0xFF2BBD)
	case 0x10:
		// Nothing to do.
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
		rCodeB(0x30) = 0; // RA3 = COM_BUF_OK
	}
		setInterruptExternal(0x18);
		break;

	// VECT_COMGETDATA (0xFF2CB4)
	case 0x14:
	{
		u8 data;
		if (system_comms_read(&data)) {
			rCodeB(0x30) = 0; // COM_BUF_OK
			rCodeB(0x35) = data;
			// Comms. Read interrupt
			t9StoreBX(data, 0x50);

			setInterruptExternal(0x19);
			return;
		}
		else {
			rCodeB(0x30) = 1; // COM_BUF_EMPTY
		}
	}
		break;
	
	// VECT_COMONRTS (0xFF2D27)
	case 0x15:
		t9StoreBX(t9LoadBX(0xB2) & 0xFE, 0xB2);
		break;
	
	// VECT_COMOFFRTS (0xFF2D33)
	case 0x16:
		t9StoreBX(t9LoadBX(0xB2) | 0x01, 0xB2);
		break;

	// VECT_COMSENDSTATUS (0xFF2D3A)
	case 0x17:
		// Nothing to do.
		rCodeW(0x30) = 0; // Send Buffer Count, never any pending data!
		break;

	// VECT_COMRECEIVESTATUS (0xFF2D4E)
	case 0x18:
		// Receive Buffer Count
		rCodeW(0x30) = system_comms_read(NULL);
		break;

	// VECT_COMCREATEBUFDATA (0xFF2D6C)
	case 0x19:
	{
		u8 count = rCodeB(0x35);
		u32 index = rCodeL(0x3C);
		while(count > 0) {
			u8 data = t9LoadBX(index);
			// Write data from (XHL3++)
			system_comms_write(data);
			index++; // Next data
			count--; // RB3 = Count Left
		}
		rCodeL(0x3C) = index;
		rCodeB(0x35) = count;
		setInterruptExternal(0x18);
	}
		break;

	// VECT_COMGETBUFDATA (0xFF2D85)
	case 0x1A:
	{
		u8 count = rCodeB(0x35);
		u32 index = rCodeL(0x3C);
		while(count > 0) {
			u8 data;
			if (system_comms_read(&data)) {
				// Read data into (XHL3++)
				t9StoreBX(data, index);
				index++; // Next data
				count--; // RB3 = Count Left

				// Comms. Read interrupt
				t9StoreBX(data, 0x50);
				setInterruptExternal(0x19);
			}
			else {
				break;
			}
		}
		rCodeL(0x3C) = index;
		rCodeB(0x35) = count;
	}
		break;
}

}
