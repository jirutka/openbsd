	.SPACE $PRIVATE$
	.SUBSPA $DATA$,QUAD=1,ALIGN=8,ACCESS=31
	.SUBSPA $BSS$,QUAD=1,ALIGN=8,ACCESS=31,ZERO,SORT=82
	.SPACE $TEXT$
	.SUBSPA $LIT$,QUAD=0,ALIGN=8,ACCESS=44
	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY

	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
; Basic immediate instruction tests.  
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.

	uxor %r4,%r5,%r6
	uxor,sbz %r4,%r5,%r6
	uxor,shz %r4,%r5,%r6
	uxor,sdc %r4,%r5,%r6
	uxor,sbc %r4,%r5,%r6
	uxor,shc %r4,%r5,%r6
	uxor,tr %r4,%r5,%r6
	uxor,nbz %r4,%r5,%r6
	uxor,nhz %r4,%r5,%r6
	uxor,ndc %r4,%r5,%r6
	uxor,nbc %r4,%r5,%r6
	uxor,nhc %r4,%r5,%r6

	uaddcm %r4,%r5,%r6
	uaddcm,sbz %r4,%r5,%r6
	uaddcm,shz %r4,%r5,%r6
	uaddcm,sdc %r4,%r5,%r6
	uaddcm,sbc %r4,%r5,%r6
	uaddcm,shc %r4,%r5,%r6
	uaddcm,tr %r4,%r5,%r6
	uaddcm,nbz %r4,%r5,%r6
	uaddcm,nhz %r4,%r5,%r6
	uaddcm,ndc %r4,%r5,%r6
	uaddcm,nbc %r4,%r5,%r6
	uaddcm,nhc %r4,%r5,%r6

	uaddcmt %r4,%r5,%r6
	uaddcmt,sbz %r4,%r5,%r6
	uaddcmt,shz %r4,%r5,%r6
	uaddcmt,sdc %r4,%r5,%r6
	uaddcmt,sbc %r4,%r5,%r6
	uaddcmt,shc %r4,%r5,%r6
	uaddcmt,tr %r4,%r5,%r6
	uaddcmt,nbz %r4,%r5,%r6
	uaddcmt,nhz %r4,%r5,%r6
	uaddcmt,ndc %r4,%r5,%r6
	uaddcmt,nbc %r4,%r5,%r6
	uaddcmt,nhc %r4,%r5,%r6
