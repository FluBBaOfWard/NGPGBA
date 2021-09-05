;@ SNK Neogeo Pocket K2Audio sound chip emulator for ARM32.
#ifdef __arm__

#include "Shared/gba_asm.h"

	.global T6W28_init
	.global T6W28_reset
	.global T6W28_L_W
	.global T6W28_R_W
	.global Vbl_T6W28_1
	.global Vbl_T6W28_2

								;@ These values are for the SMS/GG/MD vdp/sound chip.
.equ PFEED_SMS,	0x8000			;@ Periodic Noise Feedback
.equ WFEED_SMS,	0x9000			;@ White Noise Feedback

								;@ These values are for the SN76489/SN76496 sound chip.
.equ PFEED_SN,	0x4000			;@ Periodic Noise Feedback
.equ WFEED_SN,	0x6000			;@ White Noise Feedback

#define PCMWAVSIZE (512)

;@----------------------------------------------------------------------------
	.section .iwram, "ax", %progbits	;@ For the GBA
;@----------------------------------------------------------------------------
;@ r0 = Mixer reg L.
;@ r1 = Mixer reg R.
;@ r2 -> r5 = Pos+freq.
;@ r6 = Noise generator.
;@ r7 = Noise feedback.
;@ r8 = Ch volumes L.
;@ r9 = Ch volumes R.
;@ r10 = Mixerbuffer L.
;@ r11 = Mixerbuffer R.
;@ r12 = Mix length.
;@ lr = return address.
;@----------------------------------------------------------------------------
mixer:
;@----------------------------------------------------------------------------
mixloop:
	mov r0,#0x82
	mov r1,#0x82
	adds r2,r2,r2,lsl#16
	addpl r0,r0,r8
	addpl r1,r1,r9

	adds r3,r3,r3,lsl#16
	addpl r0,r0,r8,lsr#8
	addpl r1,r1,r9,lsr#8

	adds r4,r4,r4,lsl#16
	addpl r0,r0,r8,lsr#16
	addpl r1,r1,r9,lsr#16

	adds r5,r5,r5,lsl#16
	movcss r6,r6,lsr#1
	eorcs r6,r6,r7
	tst r6,#1
	addne r0,r0,r8,lsr#24
	addne r1,r1,r9,lsr#24

	strb r0,[r10],#1
	strb r1,[r11],#1

	subs r12,r12,#1
	bhi mixloop

	bx lr
;@----------------------------------------------------------------------------

	.section .ewram,"ax"
	.align 2

;@----------------------------------------------------------------------------
T6W28_reset:
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	
	adrl r0,SoundVariables
	mov r1,#0
	mov r2,#8					;@ 32/4=8
	bl memset_					;@ clear variables
	str r1,ch0Lvolume			;@ silence
	str r1,ch0Rvolume			;@ silence

	ldmfd sp!,{lr}
	bx lr
;@----------------------------------------------------------------------------
frequency_reset:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,r10,lr}

	ldr r6,freqconv				;@ (3579545/mixrate)*4096
//	ldr r5,=FREQTBL				;@ Destination
	mov r4,#2048
frqloop2:
	mov r0,r6
	mov r1,r4
;@	subs r1,r4,#2
;@	moveq r1,r6
	swi 0x060000				;@ BIOS Div, r0/r1.
	cmp r4,#7*2
	movmi r0,#0					;@ to remove real high tones.
	subs r4,r4,#2
	strh r0,[r5,r4]
	bhi frqloop2

	ldmfd sp!,{r4-r6,r10,lr}
	bx lr

