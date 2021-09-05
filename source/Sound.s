#ifdef __arm__

#include "Shared/gba_asm.h"

#define MIX_LEN (528)

	.global soundInit
	.global soundReset
	.global vblSound1
	.global vblSound2
	.global setMuteSoundGUI
	.global setMuteSoundChip
	.global soundLatchW
	.global ym1IndexW
	.global ym1DataW
	.global ym1StatusR
	.global ym1DataR
	.global ym2IndexW
	.global ym2DataW
	.global ym2StatusR
	.global ym2DataR

	.extern pauseEmulation


	.syntax unified
	.arm

	.section .ewram,"ax"
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
	str r0,[r5,#REG_SGCNT_L]

	moveq r0,#0x80
	strh r0,[r5,#REG_SGCNT_X]	;@ Sound master enable

	mov r0,#0					;@ Triangle reset
	str r0,[r5,#REG_SG3CNT_L]	;@ Sound3 disable, mute, write bank 0

								;@ Mixer channels
	strh r5,[r5,#REG_DMA1CNT_H]	;@ DMA1 stop, SN76496
	add r0,r5,#REG_FIFO_A_L		;@ DMA1 destination..
	str r0,[r5,#REG_DMA1DAD]
	ldr r0,pcmPtr0
	str r0,[r5,#REG_DMA1SAD]	;@ DMA1 src=..

/*
	ldr snptr,=SN76496_0
	mov r1,#1
	bl SN76496SetMixrate		;@ Sound, 0=low, 1=high mixrate
	ldr r1,=1536000
	bl SN76496SetFrequency		;@ Sound, chip frequency
	ldr r1,=FREQTBL
	bl SN76496Init				;@ Sound
*/

	ldrb r2,soundMode			;@ If r2=0, no sound.
	cmp r2,#1

	add r1,r5,#REG_TM0CNT_L		;@ Timer 0 controls sample rate:
	mov r4,#0
	str r4,[r1]					;@ Stop timer 0
//	ldreq r3,[snptr,#mixRate]	;@ 924=Low, 532=High.
	rsbeq r4,r3,#0x810000		;@ Timer 0 on. Frequency = 0x1000000/r3 Hz
	streq r4,[r1]

	ldmfd sp!,{r3-r5,lr}
	bx lr

;@----------------------------------------------------------------------------
soundReset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
//	ldr ymptr,=ym1
//	bl ym2203Reset				;@ sound
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
setMuteSoundChip:
;@----------------------------------------------------------------------------
	strb r0,muteSoundChip
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
	ldr r2,pcmPtr0
	str r2,[r1,#REG_DMA1SAD]	;@ DMA1 src=..
	ldr r0,=0xB640				;@ noIRQ fifo 32bit repeat incsrc fixeddst
//	strh r0,[r1,#REG_DMA1CNT_H]	;@ DMA1 go

	ldr r1,pcmPtr1
	str r1,pcmPtr0
	str r2,pcmPtr1

	bx lr
;@----------------------------------------------------------------------------
vblSound2:
	.type   vblSound2 STT_FUNC
;@----------------------------------------------------------------------------
	;@ update DMA buffer for PCM
	ldrb r0,soundMode			;@ if r0=0, no sound.
	cmp r0,#0
	bxeq lr

	mov r2,#MIX_LEN
	ldr r1,pcmPtr0
	ldr r0,muteSound
	cmp r0,#0
//	b silenceMix

//	ldreq ymptr,=ym1
//	beq YM2203Mixer

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
soundLatchW:
;@----------------------------------------------------------------------------
	strb r0,soundLatch
	bx lr
;@----------------------------------------------------------------------------
ym2DataR:
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}
//	ldr ymptr,=ym2
//	bl ym2203DataR
	ldmfd sp!,{r3,pc}

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
	.byte 0
soundLatch:
	.byte 0
	.space 2

	.section .sbss
	.align 2
//	.space ymSize
FREQTBL:
	.space 1024*2
WAVBUFFER:
	.space MIX_LEN*2
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
