#ifdef __arm__

#include "TLCS900H/TLCS900H.i"
#include "TLCS900H/TLCS900H_mac.h"

	.global sngBIOSHLE

	.syntax unified
	.arm


;@----------------------------------------------------------------------------
	.section .text
;@----------------------------------------------------------------------------
callCFunc:					;@ r0 = arg0, r1 = arg1, r2 = function.
;@----------------------------------------------------------------------------
	stmfd sp!,{r0-r2,lr}
	bl storeTLCS900
	ldmfd sp!,{r0-r2}
	adr lr,callcRet
	bx r2
callcRet:
	bl loadTLCS900
	ldmfd sp!,{lr}
	bx lr

;@----------------------------------------------------------------------------
sngBIOSHLE:
;@----------------------------------------------------------------------------
	ldr r0,[t9optbl,#tlcsLastBank]
	sub r0,t9pc,r0
	and r0,r0,#0xFF0000
	cmp r0,#0xFF0000
	bne notBios

	ldrb r0,[t9pc],#1
	tst r0,#0x40
	bne asmBiosHLE
	ldr r2,=iBIOSHLE
	bl callCFunc				;@ r0 = arg0, r1 = arg1, r2 = function.
	t9fetch 8
notBios:
	mov r11,r11
	t9fetch 8
	
;@----------------------------------------------------------------------------
asmBiosHLE:
;@----------------------------------------------------------------------------
	cmp r0,#0x40
	beq BiosReset
	cmp r0,#0x41
	beq BiosSWI_1
	cmp r0,#0x4B
	beq BiosIRQ_VBlank
	cmp r0,#0x4C
	beq BiosIRQ_Z80
	cmp r0,#0x61
	bmi BiosIRQ_User
	t9fetch 24

;@----------------------------------------------------------------------------
BiosReset:
;@----------------------------------------------------------------------------
	mov r0,#0x6C00
	str r0,[t9gprBank,#0x1C]	;@ XSP
	bl t9LoadL					;@ 0x6C00 Should also have the game start vector.
	bl encode_r0_pc
	t9fetch 24

;@----------------------------------------------------------------------------
BiosSWI_1:
;@----------------------------------------------------------------------------
	ldr r0,[t9optbl,#tlcsLastBank]
	sub r0,t9pc,r0
	bl push32
	add r2,t9optbl,#tlcsGprBanks
	ldrb r0,[r2,#0x20*3+1]
	ldr r1,=0xFFFE00
	add r0,r1,r0,lsl#2
	bl t9LoadL
	bl encode_r0_pc
	t9fetch 24

;@----------------------------------------------------------------------------
BiosIRQ_Z80:				;@ Extra work for Puyo Pop
;@----------------------------------------------------------------------------
	ldr r1,=ngpRAM+0x2F83		;@ User Z80 IRQ allowed?
	ldrb r2,[r1]
	tst r2,#0x40
	bne BiosIRQ_User

	ldr r1,=ngpRAM+0x2DA2		;@ Z80 IRQ done?
	ldrb r0,[r1]
	tst r0,#0x80
	bne not3reset
	orr r0,r0,#0x80
	strb r0,[r1]

	mov r0,#0x28
	bl t9LoadB
	and r0,r0,#0x33
	orr r0,r0,#0x04
	mov r1,#0x28
	bl t9StoreB

	mov r0,#0x25
	bl t9LoadB
	and r0,r0,#0x0F
	orr r0,r0,#0xB0
	mov r1,#0x25
	bl t9StoreB

	mov r0,#0x62
	mov r1,#0x27
	bl t9StoreB

	mov r0,#0x20
	bl t9LoadB
	orr r0,r0,#0x88
	mov r1,#0x20
	bl t9StoreB

not3reset:
	t9fetch 24
;@----------------------------------------------------------------------------
BiosIRQ_VBlank:
;@----------------------------------------------------------------------------
	ldr r1,=systemMemory+0xB0	;@ HW joypad
	ldrb r2,[r1]
	ldr r1,=ngpRAM+0x2F82		;@ BIOS joypad
	strb r2,[r1]
	ldr r1,=ngpRAM+0x2C5F		;@ BIOS joypad backup
	strb r2,[r1]
;@----------------------------------------------------------------------------
BiosIRQ_User:
;@----------------------------------------------------------------------------
	and r0,r0,#0x3F
	ldr r1,=IndexConv
	ldrb r0,[r1,r0]
	ldr r1,=0x6FB8
	add r0,r1,r0,lsl#2
	bl t9LoadL
	bl encode_r0_pc
	t9fetch 24

;@----------------------------------------------------------------------------

#endif // #ifdef __arm__
