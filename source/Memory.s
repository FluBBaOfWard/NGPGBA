#ifdef __arm__

#include "TLCS900H/TLCS900H_mac.h"
#include "ARMZ80/ARMZ80.i"

	.global empty_IO_R
	.global empty_IO_W
	.global empty_R
	.global empty_W
	.global rom_W
	.global t9StoreBX
	.global t9StoreWX
	.global t9StoreLX
	.global t9LoadBX
	.global t9LoadWX
	.global t9LoadLX
	.global t9StoreB
	.global t9StoreB_mem
	.global t9StoreW_mem
	.global t9StoreL_mem
	.global t9LoadB
	.global t9LoadB_mem
	.global t9LoadW
	.global t9LoadW_mem
	.global t9LoadL
	.global t9LoadL_mem
	.global tlcs_rom_R
	.global tlcs_romH_R
	.global z80RamW
	.global z80SoundW
	.global z80IrqW
	.global z80RamR


	.syntax unified
	.arm

	.section .text
	.align 2
;@----------------------------------------------------------------------------
empty_IO_R:					;@ Read bad IO address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0x10
	bx lr
;@----------------------------------------------------------------------------
empty_IO_W:					;@ Write bad IO address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0x18
	bx lr
;@----------------------------------------------------------------------------
empty_R:					;@ Read bad address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0
	bx lr
;@----------------------------------------------------------------------------
empty_W:					;@ Write bad address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0xBA
	bx lr
;@----------------------------------------------------------------------------
rom_W:						;@ Write ROM address (error)
;@----------------------------------------------------------------------------
	mov r11,r11					;@ No$GBA breakpoint
	mov r0,#0xB0
	bx lr
