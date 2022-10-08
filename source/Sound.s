#ifdef __arm__

#include "Shared/gba_asm.h"
#include "t6w28.i"

#define MIX_LEN (528)

	.global soundInit
	.global soundReset
	.global vblSound1
	.global vblSound2
	.global setMuteSoundGUI
	.global setMuteT6W28
	.global T6W28_L_W
	.global T6W28_R_W
	.global t6W28_0
	.global soundMode

	.extern pauseEmulation


	.syntax unified
	.arm

	.section .ewram, "ax"
	.align 2
;@----------------------------------------------------------------------------
soundInit:
	.type soundInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r3-r5,lr}
	mov r5,#REG_BASE

;@	ldrh r0,[r5,#REG_SGBIAS]
;@	bic r0,r0,#0xc000			;@ Just change bits we know about.
;@	orr r0,r0,#0x8000			;@ PWM 7-bit 131.072kHz
;@	strh r0,[r5,#REG_SGBIAS]

	ldrb r2,soundMode			;@ If r2=0, no sound.
	cmp r2,#1

	movmi r0,#0
	ldreq r0,=0x0b040000		;@ Stop all channels, output ratio=100% dsA.  use directsound A for L&R, timer 0
//	ldreq r0,=0x9a0c0000		;@ use directsound A&B, timer 0
	str r0,[r5,#REG_SGCNT_L]

	moveq r0,#0x80
	strh r0,[r5,#REG_SGCNT_X]	;@ Sound master enable

	mov r0,#0					;@ Triangle reset
	str r0,[r5,#REG_SG3CNT_L]	;@ Sound3 disable, mute, write bank 0

								;@ Mixer channels
	strh r5,[r5,#REG_DMA1CNT_H]	;@ DMA1 stop, Left channel
	strh r5,[r5,#REG_DMA2CNT_H]	;@ DMA2 stop, Right channel
	add r2,r5,#REG_FIFO_A_L		;@ DMA1 destination..
	str r2,[r5,#REG_DMA1DAD]
	add r2,r5,#REG_FIFO_B_L		;@ DMA2 destination..
	str r2,[r5,#REG_DMA2DAD]
	ldr r2,pcmPtr0
	str r2,[r5,#REG_DMA1SAD]	;@ DMA1 src=..
	add r2,r2,#MIX_LEN*2
	str r2,[r5,#REG_DMA2SAD]	;@ DMA2 src=..

	ldr t6ptr,=t6W28_0
	mov r1,#1
	bl t6W28SetMixrate			;@ Sound, 0=low, 1=high mixrate
	ldr r1,=1536000				;@ 1.536MHz
	bl t6W28SetFrequency		;@ Sound, chip frequency
	ldr r1,=FREQTBL
	bl t6W28Init				;@ Sound


	ldrb r2,soundMode			;@ If r2=0, no sound.
	cmp r2,#1

	mov r2,#0					;@ Timer 0 controls sample rate:
	str r2,[r5,#REG_TM0CNT_L]	;@ Stop timer 0
	ldreq r2,[t6ptr,#mixRate]	;@ 924=Low, 532=High.
	rsbeq r2,r2,#0x810000		;@ Timer 0 on. Frequency = 0x1000000/r3 Hz
	streq r2,[r5,#REG_TM0CNT_L]

	ldmfd sp!,{r3-r5,lr}
	bx lr

;@----------------------------------------------------------------------------
soundReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	ldr t6ptr,=t6W28_0
	bl t6W28Reset				;@ sound
	ldmfd sp!,{lr}
	bx lr

;@----------------------------------------------------------------------------
setMuteSoundGUI:
	.type   setMuteSoundGUI STT_FUNC
;@----------------------------------------------------------------------------
	ldr r1,=pauseEmulation		;@ Output silence when emulation paused.
	ldrb r0,[r1]
	strb r0,muteSoundGUI
	bx lr
;@----------------------------------------------------------------------------
setMuteT6W28:
;@----------------------------------------------------------------------------
	and r0,r0,#0xFF
	cmp r0,#0xAA
	cmpne r0,#0x55
	andeq r0,r0,#0xAA
	strbeq r0,muteSoundChip
	bx lr
;@----------------------------------------------------------------------------
vblSound1:
	.type   vblSound1 STT_FUNC
;@----------------------------------------------------------------------------
	ldrb r0,soundMode			;@ if r0=0, no sound.
	cmp r0,#0
	bxeq lr

	mov r1,#REG_BASE
	strh r1,[r1,#REG_DMA1CNT_H]	;@ DMA1 stop
	strh r1,[r1,#REG_DMA2CNT_H]	;@ DMA2 stop
	ldr r2,pcmPtr0
	str r2,[r1,#REG_DMA1SAD]	;@ DMA1 src=..
	add r0,r2,#MIX_LEN*2
	str r0,[r1,#REG_DMA2SAD]	;@ DMA2 src=..
	ldr r0,=0xB640				;@ noIRQ fifo 32bit repeat incsrc fixeddst
	strh r0,[r1,#REG_DMA1CNT_H]	;@ DMA1 go
	strh r0,[r1,#REG_DMA2CNT_H]	;@ DMA2 go

	ldr r1,pcmPtr1
	str r1,pcmPtr0
	str r2,pcmPtr1

	bx lr
;@----------------------------------------------------------------------------
vblSound2:
	.type   vblSound2 STT_FUNC
;@----------------------------------------------------------------------------
	;@ Update DMA buffer for PCM
	ldrb r0,soundMode			;@ if r0=0, no sound.
	cmp r0,#0
	bxeq lr

	mov r2,#MIX_LEN
	ldr r1,pcmPtr0
	ldr r0,muteSound
	cmp r0,#0
	ldreq r0,=t6W28_0
	beq t6W28Mixer

;@----------------------------------------------------------------------------
silenceMix:					;@ r1=destination, r2=len
;@----------------------------------------------------------------------------
	mov r0,#0
silenceLoop:
	subs r2,r2,#4
	strpl r0,[r1],#4
	bhi silenceLoop

	bx lr
;@----------------------------------------------------------------------------
T6W28_R_W:					;@ Sound right write
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}
	ldr r1,=t6W28_0
	bl t6W28W
	ldmfd sp!,{r3,lr}
	bx lr
;@----------------------------------------------------------------------------
T6W28_L_W:					;@ Sound left write
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}
	ldr r1,=t6W28_0
	bl t6W28LW
	ldmfd sp!,{r3,lr}
	bx lr


;@----------------------------------------------------------------------------
pcmPtr0:	.long WAVBUFFER
pcmPtr1:	.long WAVBUFFER+MIX_LEN

muteSound:
muteSoundGUI:
	.byte 0
muteSoundChip:
	.byte 0
	.space 2
soundMode:
	.byte 1
	.space 3

	.section .sbss
	.align 2
t6W28_0:
	.space t6Size
FREQTBL:
	.space 1024*2
WAVBUFFER:
	.space MIX_LEN*2*2
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
