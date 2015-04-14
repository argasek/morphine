; vim: ft=asm68k:ts=8:sw=8:

        XDEF    _StepperFromMap
        XDEF    _FastStepperFromMap
        XDEF    _FastExpandLine8x
        XDEF    _ExpandLine8x
        XDEF    _Increment

        XREF    _UVMapExpanderThreshold

        section ScalingOptimized, code

; a0 [int *] map
; a2 [int *] stepper
; d0 [int] width
; d1 [int] height

half_sfm    equr    d2
min_sfm     equr    d3
max_sfm     equr    d4
wrap_sfm    equr    d5
saved_sfm   equrl   d2-d5/a2

_StepperFromMap:
        movem.l saved_sfm,-(sp)
        subq.l  #1,d1
        mulu.l  d0,d1
        lea     (a0,d0.l*4),a1

        moveq.l #16,half_sfm
        move.l  _UVMapExpanderThreshold,d0
        asl.l   half_sfm,d0
        move.l  d0,max_sfm
        neg.l   d0
        move.l  d0,min_sfm
        move.l  #$01000000,wrap_sfm

.loop:
        move.l  (a1)+,d0
        sub.l   (a0)+,d0

        cmp.l   max_sfm,d0
        ble.s   .min_sfmus
        
        sub.l   wrap_sfm,d0
        bra.s   .ok

.min_sfmus:  
        cmp.l   min_sfm,d0
        bge.s   .ok

        add.l   wrap_sfm,d0
        bra.s   .ok

.ok:
        asr.l   #3,d0
        move.l  d0,(a2)+
        subq.l  #1,d1
        bgt.s   .loop

        movem.l (sp)+,saved_sfm
        rts


; a0 [int *] map
; a2 [int *] stepper
; d0 [int] width
; d1 [int] height

saved_fsfm   equrl   d2-d4/a2

_FastStepperFromMap:
        movem.l saved_fsfm,-(sp)
        move.l  d1,d4
        subq.l  #1,d4
        mulu.l  d0,d4
        lea     (a0,d0.l*4),a1

.loop:
        move.l  (a1)+,d0
        move.l  (a1)+,d1
        move.l  (a1)+,d2
        move.l  (a1)+,d3
        sub.l   (a0)+,d0
        sub.l   (a0)+,d1
        sub.l   (a0)+,d2
        sub.l   (a0)+,d3
        asr.l   #3,d0
        asr.l   #3,d1
        asr.l   #3,d2
        asr.l   #3,d3
        move.l  d0,(a2)+
        move.l  d1,(a2)+
        move.l  d2,(a2)+
        move.l  d3,(a2)+
        subq.l  #4,d4
        bgt.s   .loop

        movem.l (sp)+,saved_fsfm
        rts

; a0 [int16_t *] dst
; a1 [int *] src
; d2 [int] width

half    equr    d3
dx      equr    d4
xs      equr    d5
xe      equr    d6
saved   equrl   d2-d6

_FastExpandLine8x:
        movem.l saved,-(sp)

        move.l  (a1)+,xs
        move.l  (a1)+,xe
        moveq.l #16,half

.loop:
        move.l  xe,dx
        sub.l   xs,dx
        move.l  xs,d0
        asr.l   #3,dx

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  xe,xs
        move.l  (a1)+,xe

        subq.l  #1,d2
        bne.s   .loop

        movem.l (sp)+,saved
        rts

; a0 [int16_t *] dst
; a1 [int *] src
; d2 [int] width

saved_el8x   equrl   d2-d7/a2-a3
wrap_el8x    equr    d7
max_el8x     equr    a2
min_el8x     equr    a3

_ExpandLine8x:
        movem.l saved_el8x,-(sp)

        move.l  (a1)+,xs
        move.l  (a1)+,xe
        moveq.l #16,half
        move.l  _UVMapExpanderThreshold,d0
        asl.l   half,d0
        move.l  d0,max_el8x
        neg.l   d0
        move.l  d0,min_el8x
        move.l  #$01000000,wrap_el8x

        nop

.loop:
        move.l  xe,dx
        sub.l   xs,dx

.plus:
        cmp.l   max_el8x,dx
        ble.s   .min_el8xus
        
        sub.l   wrap_el8x,dx
        bra.s   .ok

.min_el8xus:  
        cmp.l   min_el8x,dx
        bge.s   .ok

        add.l   wrap_el8x,dx
        bra.s   .ok

.ok:
        move.l  xs,d0
        asr.l   #3,dx

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  d0,d1
        lsr.l   half,d0
        move.w  d0,(a0)+
        add.l   dx,d1

        move.l  d1,d0
        lsr.l   half,d1
        move.w  d1,(a0)+
        add.l   dx,d0

        move.l  xe,xs
        move.l  (a1)+,xe

        subq.l  #1,d2
        bne.s   .loop

        movem.l (sp)+,saved_el8x
        rts

; a0 [int *] x
; a1 [int *] dx
; d1 [int] width

_Increment:
.loop   move.l  (a1)+,d0
        add.l   d0,(a0)+
        subq.l  #1,d1
        bne.s   .loop
        rts