;@----------------------------------------------------------------------------
Vbl_T6W28_1:
;@----------------------------------------------------------------------------
	mov r1,#REG_BASE
	strh r1,[r1,#REG_DMA1CNT_H]	;@ DMA1 stop
	strh r1,[r1,#REG_DMA2CNT_H]	;@ DMA2 stop
	ldr r2,pcmptr0
	str r2,[r1,#REG_DMA1SAD]	;@ DMA1 src=..
	add r0,r2,#PCMWAVSIZE*2
	str r0,[r1,#REG_DMA2SAD]	;@ DMA2 src=..
	ldr r0,=0xB640				;@ noIRQ fifo 32bit repeat incsrc fixeddst
	strh r0,[r1,#REG_DMA1CNT_H]	;@ DMA1 go
	strh r0,[r1,#REG_DMA2CNT_H]	;@ DMA2 go

	ldr r1,pcmptr1
	str r1,pcmptr0
	str r1,pcmptr
	str r2,pcmptr1

	bx lr
;@----------------------------------------------------------------------------
Vbl_T6W28_2:
;@----------------------------------------------------------------------------
	;@ update DMA buffer for PCM

	stmfd sp!,{r3-r12,lr}
PSGMixer:
	adr r0,ch0freq
	ldmia r0,{r2-r10,r12}		;@ load freq,addr,rng, noisefb,volR, volL, ptr & len
	add r11,r10,#PCMWAVSIZE*2
;@--------------------------
	bl mixer
;@--------------------------
	adr r0,ch0freq
	stmia r0,{r2-r6}			;@ writeback freq,addr,rng

	ldmfd sp!,{r3-r12,pc}
;@----------------------------------------------------------------------------
T6W28_R_W:
;@----------------------------------------------------------------------------
	tst r0,#0x80
	andne r1,r0,#0x70
	strneb r1,lastregR
	ldreqb r1,lastregR
	movs r1,r1,lsr#5
	bcc SetFreqR
DoVolumeR:
	and r0,r0,#0x0F
	adr r2,Attenuation
	ldrb r0,[r2,r0]
	adr r2,ch0Rvolume
	strb r0,[r2,r1]
	bx lr

SetFreqR:
	cmp r1,#2
	bxmi lr
	bhi SetNoiseF				;@ noise channel
	adr r2,ch2Rreg
	tst r0,#0x80
	andeq r0,r0,#0x3F
	movne r0,r0,lsl#4
	streqb r0,[r2,#1]
	strneb r0,[r2]
	ldrh r0,[r2]
	mov r0,r0,lsr#3

//	ldr r2,=FREQTBL
	ldrh r0,[r2,r0]
	strh r0,ch2Rfreq
	ldrh r2,ch3reg
	and r2,r2,#0x3
	cmp r2,#0x3
	streqh r0,ch3freq
	bx lr

SetNoiseF:
	strh r0,ch3reg
	tst r0,#0x4
	mov r2,#PFEED_SMS			;@ Periodic noise / preset seed
	str r2,rng
	movne r2,#WFEED_SMS			;@ White noise
	str r2,noisefb
	ldr r2,freqconv
	ands r0,r0,#0x3
	moveq r1,r2,lsr#5			;@ These values sound ok
	movne r1,r2,lsr#6
	cmp r0,#0x2
	moveq r1,r2,lsr#7
	ldrhih r1,ch2Rfreq
	strh r1,ch3freq
	bx lr
;@----------------------------------------------------------------------------
T6W28_L_W:
;@----------------------------------------------------------------------------
	tst r0,#0x80
	andne r1,r0,#0x70
	strneb r1,lastregL
	ldreqb r1,lastregL
	movs r1,r1,lsr#5
	bcc SetFreqL
DoVolumeL:
	and r0,r0,#0x0F
	adr r2,Attenuation
	ldrb r0,[r2,r0]
	adr r2,ch0Lvolume
	strb r0,[r2,r1]
	bx lr

SetFreqL:
	cmp r1,#3					;@ noise channel not here.
	bxeq lr
	tst r0,#0x80
	adr r2,ch0reg
	add r2,r2,r1,lsl#1
	andeq r0,r0,#0x3F
	movne r0,r0,lsl#4
	streqb r0,[r2,#1]
	strneb r0,[r2]
	ldrh r0,[r2]
	mov r0,r0,lsr#3

//	ldr r2,=FREQTBL
	ldrh r0,[r2,r0]
	adr r2,ch0freq
	add r2,r2,r1,lsl#2
	strh r0,[r2]
	bx lr

Attenuation:
	.byte 0x3F,0x32,0x28,0x20,0x19,0x14,0x10,0x0D,0x0A,0x08,0x06,0x05,0x04,0x03,0x02,0x00
;@----------------------------------------------------------------------------
SoundVariables:
lastregL:	.byte 0
lastregR:	.byte 0
			.space 2
ch2Rreg:	.word 0
ch2Rfreq:	.word 0

ch0reg:		.word 0
ch1reg:		.word 0
ch2reg:		.word 0
ch3reg:		.word 0

;@----------------------------------------------------------------------------

ch0freq:	.word 0				;@ freq,addr, rng/noisefb & volume need to be like this for the mixer start.
ch0addr:	.word 0
ch1freq:	.word 0
ch1addr:	.word 0
ch2freq:	.word 0
ch2addr:	.word 0
ch3freq:	.word 0
ch3addr:	.word 0

rng:		.long PFEED_SMS		;@ noise generator
noisefb:	.long WFEED_SMS		;@ noise feedback

ch0Lvolume:	.byte 0
ch1Lvolume:	.byte 0
ch2Lvolume:	.byte 0
ch3Lvolume:	.byte 0

ch0Rvolume:	.byte 0
ch1Rvolume:	.byte 0
ch2Rvolume:	.byte 0
ch3Rvolume:	.byte 0

pcmptr:		.long 0
mixlength:	.long 528				;@ mixlength (528=high, 304=low)
pcmptr0:	.long 0
pcmptr1:	.long 0+PCMWAVSIZE
;@----------------------------------------------------------------------------

mixrate:		.long 532			;@ mixrate (532=high, 924=low), (mixrate=0x1000000/mixer_frequency)
freqconv:		.long 0x61698		;@ Frequency conversion (0x61698=high, 0xA9308=low) (3072000/mixer_frequency)*4096

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
