#ifdef __arm__

#include "TLCS900H/TLCS900H_mac.h"

	.global sngJR_hack0
	.global sngJR_hack1

;@----------------------------------------------------------------------------

	.syntax unified
	.arm

	.section .text
	.align 2
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
	.end
#endif // #ifdef __arm__
