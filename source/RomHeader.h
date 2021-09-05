#ifndef NGPADVANCE_HEADER
#define NGPADVANCE_HEADER

#define COUNTRY 2					// Japan
#define SMSID 0x1A534D53			// "SMS",0x1A

typedef struct {
	u32 identifier;
	u32 filesize;
	u32 flags;
	u32 spritefollow;
	u32 reserved[4];
	char name[32];
} romheader;	

#endif			// NGPADVANCE_HEADER
