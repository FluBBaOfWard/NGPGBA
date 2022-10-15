#ifndef NGPHEADER
#define NGPHEADER

/// NgpHeader
typedef struct
{
	const char licence[28];		// 0x00 - 0x1B
	const u32  startPC;			// 0x1C - 0x1F
	const u16  catalog;			// 0x20 - 0x21
	const u8   subCatalog;		// 0x22
	const u8   mode;			// 0x23, 0x00 = B&W, 0x10 = Color.
	const char name[12];		// 0x24 - 0x2F

	const u32  reserved1;		// 0x30 - 0x33
	const u32  reserved2;		// 0x34 - 0x37
	const u32  reserved3;		// 0x38 - 0x3B
	const u32  reserved4;		// 0x3C - 0x3F
} NgpHeader;

#endif	// NGPHEADER
