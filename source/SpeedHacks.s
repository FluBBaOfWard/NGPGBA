#ifdef __arm__

#include "TLCS900H/TLCS900H_mac.h"
#include "ARMZ80/ARMZ80mac.h"

	.global hacksInit

;@----------------------------------------------------------------------------

	.syntax unified
	.arm

	.section .text						;@ For anything else
	.align 2
;@----------------------------------------------------------------------------
hacksInit:
	.type   hacksInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}

//	mov r0,#0x28				;@ jrz
//	ldr r1,=z80jrzHack
//	bl Z80RedirectOpcode

	mov r4,#0x60
	adr r5,OriginalOpcodes
origLoop:
	mov r0,r4
	ldr r1,[r5],#4
	bl tlcs900HRedirectOpcode
	add r4,r4,#1
	cmp r4,#0x70
	bne origLoop

	ldr r0,=emuSettings
	ldr r0,[r0]					;@ Speed hacks enabled?
	tst r0,#0x20000
	beq noHacks

	ldr r4,=romSpacePtr
	ldr r4,[r4]					;@ Get rom base
	ldrh r0,[r4,#0x20]
	ldr r1,=0x0117
	cmp r0,r1
	bpl noHacks

	and r3,r0,#0x000F
	mov r1,#10
	and r2,r0,#0x00F0
	mul r2,r1,r2
	add r3,r3,r2,lsr#4
	mov r1,#100
	and r2,r0,#0x0F00
	mul r2,r1,r2
	add r1,r3,r2,lsr#8

	adr r4,SpeedHacks
	ldrb r0,[r4,r1,lsl#1]!
	cmp r0,#0
	beq noHacks
	ldr r1,=sngJR_hack0
	bl InstallHack
	ldrb r0,[r4,#1]
	cmp r0,#0
	ldr r1,=sngJR_hack1
	blne InstallHack
noHacks:
	ldmfd sp!,{r4-r6,lr}
	bx lr

InstallHack:
	and r2,r0,#0x0F
	rsb r2,r2,#0x10
	strb r2,[r1,#0x18]			;@ Change compare value for branch distance.

	mov r0,r0,lsr#4
	adr r3,HackOpcodes
	ldr r2,[r3,r0,lsl#2]
	ldr r3,[r2]
	str r3,[r1]
	ldr r3,[r2,#4]
	str r3,[r1,#4]
	ldr r3,[r2,#8]
	str r3,[r1,#8]
	ldrb r3,[r2,#15]
	strb r3,[r1,#15]
	add r0,r0,#0x60
	b tlcs900HRedirectOpcode	;@ Insert new pointer to hack opcode in optable

OriginalOpcodes:
	.long sngJR_never, sngJR_lt,      sngJR_le,      sngJR_ule,      sngJR_ov,  sngJR_mi,      sngJR_z,       sngJR_c
	.long sngJR,       sngJR_ge,      sngJR_gt,      sngJR_ugt,      sngJR_nov, sngJR_pl,      sngJR_nz,      sngJR_nc
HackOpcodes:
	.long sngJR_never, sngJR_lt_hack, sngJR_le,      sngJR_ule,      sngJR_ov,  sngJR_mi_hack, sngJR_z_hack,  sngJR_c_hack
	.long sngJR_hack,  sngJR_ge,      sngJR_gt_hack, sngJR_ugt_hack, sngJR_nov, sngJR_pl,      sngJR_nz_hack, sngJR_nc_hack

SpeedHacks:
	.byte 0x00,0x00	;@ #000
	.byte 0xE8,0x00	;@ #001 King Of Fighters - Round 1 (W)
	.byte 0x63,0x74	;@ #002 Neogeo Cup '98 (W)
	.byte 0x00,0x00	;@ #003
	.byte 0x00,0x00	;@ #004 Melon-chan no Seichou Nikki (J)
	.byte 0x68,0x00	;@ #005 Shougi no Tatsujin (J)
	.byte 0x00,0x00	;@ #006
	.byte 0x74,0x00	;@ #007 Baseball Stars (W)
	.byte 0x69,0x00	;@ #008 Samurai Shodown! - Pocket Fighting Series (W)
	.byte 0x00,0x00	;@ #009 Pocket Tennis (W)
	.byte 0xE9,0x00	;@ #010 BioMotor Unitron (J) (C)
	.byte 0x68,0x00	;@ #011 Real Bout Fatal Fury Contact (W) (C)
	.byte 0xE9,0x00	;@ #012 Renketsu Puzzle Tsunagete Pon! (J)
	.byte 0x00,0x00	;@ #013
	.byte 0x88,0x00	;@ #014 Neo Cherry Master - Real Casino Series (W)
	.byte 0x6C,0x00	;@ #015 Neo Dragon's Wild (W) (C)
	.byte 0x88,0x00	;@ #016 Neo Mystery Bonus - Real Casino Series (W) (C)
	.byte 0x78,0x00	;@ #017 Neo Derby Champ Daiyosou (J) (C)
	.byte 0x00,0x00	;@ #018
	.byte 0x00,0x00	;@ #019
	.byte 0x58,0x68	;@ #020 Puzzle Bobble Mini (E) (C)
	.byte 0xE9,0xA9	;@ #021 Metal Slug - 1st Mission (W) (C)
	.byte 0x00,0x00	;@ #022
	.byte 0xE8,0x00	;@ #023 King Of Fighters - Round 2 (W) (C)
	.byte 0x88,0x00	;@ #024 Neo Cherry Master Color - Real Casino Series (W) (C)
	.byte 0x74,0x00	;@ #025 Baseball Stars Color (W) (C)
	.byte 0x66,0x00	;@ #026 Neo Poke Pro Yakyuu (J) (C)
	.byte 0x68,0x00	;@ #027 Shougi no Tatsujin Color (J) (C)
	.byte 0x00,0x00	;@ #028 Pocket Tennis Color (W) (C)
	.byte 0xE9,0x00	;@ #029 Renketsu Puzzle Tsunagete Pon Color (J) (C)
	.byte 0x78,0x00	;@ #030 Samurai Shodown! 2 - Pocket Fighting Series (W) (C)
	.byte 0x68,0x00	;@ #031 Bust-A-Move Pocket (U) (C)
	.byte 0x66,0x00	;@ #032 Party Mail (J) (C)
	.byte 0x69,0x00	;@ #033 Dokodemo Mahjong (J) (C)
	.byte 0x00,0x00	;@ #034
	.byte 0x58,0x00	;@ #035 Neo Turf Masters (W) (C)
	.byte 0x00,0x00	;@ #036 Dive Alert - Burn's Version (J) (C)
	.byte 0x00,0x00	;@ #037 Dive Alert - Rebecca's Version (J) (C)
	.byte 0xB3,0x00	;@ #038 Crush Roller (W) (C)
	.byte 0x63,0x00	;@ #039 Neo Geo Cup '98 Plus (W) (C)
	.byte 0x65,0x00	;@ #040 Shanghai Mini (W) (C)
	.byte 0x69,0x00	;@ #041 Puyo Pop (W) (C)
	.byte 0x00,0x00	;@ #042
	.byte 0x00,0x00	;@ #043
	.byte 0x68,0x00	;@ #044 Pocket Love If (J) (C)
	.byte 0x14,0x00	;@ #045 Dark Arms - Beast Buster 1999 (W) (C)
	.byte 0x69,0x00	;@ #046 Pachinko Hisshou Guide - Pocket Parlor (J) (C)
	.byte 0x00,0x00	;@ #047
	.byte 0x68,0x00	;@ #048 Magical Drop Pocket (J) (C)
	.byte 0xE9,0x00	;@ #049 Renketsu Puzzle Tsunagete Pon 2 (J) (C)
	.byte 0xE9,0x00	;@ #050 Kikou Seiki Unitron (J) (C)
	.byte 0x68,0x00	;@ #051 Faselei! (J) (C)
	.byte 0x69,0x00	;@ #052 Pachi-Slot Aruze Oukoku Pocket - Hanabi (J) (C)
	.byte 0xE9,0x00	;@ #053 BioMotor Unitron (E) (C)
	.byte 0xE9,0x00	;@ #054 Puzzle Link (E) (C)
	.byte 0x00,0x00	;@ #055 Pacman (W) (C)
	.byte 0xE9,0x00	;@ #056 SNK Vs Capcom - Card Fighters Clash (SNK Version) (J) (C)
	.byte 0xE9,0x00	;@ #057 SNK Vs Capcom - Card Fighters Clash (Capcom Version) (J) (C)
	.byte 0x68,0x00	;@ #058 Magical Drop Pocket (E) (C)
	.byte 0x69,0x7A	;@ #059 Sonic The Hedgehog - Pocket Adventure (W) (C)
	.byte 0x6A,0x00	;@ #060 Densha De GO! 2 On Neogeo Pocket (J) (C)
	.byte 0xE9,0x69	;@ #061 Metal Slug - 2nd Mission (W) (C), 69
	.byte 0x69,0xA9	;@ #062 Mizuki Shigeru Youkai Shashin Kan (J) (C)
	.byte 0x69,0x00	;@ #063 Mezase! Kanji Ou (J) (C)
	.byte 0x58,0x00	;@ #064 Last Blade, The (Bakumatsu Rouman Tokubetsu) (J) (C)
	.byte 0x68,0x00	;@ #065 SNK Gals Fighters (J) (C)
	.byte 0x67,0x00	;@ #066 Big Bang Pro Wrestling/Wrestling Madness (W)(J) (C)
	.byte 0xE9,0x00	;@ #067 SNK Vs Capcom - Card Fighters Clash (SNK Version) (E) (C)
	.byte 0xE9,0x00	;@ #068 SNK Vs Capcom - Card Fighters Clash (Capcom Version) (E) (C)
	.byte 0xE8,0x00	;@ #069 SNK Vs Capcom - The Match Of The Millenium (W) (C)
	.byte 0x88,0x00	;@ #070 Neo 21 - Real Casino Series (E) (C)
	.byte 0x66,0x00	;@ #071 Dynamite Slugger (W) (C)
	.byte 0x00,0x00	;@ #072
	.byte 0x00,0x00	;@ #073
	.byte 0x69,0x00	;@ #074 Pachi-Slot Aruze Oukoku Pocket - Azteca (J) (C)
	.byte 0x00,0x00	;@ #075 Cool Boarders Pocket (W) (C)
	.byte 0xE9,0x00	;@ #076 Puzzle Link 2 (E) (C)
	.byte 0x00,0x00	;@ #077
	.byte 0x68,0x00	;@ #078 Soreike! Hanafuda Doujyou (J) (C)
	.byte 0x65,0x00	;@ #079 Pocket Reversi (J) (C)
	.byte 0x00,0x00	;@ #080 Cotton - Fantastic Night Dreams (J) (C)
	.byte 0x65,0x00	;@ #081 Oekaki Puzzle (J) (C)
	.byte 0x65,0x00	;@ #082 Evolution (J) (C)
	.byte 0x00,0x00	;@ #083 Bikkuriman 2000 - Viva! Pocket Festival! (J) (C)
	.byte 0x69,0x00	;@ #084 Pachi-Slot Aruze Oukoku Pocket - Ward of Lights (J) (C)
	.byte 0x69,0x00	;@ #085 Ogre Battle Gaiden (J) (C)
	.byte 0x00,0x00	;@ #086
	.byte 0x66,0x85	;@ #087 Memories Off Pure (J) (C), 85
	.byte 0x00,0x00	;@ #088 Dive Alert - Matt's Version (E) (C)
	.byte 0x00,0x00	;@ #089 Dive Alert - Becky's Version (E) (C)
	.byte 0x68,0x00	;@ #090 Faselei! (E) (C)
	.byte 0x69,0x00	;@ #091 Koi Koi Mahjong (J) (C)
	.byte 0x18,0x79	;@ #092 King of Fighters Battle De Paradise
	.byte 0x68,0x00	;@ #093 SNK Gals Fighters (E) (C)
	.byte 0x69,0x68	;@ #094 Rockman Battle & Fighters
	.byte 0x58,0x00	;@ #095 Last Blade, The (E) (C)
	.byte 0xE8,0x00	;@ #096 Nigeronpa (J) (C)
	.byte 0x69,0x00	;@ #097 Ganbare NeoPoke Kun (J) (C)
	.byte 0xE8,0x00	;@ #098 Neo Baccarat - Real Casino Series (E) (C)
	.byte 0x65,0x00	;@ #099 Evolution - Eternal Dungeons (E) (C)
	.byte 0xF9,0x00	;@ #100 Cool Cool Jam (J) (C)
	.byte 0x00,0x00	;@ #101
	.byte 0x69,0x00	;@ #102 Pachi-Slot Aruze Oukoku Pocket - Porcano 2 (J) (C)
	.byte 0x00,0x00	;@ #103 Delta Warp (J) (C)
	.byte 0x65,0x00	;@ #104 Pocket Reversi (E) (C)
	.byte 0x00,0x00	;@ #105 Cotton - Fantastic Night Dreams (E) (C)
	.byte 0x65,0x00	;@ #106 Picture Puzzle (E) (C)
	.byte 0x69,0x00	;@ #107 Pachi-Slot Aruze Oukoku Pocket - Delsol 2 (J) (C)
	.byte 0x69,0x00	;@ #108 Pachi-Slot Aruze Oukoku Pocket - Dekahel 2 (J) (C)
	.byte 0x68,0x00	;@ #109 Infinity Cure (J) (C)
	.byte 0x69,0x00	;@ #110 Pachi-Slot Aruze Oukoku Pocket - Oohanabi (J) (C)
	.byte 0x18,0x00	;@ #111 Super Real Mahjong - Premium Collection (J) (C)
	.byte 0x69,0x00	;@ #112 Pachi-Slot Aruze Oukoku Pocket - e-Cup (J) (C)
	.byte 0x00,0x00	;@ #113
	.byte 0x00,0x00	;@ #114
	.byte 0x00,0x00	;@ #115
	.byte 0xE9,0x00	;@ #116 SNK vs. Capcom - Card Fighters 2 - Expand Edition (J) (C)

	.byte 0x00,0x00	;@ #1234 Cool Cool Jam (SAMPLE) (E) (C)
	.byte 0x00,0x00	;@ #9019 Sonic the Hedgehog - Pocket Adventure [sample] (C)
	.byte 0x00,0x00	;@ #9020 SvC F1 TRIAL
noHack:
	.byte 0x00,0x00
	.align 2

;@----------------------------------------------------------------------------
sngJR_lt_hack:				;@ 0x61
;@----------------------------------------------------------------------------
	mov r0,r0
	eor r1,t9f,t9f,lsr#3		;@ S^V
	tst r1,#PSR_V
	beq lt_end
lt_end:
;@----------------------------------------------------------------------------
sngJR_mi_hack:				;@ 0x65
;@----------------------------------------------------------------------------
	mov r0,r0
	mov r0,r0
	tst t9f,#PSR_S
	beq mi_end
mi_end:
;@----------------------------------------------------------------------------
sngJR_z_hack:				;@ 0x66
;@----------------------------------------------------------------------------
	mov r0,r0
	mov r0,r0
	tst t9f,#PSR_Z
	beq z_end
z_end:
;@----------------------------------------------------------------------------
sngJR_c_hack:				;@ 0x67
;@----------------------------------------------------------------------------
	mov r0,r0
	mov r0,r0
	tst t9f,#PSR_C
	beq c_end
c_end:
;@----------------------------------------------------------------------------
sngJR_hack:					;@ 0x68
;@----------------------------------------------------------------------------
	mov r0,r0
	mov r0,r0
	b jr_end
	beq jr_end
jr_end:
;@----------------------------------------------------------------------------
sngJR_gt_hack:				;@ 0x6A
;@----------------------------------------------------------------------------
	eor r1,t9f,t9f,lsr#3		;@ S^V
	orr r1,r1,t9f,lsr#2			;@ Z
	tst r1,#PSR_V
	bne gt_end
gt_end:
;@----------------------------------------------------------------------------
sngJR_ugt_hack:				;@ 0x6B
;@----------------------------------------------------------------------------
	mov r0,r0
	orr r1,t9f,t9f,lsr#1		;@ C|Z
	tst r1,#PSR_C
	bne ugt_end
ugt_end:
;@----------------------------------------------------------------------------
sngJR_nz_hack:				;@ 0x6E
;@----------------------------------------------------------------------------
	mov r0,r0
	mov r0,r0
	tst t9f,#PSR_Z
	bne nz_end
nz_end:
;@----------------------------------------------------------------------------
sngJR_nc_hack:				;@ 0x6F
;@----------------------------------------------------------------------------
	mov r0,r0
	mov r0,r0
	tst t9f,#PSR_C
	bne nc_end
nc_end:

#if GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
	.align 2
#endif
;@----------------------------------------------------------------------------
sngJR_hack0:				;@
;@----------------------------------------------------------------------------
	eor r1,t9f,t9f,lsr#3		;@ S^V
	orr r1,r1,t9f,lsr#2			;@ Z
	tst r1,#PSR_V
	bne sngJR_never
	ldrsb r0,[t9pc],#1
	add t9pc,t9pc,r0
	cmp r0,#-7					;@ Speedhack 0
	moveq t9cycles,#0
	t9fetch 8
;@----------------------------------------------------------------------------
sngJR_hack1:				;@
;@----------------------------------------------------------------------------
	eor r1,t9f,t9f,lsr#3		;@ S^V
	orr r1,r1,t9f,lsr#2			;@ Z
	tst r1,#PSR_V
	bne sngJR_never
	ldrsb r0,[t9pc],#1
	add t9pc,t9pc,r0
	cmp r0,#-7					;@ Speedhack 1
	moveq t9cycles,#0
	t9fetch 8

;@----------------------------------------------------------------------------
z80jrzHack:					;@ Jump if zero
;@----------------------------------------------------------------------------
	ldrsb r0,[z80pc],#1
	tst z80f,#PSR_Z
	beq skipJRZ
	subne z80cyc,z80cyc,#5*CYCLE
	addne z80pc,z80pc,r0
	cmp r0,#-28
	andeq z80cyc,z80cyc,#CYC_MASK
skipJRZ:
	fetch 7
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
