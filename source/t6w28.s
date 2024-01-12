;@
;@  T6W28.s
;@  T6W28
;@
;@  Created by Fredrik Ahlström on 2008-04-02.
;@  Copyright © 2008-2024 Fredrik Ahlström. All rights reserved.
;@
;@ SNK Neogeo Pocket K2Audio sound chip emulator for ARM32.

#ifdef __arm__

#include "t6w28.i"

#define PCMWAVSIZE (528)

	.global t6W28Init
	.global t6W28Reset
	.global t6W28SetMixrate
	.global t6W28SetFrequency
	.global t6W28SaveState
	.global t6W28LoadState
	.global t6W28GetStateSize
	.global t6W28Mixer
	.global t6W28LW
	.global t6W28W

								;@ These values are for the SMS/GG/MD vdp/sound chip.
	.equ PFEED_SMS,	0x8000		;@ Periodic Noise Feedback
	.equ WFEED_SMS,	0x9000		;@ White Noise Feedback

	.syntax unified
	.arm

#ifdef NDS
	.section .itcm						;@ For the NDS
#elif GBA
	.section .iwram, "ax", %progbits	;@ For the GBA
#else
	.section .text
#endif
	.align 2
;@----------------------------------------------------------------------------
;@ r0 = Mix length.
;@ r1 = Mixerbuffer L.
;@ r2 = t6ptr.
;@ r3 -> r6 = Pos+freq.
;@ r7  = currentBits.
;@ r8 = Noise generator.
;@ r9 = Noise feedback.
;@ r12 = Mixer reg L.
;@ lr = Mixer reg R.
;@----------------------------------------------------------------------------
t6W28Mixer:					;@ r0=len, r1=dest, r2=t6ptr
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r9,lr}
	ldmia r2,{r3-r9,lr}		;@ Load freq/addr0-3, currentBits, rng, noisefb, attChg
	tst lr,#0xff
	blne calculateVolumes