;@----------------------------------------------------------------------------
t9StoreBX:					;@ r0=value, r1=address
	.type	t9StoreBX STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{t9ptr,lr}
	ldr t9ptr,=tlcs900HState
	bl t9StoreB
	ldmfd sp!,{t9ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
t9StoreWX:					;@ r0=value, r1=address
	.type	t9StoreWX STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{t9Mem,t9ptr,lr}
	ldr t9ptr,=tlcs900HState
	mov t9Mem,r1
	bl t9StoreW_mem
	ldmfd sp!,{t9Mem,t9ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
t9StoreLX:					;@ r0=value, r1=address
	.type	t9StoreLX STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{t9Mem,t9ptr,lr}
	ldr t9ptr,=tlcs900HState
	mov t9Mem,r1
	bl t9StoreL_mem
	ldmfd sp!,{t9Mem,t9ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
t9LoadBX:					;@ r0=address
	.type	t9LoadBX STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{t9ptr,lr}
	ldr t9ptr,=tlcs900HState
	bl t9LoadB
	ldmfd sp!,{t9ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
t9LoadWX:					;@ r0=address
	.type	t9LoadWX STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{t9ptr,lr}
	ldr t9ptr,=tlcs900HState
	bl t9LoadW
	ldmfd sp!,{t9ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
t9LoadLX:					;@ r0=address
	.type	t9LoadLX STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{t9ptr,lr}
	ldr t9ptr,=tlcs900HState
	bl t9LoadL
	ldmfd sp!,{t9ptr,lr}
	bx lr
;@----------------------------------------------------------------------------

#ifdef NDS
	.section .itcm						;@ For the NDS ARM9
#elif GBA
	.section .iwram, "ax", %progbits	;@ For the GBA
#endif
	.align 2
;@----------------------------------------------------------------------------
t9StoreB_mem:
;@----------------------------------------------------------------------------
	bic r1,t9Mem,#0xFF000000
;@----------------------------------------------------------------------------
t9StoreB:					;@ r0=value, r1=address
;@----------------------------------------------------------------------------
	movs r2,r1,lsr#8
	beq t9StoreB_Low
	movs r2,r1,lsr#14
	cmp r2,#1
	beq t9StoreB_ram
	cmp r2,#2
	beq t9StoreB_vram
	mov r2,r2,lsr#7
	cmp r2,#1
	beq FlashWriteLO
	cmp r2,#4
	beq FlashWriteHI
	b rom_W
;@----------------------------------------------------------------------------
t9StoreW_mem:
;@----------------------------------------------------------------------------
	bic r1,t9Mem,#0xFF000000
;@----------------------------------------------------------------------------
t9StoreW:					;@ r0=value, r1=address
;@----------------------------------------------------------------------------
	tst r1,#1
	bne t9StoreUnevenW
	mov r2,r1,lsr#14
	cmp r2,#1
	beq t9StoreW_ram
	cmp r2,#2
	beq t9StoreW_vram

;@----------------------------------------------------------------------------
t9StoreUnevenW:
;@----------------------------------------------------------------------------
	t9eatcycles 1
	stmfd sp!,{r0,r1,lr}
	bl t9StoreB
	ldmfd sp!,{r0,r1,lr}
	mov r0,r0,lsr#8
	add r1,r1,#1
	b t9StoreB
;@----------------------------------------------------------------------------
t9StoreL_mem:
;@----------------------------------------------------------------------------
	bic r1,t9Mem,#0xFF000000
;@----------------------------------------------------------------------------
t9StoreL:					;@ r0=value, r1=address
;@----------------------------------------------------------------------------
	stmfd sp!,{r0,r1,lr}
	bl t9StoreW
	ldmfd sp!,{r0,r1,lr}
	mov r0,r0,lsr#16
	add r1,r1,#2
	b t9StoreW
;@----------------------------------------------------------------------------
t9StoreW_ram:				;@ Write RAM (0x004000-0x007FFF)
;@----------------------------------------------------------------------------
	ldr r2,=ngpRAM-0x4000
	strh r0,[r2,r1]
	bx lr
;@----------------------------------------------------------------------------
t9StoreB_ram:				;@ Write RAM (0x004000-0x007FFF)
;@----------------------------------------------------------------------------
	ldr r2,=ngpRAM-0x4000
	strb r0,[r2,r1]
	bx lr
;@----------------------------------------------------------------------------
t9StoreW_vram:				;@ Write VRAM (0x009000-0x00BFFF)
;@----------------------------------------------------------------------------
	tst r1,#0x007000
	beq k2GE_0W_W
	ldr r2,=k2geRAM-0x9000
	strh r0,[r2,r1]
	ldr r2,=DIRTYTILES-0x900
	mov r0,#0
	strb r0,[r2,r1,lsr#4]		;@ Each dirty byte is 0x10 VRAM bytes
	bx lr
;@----------------------------------------------------------------------------
t9StoreB_vram:				;@ Write VRAM (0x009000-0x00BFFF)
;@----------------------------------------------------------------------------
	tst r1,#0x007000
	beq k2GE_0W
	ldr r2,=k2geRAM-0x9000
	strb r0,[r2,r1]
	ldr r2,=DIRTYTILES-0x900
	mov r0,#0
	strb r0,[r2,r1,lsr#4]		;@ Each dirty byte is 0x10 VRAM bytes
	bx lr
;@----------------------------------------------------------------------------
t9LoadB_mem:
;@----------------------------------------------------------------------------
	bic r0,t9Mem,#0xFF000000
;@----------------------------------------------------------------------------
t9LoadB:					;@ r0=address
;@----------------------------------------------------------------------------
	movs r2,r0,lsr#8
	beq t9LoadB_Low
	movs r2,r0,lsr#14
	cmp r2,#1
	beq tlcs_ram_R
	bmi ram_low_R
	cmp r2,#2
	beq tlcs_vram_R
	mov r2,r2,lsr#7
	cmp r2,#1
	ldreq pc,[t9ptr,#readRomPtrLo]
	cmp r2,#4
	ldreq pc,[t9ptr,#readRomPtrHi]
	cmp r2,#7
	beq tlcs_bios_R
ram_low_R:
	mov r11,r11
	mov r0,#0
	mov r1,#0
	bx lr
;@----------------------------------------------------------------------------
t9LoadW_mem:
;@----------------------------------------------------------------------------
	bic r0,t9Mem,#0xFF000000
;@----------------------------------------------------------------------------
t9LoadW:					;@ r0=address
;@----------------------------------------------------------------------------
	tst r0,#1
	bne t9LoadUnevenW
t9LoadEvenW:				;@ r0=address
	movs r2,r0,lsr#8
	beq t9LoadWAsB
	movs r2,r0,lsr#14
	cmp r2,#1
	beq tlcs_ram_W_R
	bmi ram_low_R
	cmp r2,#2
	beq tlcs_vram_W_R
	mov r2,r2,lsr#7
	cmp r2,#1
	beq tlcs_rom_W_R
	cmp r2,#4
	beq tlcs_romH_W_R
	cmp r2,#7
	beq tlcs_bios_W_R
	b t9LoadWAsB
;@----------------------------------------------------------------------------
t9LoadUnevenW:				;@ r0=address
;@----------------------------------------------------------------------------
	t9eatcycles 1
;@----------------------------------------------------------------------------
t9LoadWAsB:					;@ r0=address
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}
	mov r3,r0
	bl t9LoadB
	mov r4,r0
	add r0,r3,#1
	bl t9LoadB
	orr r0,r4,r0,lsl#8
	ldmfd sp!,{r4,lr}
	bx lr
;@----------------------------------------------------------------------------
t9LoadL_mem:
;@----------------------------------------------------------------------------
	bic r0,t9Mem,#0xFF000000
;@----------------------------------------------------------------------------
t9LoadL:					;@ r0=address
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,r5,lr}
	tst r0,#1
	bne t9LoadUnevenL
	mov r5,r0
	bl t9LoadEvenW
	mov r4,r0
	add r0,r5,#2
	bl t9LoadEvenW
	orr r0,r4,r0,lsl#16
	ldmfd sp!,{r4,r5,lr}
	bx lr
;@----------------------------------------------------------------------------
t9LoadUnevenL:				;@ r0=address
;@----------------------------------------------------------------------------
	sub t9cycles,t9cycles,#1*T9CYCLE
	mov r5,r0
	bl t9LoadB
	mov r4,r0
	add r0,r5,#1
	bl t9LoadEvenW
	orr r4,r4,r0,lsl#8
	add r0,r5,#3
	bl t9LoadB
	orr r0,r4,r0,lsl#24
	ldmfd sp!,{r4,r5,lr}
	bx lr
;@----------------------------------------------------------------------------
tlcs_ram_R:					;@ Read RAM byte (0x004000-0x007FFF)
;@----------------------------------------------------------------------------
	ldr r1,=ngpRAM-0x4000
	ldrb r0,[r1,r0]!
	bx lr
;@----------------------------------------------------------------------------
tlcs_vram_R:				;@ Read VRAM byte (0x009000-0x00BFFF)
;@----------------------------------------------------------------------------
	tst r0,#0x007000
	beq k2GE_0R
	ldr r1,=k2geRAM-0x9000
	ldrb r0,[r1,r0]!
	bx lr
;@----------------------------------------------------------------------------
tlcs_rom_R:					;@ Read ROM byte (0x200000-0x3FFFFF)
;@----------------------------------------------------------------------------
	ldr r1,[t9ptr,#romBaseLo]
	ldrb r0,[r1,r0]!
	bx lr
;@----------------------------------------------------------------------------
tlcs_romH_R:				;@ Read ROM byte (0x800000-0x9FFFFF)
;@----------------------------------------------------------------------------
	ldr r1,[t9ptr,#romBaseHi]
	ldrb r0,[r1,r0]!
	bx lr
;@----------------------------------------------------------------------------
tlcs_bios_R:				;@ Read BIOS byte (0xFF0000-0xFFFFFF)
;@----------------------------------------------------------------------------
	ldr r1,[t9ptr,#biosBase]
	ldrb r0,[r1,r0]!
	bx lr

;@----------------------------------------------------------------------------
tlcs_ram_W_R:				;@ Read RAM word (0x004000-0x007FFF)
;@----------------------------------------------------------------------------
	ldr r1,=ngpRAM-0x4000
	ldrh r0,[r1,r0]
	bx lr
;@----------------------------------------------------------------------------
tlcs_vram_W_R:				;@ Read VRAM word (0x009000-0x00BFFF)
;@----------------------------------------------------------------------------
	tst r0,#0x007000
	beq k2GE_0R_W
	ldr r1,=k2geRAM-0x9000
	ldrh r0,[r1,r0]
	bx lr
;@----------------------------------------------------------------------------
tlcs_rom_W_R:				;@ Read ROM word (0x200000-0x3FFFFF)
;@----------------------------------------------------------------------------
	t9eatcycles 1
	ldr r1,[t9ptr,#romBaseLo]
	ldrh r0,[r1,r0]
	bx lr
;@----------------------------------------------------------------------------
tlcs_romH_W_R:				;@ Read ROM word (0x800000-0x9FFFFF)
;@----------------------------------------------------------------------------
	t9eatcycles 1
	ldr r1,[t9ptr,#romBaseHi]
	ldrh r0,[r1,r0]
	bx lr
;@----------------------------------------------------------------------------
tlcs_bios_W_R:				;@ Read BIOS word (0xFF0000-0xFFFFFF)
;@----------------------------------------------------------------------------
	ldr r1,[t9ptr,#biosBase]
	ldrh r0,[r1,r0]
	bx lr

;@----------------------------------------------------------------------------
z80RamW:					;@ Write mem (0x0000-0x0FFF)
;@----------------------------------------------------------------------------
	cmp addy,#0x1000
	ldrmi r1,=ngpRAM+0x3000
	strbmi r0,[r1,addy]
	bxmi lr
	b empty_W
;@----------------------------------------------------------------------------
z80RamR:					;@ Read Ram (0x0000-0x0FFF)
;@----------------------------------------------------------------------------
	cmp addy,#0x1000
	ldrmi r1,=ngpRAM+0x3000
	ldrbmi r0,[r1,addy]
	bxmi lr
	b empty_R
;@----------------------------------------------------------------------------
z80SoundW:					;@ Write mem (0x4000-0x4001)
;@----------------------------------------------------------------------------
	mov r1,#0x4000
	cmp addy,r1					;@ 0x4000, sound Right
	beq T6W28_R_W
	add r1,r1,#1				;@ 0x4001, sound Left
	cmp addy,r1
	beq T6W28_L_W
	b empty_W
;@----------------------------------------------------------------------------
z80IrqW:					;@ Write mem (0xC000)
;@----------------------------------------------------------------------------
	cmp addy,#0xC000
	bne empty_W
	stmfd sp!,{r3,lr}
		mov r0,#0x0C			;@ This must load up tlcs900 regs before executing.
		bl setInterruptExternal
	ldmfd sp!,{r3,lr}
	bx lr

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
