#ifdef __arm__

#include "TLCS900H/TLCS900H.i"
#include "ARMZ80/ARMZ80.i"
#include "K2GE/K2GE.i"

	.global isConsoleRunning
	.global isConsoleSleeping
	.global tweakCpuSpeed
	.global tweakZ80Speed
	.global frameTotal
	.global waitMaskIn
	.global waitMaskOut
	.global setInterruptExternal
	.global Z80_SetEnable
	.global Z80_nmi_do
	.global getRegAdr

	.global run
	.global stepFrame
	.global cpuReset

	.syntax unified
	.arm

#if GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
run:						;@ Return after X frame(s)
	.type   run STT_FUNC
;@----------------------------------------------------------------------------
	ldrh r0,waitCountIn
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountIn
	bxne lr
	stmfd sp!,{r4-r11,lr}

;@----------------------------------------------------------------------------
runStart:
;@----------------------------------------------------------------------------
	ldr r0,=EMUinput
	ldr r0,[r0]
	ldr r3,joyClick
	eor r3,r3,r0
	and r3,r3,r0
	str r0,joyClick

	tst r3,#0x04				;@ GBA Select?
	ldrne r2,=systemMemory+0xB3
	ldrbne r2,[r2]
	tstne r2,#4					;@ Power button NMI enabled?
	movne r0,#0x08				;@ 0x08 = Power button on NGP
	blne setInterruptExternal

	bl refreshEMUjoypads		;@ Z=1 if communication ok

;@----------------------------------------------------------------------------
ngpFrameLoop:
;@----------------------------------------------------------------------------
	ldrh r0,z80Enabled
	ands r0,r0,r0,lsr#8
	beq NoZ80Now

	ldr z80ptr,=Z80OpTable
	ldr r0,z80CyclesPerScanline
	bl Z80RestoreAndRunXCycles
	add r0,z80ptr,#z80Regs
	stmia r0,{z80f-z80pc,z80sp}	;@ Save Z80 state
NoZ80Now:
;@--------------------------------------
	ldr t9ptr,=tlcs900HState
	ldr r0,tlcs900hCyclesPerScanline
	bl tlcsRestoreAndRunXCycles
;@--------------------------------------
	ldr geptr,=k2GE_0
	bl k2GEDoScanline
	cmp r0,#0
	bne ngpFrameLoop
;@----------------------------------------------------------------------------

	ldr r1,=fpsValue
	ldr r0,[r1]
	add r0,r0,#1
	str r0,[r1]

	ldr r1,frameTotal
	add r1,r1,#1
	str r1,frameTotal

	ldrh r0,waitCountOut
	add r0,r0,#1
	ands r0,r0,r0,lsr#8
	strb r0,waitCountOut
	ldmfdeq sp!,{r4-r11,lr}		;@ Exit here if doing single frame:
	bxeq lr						;@ Return to rommenu()
	b runStart

;@----------------------------------------------------------------------------
tlcs900hCyclesPerScanline:	.long 0
z80CyclesPerScanline:	.long 0
joyClick:			.long 0
frameTotal:			.long 0		;@ Let Gui.c see frame count for savestates
waitCountIn:		.byte 0
waitMaskIn:			.byte 0
waitCountOut:		.byte 0
waitMaskOut:		.byte 0

z80Enabled:			.byte 0
gZ80OnOff:			.byte 1
					.byte 0,0

;@----------------------------------------------------------------------------
stepFrame:					;@ Return after 1 frame
	.type stepFrame STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r11,lr}
;@----------------------------------------------------------------------------
ngpStepLoop:
;@----------------------------------------------------------------------------
	ldrh r0,z80Enabled
	ands r0,r0,r0,lsr#8
	beq NoZ80Step

	ldr z80ptr,=Z80OpTable
	ldr r0,z80CyclesPerScanline
	bl Z80RestoreAndRunXCycles
	add r0,z80ptr,#z80Regs
	stmia r0,{z80f-z80pc,z80sp}	;@ Save Z80 state
NoZ80Step:
;@--------------------------------------
	ldr t9ptr,=tlcs900HState
	ldr r0,tlcs900hCyclesPerScanline
	bl tlcsRestoreAndRunXCycles
;@--------------------------------------
	ldr geptr,=k2GE_0
	bl k2GEDoScanline
	cmp r0,#0
	bne ngpStepLoop
