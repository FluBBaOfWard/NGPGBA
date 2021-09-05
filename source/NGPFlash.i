;@ ASM header for Flash memory used in NeoGeo Pocket carts.
;@

/** Max number of blocks on a chip */
#define MAX_BLOCKS (35)

/*
The relation between block number and flash chip is as follows:
 # |  16Mbit (2f)  |  8Mbit (2c) |  4Mbit (ab)
---+---------------+-------------+-------------
 0 | 000000-00ffff | 00000-0ffff | 00000-0ffff
 1 | 010000-01ffff | 10000-1ffff | 10000-1ffff
 2 | 020000-02ffff | 20000-2ffff | 20000-2ffff
 3 | 030000-03ffff | 30000-3ffff | 30000-3ffff
 4 | 040000-04ffff | 40000-4ffff | 40000-4ffff
 5 | 050000-05ffff | 50000-5ffff | 50000-5ffff
 6 | 060000-06ffff | 60000-6ffff | 60000-6ffff
 7 | 070000-07ffff | 70000-7ffff | 70000-77fff
 8 | 080000-08ffff | 80000-8ffff | 78000-79fff
 9 | 090000-09ffff | 90000-9ffff | 7a000-7bfff
10 | 0a0000-0affff | a0000-affff | 7c000-7ffff
11 | 0b0000-0bffff | b0000-bffff |
12 | 0c0000-0cffff | c0000-cffff |
13 | 0d0000-0dffff | d0000-dffff |
14 | 0e0000-0effff | e0000-effff |
15 | 0f0000-0fffff | f0000-f7fff |
16 | 100000-10ffff | f8000-f9fff |
17 | 110000-11ffff | fa000-fbfff |
18 | 120000-12ffff | fc000-fffff |
19 | 130000-13ffff |             |
20 | 140000-14ffff |             |
21 | 150000-15ffff |             |
22 | 160000-16ffff |             |
23 | 170000-17ffff |             |
24 | 180000-18ffff |             |
25 | 190000-19ffff |             |
26 | 1a0000-1affff |             |
27 | 1b0000-1bffff |             |
28 | 1c0000-1cffff |             |
29 | 1d0000-1dffff |             |
30 | 1e0000-1effff |             |
31 | 1f0000-1f7fff |             |
32 | 1f8000-1f9fff |             |
33 | 1fa000-1fbfff |             |
34 | 1fc000-1fffff |             |
The last block is always reserved for use by the system. The Neogeo Pocket Color
bios does some tests on this last block to see if the flash functionality is
working. It does this on every boot!
*/

//Toshiba  0x98
//Sharp    0xb0
//Samsung  0xec

// 4mbit   0xab
// 8mbit   0x2c
//16mbit   0x2f

// Flash_command
	.equ CMD_READ, 0xF0           // xxxx F0 or 5555 AA, 2AAA 55, 5555 F0
	.equ CMD_ID_READ, 0x90        // 5555 AA, 2AAA 55, 5555 90, read from block_address?
	.equ CMD_WRITE, 0xA0          // 5555 AA, 2AAA 55, 5555 A0, address data
	.equ CMD_ERASE, 0x80          // 5555 AA, 2AAA 55, 5555 80
	.equ CMD_ERASE_CHIP, 0x10     // 5555 AA, 2AAA 55, 5555 80, 5555 AA, 2AAA 55, 5555 10
	.equ CMD_ERASE_BLOCK, 0x30    // 5555 AA, 2AAA 55, 5555 80, 5555 AA, 2AAA 55, block_address 30
	.equ CMD_PROTECT, 0x9A        // 5555 AA, 2AAA 55, 5555 9A, 5555 AA, 2AAA 55, block_address 9A

	fptr		.req r12
						;@ NGPFlash.s
	.struct 0
NGPFlashState:				;@
flashMem:		.long 0		;@ Pointer to memory
vendorID:		.byte 0		;@ Flash vendor ID
sizeID:			.byte 0		;@ Flash size ID
selectedBlock:	.byte 0		;@ Currently selected block

flashState:		.byte 0
command:		.byte 0

dirtyBlocks:	.space MAX_BLOCKS
	.align 2
NGPFlashSize:

;@----------------------------------------------------------------------------

