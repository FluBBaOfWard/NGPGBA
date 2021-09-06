// Flash memory emulation, used in NeoGeo Pocket carts.

#ifdef __arm__

#include "NGPFlash.i"
#include "TLCS900H/TLCS900H.i"

	.global ngpFlashReset
	.global FlashWriteLO
	.global FlashWriteHI
	.global getSaveStartAddress

	.syntax unified
	.arm

	.section .ewram
	.align 2
;@----------------------------------------------------------------------------
ngpFlashInit:			;@ Only need to be called once
;@----------------------------------------------------------------------------

	bx lr
;@----------------------------------------------------------------------------
ngpFlashReset:			;@ r0=flash size in bytes, r1 = flash mem ptr, r12=fptr
;@----------------------------------------------------------------------------
	stmfd sp!,{r0-r3,lr}
	str r1,flashMemory
	mov r1,#0x2F			;@ flashId 16Mbit
	mov r2,#0x1F			;@ sizeMask 16Mbit
	mov r3,#34				;@ Last block
//	cmp r0,#0x400000
//	cmpne r0,#0x200000
	cmp r0,#0x100000
	moveq r1,#0x2C			;@ flashId 8Mbit
	moveq r2,#0x0F			;@ sizeMask 8Mbit
	moveq r3,#18			;@ Last block
	cmp r0,#0x80000
	moveq r1,#0xAB			;@ flashId 4Mbit
	moveq r2,#0x07			;@ sizeMask 4Mbit
	moveq r3,#10			;@ Last block
	str r0,flashSize
	strb r1,flashSizeId
	strb r2,flashSizeMask
	strb r3,lastBlock

	ldmfd sp!,{r0-r3,lr}
	bx lr

;@----------------------------------------------------------------------------
ngpFlashSaveState:		;@ In r0=destination, r1=fptr. Out r0=state size.
	.type   ngpFlashSaveState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,r5,lr}
	mov r4,r0				;@ Store destination
	mov r5,r1				;@ Store fptr (r1)

	ldmfd sp!,{r4,r5,lr}
	ldr r0,=0x14
	bx lr
;@----------------------------------------------------------------------------
ngpFlashLoadState:		;@ In r0=fptr, r1=source. Out r0=state size.
	.type   ngpFlashLoadState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,r5,lr}
	mov r5,r0				;@ Store fptr (r0)
	mov r4,r1				;@ Store source

	mov fptr,r5
	ldmfd sp!,{r4,r5,lr}
;@----------------------------------------------------------------------------
ngpFlashGetStateSize:	;@ Out r0=state size.
	.type   ngpFlashGetStateSize STT_FUNC
;@----------------------------------------------------------------------------
	ldr r0,=0x14
	bx lr


;@----------------------------------------------------------------------------
getSaveStartAddress:
	.type   getSaveStartAddress STT_FUNC
;@----------------------------------------------------------------------------
	ldrb r3,lastBlock
	mov r0,#0
saveSLoop:
	ldr r2,=flashBlocks
	ldrb r1,[r2,r0]
	tst r1,#0x81			;@ Modified or needs saving.
	bne saveFound
	add r0,r0,#1
	cmp r0,r3
	ble saveSLoop
	mov r0,#0
	bx lr
saveFound:
	b getAddressFromBlock
;@----------------------------------------------------------------------------
getAddressFromBlock:	;@ In r0=blockNr, out r0=address
;@----------------------------------------------------------------------------
	ldrb r2,flashSizeMask
	subs r1,r0,r2			;@ Over the last 64kB block?
	mov r0,r0,lsl#16
	ble adrDone
	add r0,r0,#0x8000
	cmp r1,#2
	addeq r0,r0,#0x2000
	cmp r1,#3
	addeq r0,r0,#0x4000
adrDone:
	bx lr
;@----------------------------------------------------------------------------
markBlockModifiedFromAddress:	;@ In r0=address, out r0=address
;@----------------------------------------------------------------------------
	stmfd sp!,{r0-r2,lr}
	bl getBlockFromAddress
	ldr r2,=flashBlocks
	ldrb r1,[r2,r0]
	orr r1,r1,#0x81			;@ Both modified and needs saving.
	strb r1,[r2,r0]
	ldmfd sp!,{r0-r2,pc}
;@----------------------------------------------------------------------------
getBlockFromAddress:	;@ In r0=address, out r0=blockNr
;@----------------------------------------------------------------------------
	ldrb r2,flashSizeMask
	and r1,r2,r0,lsr#16
	cmp r1,r2				;@ Is it the last 64kB block?
	bne blockDone
	tst r0,0x8000
	beq blockDone
	addne r1,r1,#1
	tst r0,0x4000
	addne r1,r1,#2
	bne blockDone
	tst r0,0x2000
	addne r1,r1,#1
blockDone:
	mov r0,r1
	bx lr
;@----------------------------------------------------------------------------
getBlockInfoFromAddress:	;@ In r0=address, out r0=start adr, r1=size
;@----------------------------------------------------------------------------
	mov r1,#0x10000			;@ Block size
	ldrb r3,flashSizeMask
	and r2,r0,r3,lsl#16
	cmp r2,r3,lsl#16		;@ Is it the last 64kB block?
	bne blockInfDone
	mov r1,#0x8000			;@ Block size
	tst r0,0x8000
	beq blockInfDone
	addne r2,r2,#0x8000
	tst r0,0x4000
	addne r2,r2,#0x4000
	movne r1,#0x4000		;@ Block size
	bne blockInfDone
	mov r1,#0x2000			;@ Block size
	tst r0,0x2000
	addne r2,r2,#0x2000