;@----------------------------------------------------------------------------

	ldr r1,frameTotal
	add r1,r1,#1
	str r1,frameTotal

	ldmfd sp!,{r4-r11,lr}
	bx lr
;@----------------------------------------------------------------------------
isConsoleRunning:
	.type   isConsoleRunning STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=ngpRAM+0x2C7C
	ldrh r0,[r1]
	ldr r1,=0x5AA5
	cmp r0,r1
	moveq r0,#1
	movne r0,#0
	bx lr
;@----------------------------------------------------------------------------
isConsoleSleeping:
	.type   isConsoleSleeping STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=ngpRAM+0x2C7C
	ldrh r0,[r1]
	ldr r1,=0xA55A
	cmp r0,r1
	moveq r0,#1
	movne r0,#0
	bx lr
;@---------------------------------------------------------------------------
setInterruptExternal:		;@ r0 = index
	.type setInterruptExternal STT_FUNC
;@---------------------------------------------------------------------------
	stmfd sp!,{t9ptr,lr}
	ldr t9ptr,=tlcs900HState
	bl setInterrupt
	ldmfd sp!,{t9ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
Z80_SetEnable:				;@ Address 0xB9 of the TLCS-900H, r0=enabled
;@----------------------------------------------------------------------------
	cmp r0,#0x55				;@ On
	cmpne r0,#0xAA				;@ Off
	bxne lr
	and r0,r0,#1
	strb r0,z80Enabled
	eor r0,r0,#1
	stmfd sp!,{z80ptr,lr}
	ldr z80ptr,=Z80OpTable
	bl Z80SetResetPin
	ldmfd sp!,{z80ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
Z80_nmi_do:					;@ Address 0xBA of the TLCS-900H
;@----------------------------------------------------------------------------
	ldrb r1,z80Enabled
	cmp r1,#0
	bxeq lr
	stmfd sp!,{z80ptr,lr}
	ldr z80ptr,=Z80OpTable
	mov r0,#1
	bl Z80SetNMIPin
	ldmfd sp!,{z80ptr,lr}
	bx lr
;@----------------------------------------------------------------------------
cpu1SetIRQ:
;@----------------------------------------------------------------------------
	stmfd sp!,{z80ptr,lr}
	ldr z80ptr,=Z80OpTable
	bl Z80SetIRQPin
	ldmfd sp!,{z80ptr,pc}
;@----------------------------------------------------------------------------
getRegAdr:				;@ r0=register, 0x00-0xFF
	.type   getRegAdr STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=tlcs900HState
	ldr r2,[r1,#tlcsCurrentMapBank]
	ldrsb r0,[r2,r0]
	ldr r2,[r1,#tlcsCurrentGprBank]
	add r0,r2,r0
	bx lr
;@----------------------------------------------------------------------------
tweakCpuSpeed:				;@ in r0=0 normal / !=0 half speed.
	.type   tweakCpuSpeed STT_FUNC
;@----------------------------------------------------------------------------
	cmp r0,#0
;@---Speed - 6.144MHz / 60Hz / 198 lines	;NGP TLCS-900H.
	ldr r0,=T9_HINT_RATE				;@ 515
	movne r0,r0,lsr#1
	str r0,tlcs900hCyclesPerScanline
;@---Speed - 3.072MHz / 60Hz / 198 lines	;NGP Z80.
	mov r0,r0,lsr#1
	str r0,z80CyclesPerScanline
	bx lr
;@----------------------------------------------------------------------------
tweakZ80Speed:				;@ in r0=0 normal, 1=half speed, 2=1/4 speed...
	.type   tweakZ80Speed STT_FUNC
;@----------------------------------------------------------------------------
;@---Speed - 3.072MHz / 60Hz / 198 lines	;NGP Z80.
	ldr r1,=T9_HINT_RATE/2				;@ 515/2
	mov r1,r1,lsr r0
	str r1,z80CyclesPerScanline
	bx lr
;@----------------------------------------------------------------------------
cpuReset:					;@ Called by loadCart/resetGame
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldr r0,=emuSettings
	ldr r0,[r0]
	and r0,r0,#0x10000
	bl tweakCpuSpeed

	ldr r0,=tlcs900HState
	adr r1,cpu1SetIRQ
	bl tlcs900HReset

;@--------------------------------------
	ldr r0,=gZ80Speed
	ldrb r0,[r0]
	and r0,r0,#7
	bl tweakZ80Speed

	ldr r0,=Z80OpTable
	mov r1,#0
	bl Z80Reset

	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
