#ifndef EMUBASE
#define EMUBASE

#ifdef __cplusplus
extern "C" {
#endif

#define COUNTRY 2					// Japan
#define NGPID 0x1A50474E			// "NGP",0x1A

typedef struct {
	const u32 identifier;
	const u32 filesize;
	const u32 flags;			// Bit 2 = Color.
	const u32 spritefollow;
	const u8 bios;				// Bit 0 = Bios.
	const u8 reserved[15];
	const char name[32];
	const u8 romData[];
} RomHeader;

typedef struct {				//(config struct)
	char magic[4];				//="CFG",0
	int emuSettings;
	int unused;					// unused
	u8 gammaValue;				// from gfx.s
	u8 sprites;					// from gfx.s
	u8 config;					// from cart.s
	u8 controller;				// from io.s
	u8 alarmHour;
	u8 alarmMinute;
	u8 birthDay;
	u8 birthMonth;
	u8 birthYear;
	u8 language;
	u8 palette;
} ConfigData;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !EMUBASE