;@----------------------------------------------------------------------------
mixLoop:
	adds r3,r3,r3,lsl#16
	eorcs r7,r7,#0x04

	adds r4,r4,r4,lsl#16
	eorcs r7,r7,#0x08

	adds r5,r5,r5,lsl#16
	eorcs r7,r7,#0x10

	adds r6,r6,r6,lsl#16
	biccs r7,r7,#0x20
	movscs r8,r8,lsr#1
	eorcs r8,r8,r9
	orrcs r7,r7,#0x20

	ldr lr,[r2,r7]
	subs r0,r0,#1
	mov r12,lr,lsr#16
	strbpl r12,[r1,#PCMWAVSIZE*2]
	strbpl lr,[r1],#1
	bhi mixLoop

	stmia r2,{r3-r8}			;@ Writeback freq,addr, currentBits,rng
	ldmfd sp!,{r4-r9,lr}
	bx lr
;@----------------------------------------------------------------------------

	.section .text
	.align 2
;@----------------------------------------------------------------------------
t6W28Init:					;@ t6ptr=r0=pointer to struct, r1=FREQTABLE
;@----------------------------------------------------------------------------
	stmfd sp!,{t6ptr,lr}
	bl frequencyCalculate
	ldmfd sp!,{t6ptr,lr}
;@----------------------------------------------------------------------------
t6W28Reset:					;@ t6ptr=r0=pointer to struct
;@----------------------------------------------------------------------------
	mov r1,#0
	mov r2,#(t6StateEnd-t6StateStart)/4		;@ 64/4=16
rLoop:
	subs r2,r2,#1
	strpl r1,[r0,r2,lsl#2]
	bhi rLoop

	mov r2,#PFEED_SMS
	strh r2,[r0,#rng]
	mov r2,#WFEED_SMS
	strh r2,[r0,#noiseFB]
	mov r2,#calculatedVolumes
	str r2,[r0,#currentBits]	;@ Add offset to calculatedVolumes
	ldr r1,=0x00800080
	str r1,[r0,r2]				;@ Clear volume 0

	bx lr

;@----------------------------------------------------------------------------
t6W28SetMixrate:			;@ snptr=r0=ptr to struct, r1 in. 0 = low, 1 = high
;@----------------------------------------------------------------------------
	cmp r1,#0
	moveq r1,#924				;@ low,  18157Hz
	movne r1,#532				;@ high, 31536Hz
	str r1,[t6ptr,#mixRate]
	moveq r1,#304				;@ low
	movne r1,#528				;@ high
	str r1,[t6ptr,#mixLength]
	bx lr
;@----------------------------------------------------------------------------
t6W28SetFrequency:			;@ snptr=r0=ptr to struct, r1=frequency of chip.
;@----------------------------------------------------------------------------
	ldr r2,[t6ptr,#mixRate]
	mul r1,r2,r1
	mov r1,r1,lsr#10
	str r1,[t6ptr,#freqConv]	;@ Frequency conversion (SN76496freq*mixrate)/1024
	bx lr
;@----------------------------------------------------------------------------
frequencyCalculate:			;@ t6ptr=r0=ptr to struct, r1=FREQTABLE
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	str r1,[t6ptr,#freqTablePtr]
	mov r5,r1					;@ Destination
	ldr r6,[t6ptr,#freqConv]	;@ (sn76496/gba)*4096
	mov r4,#2048
frqLoop:
	mov r0,r6
	mov r1,r4
	swi 0x060000				;@ BIOS Div, r0/r1.
	cmp r4,#7*2
	movmi r0,#0					;@ To remove real high tones.
	subs r4,r4,#2
	strh r0,[r5,r4]
	bhi frqLoop

	ldmfd sp!,{r4-r6,lr}
	bx lr

;@----------------------------------------------------------------------------
t6W28SaveState:				;@ In r0=destination, r1=snptr. Out r0=state size.
	.type   sn76496SaveState STT_FUNC
;@----------------------------------------------------------------------------
	mov r2,#t6StateEnd-t6StateStart
	stmfd sp!,{r2,lr}

	bl memcpy

	ldmfd sp!,{r0,lr}
	bx lr
;@----------------------------------------------------------------------------
t6W28LoadState:				;@ In r0=snptr, r1=source. Out r0=state size.
	.type   sn76496LoadState STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r0,lr}

	mov r2,#t6StateEnd-t6StateStart
	bl memcpy
	ldmfd sp!,{r0,lr}
	mov r1,#1
	strb r1,[r0,#snAttChg]

;@----------------------------------------------------------------------------
t6W28GetStateSize:			;@ Out r0=state size.
	.type   sn76496GetStateSize STT_FUNC
;@----------------------------------------------------------------------------
	mov r0,#t6StateEnd-t6StateStart
	bx lr

;@----------------------------------------------------------------------------
	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
t6W28W:						;@ In r0 = value, r1 = struct-pointer, right ch.
	.type   t6W28W STT_FUNC
;@----------------------------------------------------------------------------
	tst r0,#0x80
	andne r3,r0,#0x70
	strbne r3,[r1,#snLastReg]
	ldrbeq r3,[r1,#snLastReg]
	movs r3,r3,lsr#5
	add r2,r1,r3,lsl#2
	bcc setFreq
doVolume:
	and r0,r0,#0x0F
	ldrb r3,[r2,#ch0Att]
	eors r3,r3,r0
	strbne r0,[r2,#ch0Att]
	strbne r3,[r1,#snAttChg]
	bx lr

setFreq:
	cmp r3,#2
	bhi setNoiseFreq
	bxmi lr
	tst r0,#0x80
	andeq r0,r0,#0x3F
	movne r0,r0,lsl#4
	strbeq r0,[r2,#ch0Reg+1]
	strbne r0,[r2,#ch0Reg]
	ldrh r0,[r2,#ch0Reg]
	mov r0,r0,lsr#3
	strh r0,[r1,#ch1Reg]

	ldr r2,[r1,#freqTablePtr]
	ldrh r0,[r2,r0]
	cmp r3,#2					;@ Ch2
	ldrbeq r2,[r1,#ch3Reg]
	cmpeq r2,#3
	strheq r0,[r1,#ch3Frq]
	bx lr

setNoiseFreq:
	and r2,r0,#3
	strb r2,[r1,#ch3Reg]
	tst r0,#4
	mov r0,#PFEED_SMS			;@ Periodic noise
	strh r0,[r1,#rng]
	movne r0,#WFEED_SMS			;@ White noise
	strh r0,[r1,#noiseFB]
	mov r3,#0x0400				;@ These values sound ok
	mov r3,r3,lsl r2
	cmp r2,#3
	ldrheq r3,[r1,#ch1Reg]
	ldreq r2,[r1,#freqTablePtr]
	ldrheq r3,[r2,r3]
	strh r3,[r1,#ch3Frq]
	bx lr
;@----------------------------------------------------------------------------
t6W28LW:					;@ In r0 = value, r1 = struct-pointer, left ch.
	.type   t6W28L_W STT_FUNC
;@----------------------------------------------------------------------------
	tst r0,#0x80
	andne r3,r0,#0x70
	strbne r3,[r1,#snLastRegL]
	ldrbeq r3,[r1,#snLastRegL]
	movs r3,r3,lsr#5
	add r2,r1,r3,lsl#2
	bcc setFreqL
doVolumeL:
	and r0,r0,#0x0F
	ldrb r3,[r2,#ch0AttL]
	eors r3,r3,r0
	strbne r0,[r2,#ch0AttL]
	strbne r3,[r1,#snAttChg]
	bx lr

setFreqL:
	cmp r3,#3					;@ Noise channel
	bxeq lr
	tst r0,#0x80
	andeq r0,r0,#0x3F
	movne r0,r0,lsl#4
	strbeq r0,[r2,#ch0RegL+1]
	strbne r0,[r2,#ch0RegL]
	ldrh r0,[r2,#ch0RegL]
	mov r0,r0,lsr#3
	ldr r3,[r1,#freqTablePtr]
	ldrh r0,[r3,r0]
	strh r0,[r2,#ch0Frq]

//	cmp r3,#2					;@ Ch2
//	ldrbeq r2,[r1,#ch3Reg]
//	cmpeq r2,#3
//	strheq r0,[r1,#ch3Frq]
	bx lr

;@----------------------------------------------------------------------------
calculateVolumes:			;@ In r2 = snptr
;@----------------------------------------------------------------------------
	stmfd sp!,{r0,r1,r3-r7,lr}

	adr r1,attenuation

	ldrb r0,[r2,#ch0Att]
	ldr r3,[r1,r0,lsl#2]
	ldrb r0,[r2,#ch0AttL]
	ldr r0,[r1,r0,lsl#2]
	orr r3,r3,r0,lsl#16

	ldrb r0,[r2,#ch1Att]
	ldr r4,[r1,r0,lsl#2]
	ldrb r0,[r2,#ch1AttL]
	ldr r0,[r1,r0,lsl#2]
	orr r4,r4,r0,lsl#16

	ldrb r0,[r2,#ch2Att]
	ldr r5,[r1,r0,lsl#2]
	ldrb r0,[r2,#ch2AttL]
	ldr r0,[r1,r0,lsl#2]
	orr r5,r5,r0,lsl#16

	ldrb r0,[r2,#ch3Att]
	ldr r6,[r1,r0,lsl#2]
	ldrb r0,[r2,#ch3AttL]
	ldr r0,[r1,r0,lsl#2]
	orr r6,r6,r0,lsl#16

	add lr,r2,#calculatedVolumes
	ldr r7,=0x00800080
	mov r1,#15
volLoop:
	movs r0,r1,lsl#31
	movmi r0,r3
	addcs r0,r0,r4
	teq r1,r1,lsl#29
	addmi r0,r0,r5
	addcs r0,r0,r6
	add r0,r7,r0,lsr#8
	str r0,[lr,r1,lsl#2]
	subs r1,r1,#1
	bne volLoop
	strb r1,[r2,#snAttChg]
	ldmfd sp!,{r0,r1,r3-r7,pc}
;@----------------------------------------------------------------------------
attenuation:						;@ each step * 0.79370053 (-2dB?)
	.long 0x3FFF,0x32CB,0x2851,0x2000,0x1966,0x1428,0x1000,0x0CB3
	.long 0x0A14,0x0800,0x0659,0x050A,0x0400,0x032C,0x0285,0x0000
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