blockInfDone:
	mov r0,r2
	bx lr
;@----------------------------------------------------------------------------
ReadFlashInfo:			;@ In r0=address.
;@----------------------------------------------------------------------------
	mov r11,r11
	ands r1,r0,#0x03
	moveq r0,#0x98			;@ 0x00 Manufacturer, 98 = Toshiba, EC = Samsung, B0 = Sharp.
	bxeq lr
	cmp r1,#2
	ldrbmi r0,flashSizeId	;@ 0x01 Size, AB = 4Mbit, 2C = 8Mbit, 2F = 16Mbit
	moveq r0,#0x02			;@ 0x02 Block not protected
	movhi r0,#0x80			;@ 0x03 ??? 0x80
	bx lr
;@----------------------------------------------------------------------------
FlashWriteLO:			;@ In r0=value, r1=address.
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r5,lr}
	and r0,r0,#0xFF
	bic r4,r1,#0xFF0000
	ldrb r2,currentWriteCycle
	and r2,r2,#0x07
	ldr pc,[pc,r2,lsl#2]
	.long 0
	.long FlashProg1
	.long FlashProg2
	.long FlashCommand
	.long FlashProg1
	.long FlashProg2
	.long FlashCommand2
	.long FlashWrite
	.long FlashCycEnd

FlashProg1:				;@ F_READ
	cmp r0,#CMD_READ
	beq FlashSetRead
	ldr r3,=0x5555
	cmp r4,r3
	cmpeq r0,#0xAA
	addeq r2,r2,#1
	b FlashCycEnd

FlashProg2:				;@ F_PROG
	ldr r3,=0x2AAA
	cmp r4,r3
	cmpeq r0,#0x55
	addeq r2,r2,#1
	movne r2,#0
	b FlashCycEnd

FlashCommand:			;@ F_COMMAND
	ldr r3,=0x5555
	cmp r4,r3
	bne FlashSetRead	;@ Or just end?
	strb r0,currentCommand

	cmp r0,#CMD_READ
	beq FlashSetRead

	cmp r0,#CMD_WRITE
	moveq r2,#6
	beq FlashCycEnd

	cmp r0,#CMD_ID_READ
	ldreq r1,=ReadFlashInfo
	streq r1,[t9optbl,#readRomPtrLo]
	moveq r2,#0
	beq FlashCycEnd

	cmp r0,#CMD_ERASE
	cmpne r0,#CMD_PROTECT
	addeq r2,r2,#1
	b FlashCycEnd

FlashCommand2:			;@ F_COMMAND2
	ldrb r2,currentCommand
	cmp r2,#CMD_ERASE
	beq FlashErase
	cmp r2,#CMD_PROTECT
	beq FlashProtect
	b FlashSetRead

FlashErase:
	cmp r0,#CMD_ERASE_BLOCK
	bne FlashEraseChip
FlashEraseBlock:
	mov r0,r1
	bl markBlockModifiedFromAddress
	bl getBlockInfoFromAddress
	ldr r2,flashMemory
	add r0,r0,r2
	mov r2,r1			;@ Length
	mov r1,#-1
	bl memset_
	b FlashSetRead

FlashEraseChip:
	ldr r3,=0x5555
	cmp r4,r3
	bne FlashSetRead	;@ Or just end?
	mov r11,r11
	b FlashCycEnd

;@----------------------------------------------------------------------------
FlashProtect:
;@----------------------------------------------------------------------------
	mov r0,r1
	bl getBlockFromAddress
	ldr r3,=flashBlocks
	ldrb r1,[r3,r0]
	orr r1,r1,#0x02			;@ Protect bit
	strb r1,[r3,r0]
	b FlashCycEnd

;@----------------------------------------------------------------------------
FlashWrite:				;@ F_ID_READ
;@----------------------------------------------------------------------------
	mov r4,r0
	mov r0,r1
	bl markBlockModifiedFromAddress
	bic r1,r1,#0xFE00000
	ldr r2,flashMemory
	ldrb r3,[r2,r1]
	and r3,r3,r4
	strb r3,[r2,r1]

FlashSetRead:
	mov r0,#CMD_READ
	strb r0,currentCommand
	ldr r1,=tlcs_rom_R
	str r1,[t9optbl,#readRomPtrLo]
	mov r2,#0
FlashCycEnd:
	strb r2,currentWriteCycle
	ldmfd sp!,{r4-r5,lr}
	bx lr
;@----------------------------------------------------------------------------
FlashWriteHI:
;@----------------------------------------------------------------------------
	bx lr

flashMemory:
	.long 0
flashSize:
	.long 0
flashSizeMask:
	.byte 0
flashSizeId:
	.byte 0
currentWriteCycle:
	.byte 0
currentCommand:
	.byte 0
lastBlock:
	.byte 0

flashBlocks:	;@ Bit 0=modified, bit 1=protected, bit 7=should be saved.
	.space MAX_BLOCKS
// Padding
	.align 2

;@----------------------------------------------------------------------------
#endif // #ifdef __arm__
