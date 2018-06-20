*****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************
*==============================================================================
*
*
*       TCH INTERLEAVER
*
*       Revision Date:  08/25/99
*
*
*       USAGE  This routine is C callable and can be called as
*
*            void intrleav8(Char *in, Bool steal, Int *bindx,Char out[][]);
*
*       CYCLES
*
*==============================================================================

        .global _intrleav8
        .text
                        
*** BEGIN Benchmark Timing ***
_intrleav8:

;	  NOP
;	  NOP
;	  NOP
;	  NOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;        MVK     .S2     080000040h, B15
;        MVKH    .S2     080000040h, B15
;        MVK     .S1     080000100h, A4
;        MVKH    .S1     080000100h, A4
;        MVK     .S2     080000800h, B6
;        MVKH    .S2     080000800h, B6
; 	  MVK	    .S2	1,B4					;STEAL FLAG
;	  MVK	    .S1	01010101h,A8
;	  MVKH    .S1	01010101h,A8
;        STW     .D1     A8,    *A4++                   ;init first dataword in
;        STW     .D1     A8,    *A4--                   ;init first dataword in
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ADD     .L1X    -4,     B15,    A1      ; copy stack pointer
||      STW     .D2     A14,    *-B15[10]        ; push A14 on stack

        STW     .D1     A10,    *A1--[2]        ; push A10 on stack
||      STW     .D2     B10,    *B15--[2]       ; push B10 on stack

        STW     .D1     A11,    *A1--[2]        ; push A11 on stack
||      STW     .D2     B11,    *B15--[2]       ; push B11 on stack
||      MVK     .S1     0,    A9			;init interleaved frame destination addrs 
||      MVK     .S2     116,  B9

        STW     .D1     A12,    *A1--[2]        ; push A12 on stack
||      STW     .D2     B12,    *B15--[2]       ; push B12 on stack
||      MVK     .S1     228,  A7			;init current frame save indices
||      MVK     .S2     285,  B7

        STW     .D1     A13,    *A1--[2]        ; push A13 on stack
||      STW     .D2     B13,    *B15--[2]       ; push B13 on stack
||      MVK     .S1     0,    A5
||      MVK     .S2     57,   B2

        STW     .D1     A15,    *A1             ; push A15 on stack
||      STW     .D2     B14,    *B15--[3]       ; push B14 on stack
||      MVK     .S1     464,  A8			; prev. frame subblock 5 addr ptr
||      MVK     .S2     521,  B8			; 521=464+57..prev frame subblock 6 addr ptr

	  MVK     .S2     692,  B14               ; 692=464+228..prev frame subblock 6 addr ptr
	  LDB     .D2     *B6[B14],A6             ; reload previous frame steal flag

INIT:
        MVK     .S1     56,   A1                ; init loop index
||      MVK     .S2     64,   B0                ; init modulo 64 index
||      MV      .L1X    B6,   A3                ; A5 = intrlv buf base addr
||      MV      .L2X    A4,   B5                ; A5 = input buf base addr

        LDB     .D1     *A4[A5],A11             ; current frame subblock 0 in
||      LDB     .D2     *B5[B2],B11             ; current frame subblock 1 in
||      MVK     .S1     456,   A15			;init modulo 64 buffer edge
||      MV      .L1X    B0,   A0                ; A reg modulo 64 index

	  ADD	    .L1X	B6, A9, A9			 ;init ping-pong destination addr
||	  ADD	    .L2	B6, B9, B9

  	  ADD	    .L1X	B6, A8, A8			 ;init next frame save addr dest
||	  ADD	    .L2	B6, B8, B8

	  NOP	     2

        STB     .D1     A11,  *A9++      ;begin fill 1st frame out
||      STB     .D2     B11,  *B9++      ;begin fill 2nd frame out
||      ADD     .L1     A5,A0,A5         ; modulo 64 index update
||      ADD     .L2     B2,B0,B2 

BLOCK_O1_FILL_LOOP:
        LDB     .D1     *A8,   A13       ;previous frame subblock 4 in
||      LDB     .D2     *B8,   B13       ;previous frame subblock 5 in
||	  SUB	    .S1	A1,  1, A1	     ;decrement loop index

        LDB     .D1     *A4[A7],A12      ; current frame subblock 4 in
||      LDB     .D2     *B5[B7],B12      ; current frame subblock 5 in
||      ADD     .L1     A7,A0,A10        ; modulo 64 index update
||      ADD     .L2     B7,B0,B10
||	  MVK	    .S2	27,  B1	     ; setup index for steal flag insert test
||	  MV	    .S1X	B4,  A2	     ; setup index for steal flag insert test

        LDB     .D1     *A4[A5],A11      ; current frame subblock 0 in
||      LDB     .D2     *B5[B2],B11      ; current frame subblock 1 in
||      ADD     .L1     A5,A0,A14        ; modulo 64 index update
||      ADD     .S2     B2,B0,B14
||  [A1]B	    .S1	BLOCK_O1_FILL_LOOP
||	  CMPEQ   .L2X	A1,  B1, B1	     ; test index for steal flag insert

	  NOP					     ;needed for LDB 4 cycle latency
||  [B1]STB     .D1     A2,  *A9++       ;insert steal flags mid-frame
||  [B1]STB     .D2     B4,  *B9++       ;ditto

	  CMPLT   .L1	A10, A15, A2     ;wraparound test	
||	  CMPLT   .L2X	B10, A15, B1     ;wraparound test
||  [B1]STB     .D1     A2,  *A9++       ;insert steal flags mid-frame
||  [B1]STB     .D2     B4,  *B9++       ;ditto

        STB     .D1     A13, *A9++       ;fill interleaved subblock 0 out
||      STB     .D2     B13, *B9++       ;fill interleaved subblock 1 out
|| [!A2]SUB	    .S1	A10,A15,A7	     ;wraparound buffer
|| [!B1]SUB	    .S2X	B10,A15,B7
||  [A2]MPY     .M1	1,  A10,A7	     ;no wraparound buffer
||  [B1]MV	    .L2	B10,    B7	     ;no wraparound
||	  CMPLT   .L1	A14,A15,A2	     ;wraparound test

        STB     .D1     A12, *A8++       ;save current frame subblock 4 out
||      STB     .D2     B12, *B8++       ;save current frame subblock 5 out
|| [!A2]SUB	    .S1	A14, A15,A5	     ;wraparound buffer
||  [A2]MV	    .L1	A14,     A5	     ;no wraparound
||	  CMPLT   .L2X	B14, A15, B1     ;wraparound test

        STB     .D1     A11, *A9++       ;fill interleaved subblock 0 out
||      STB     .D2     B11, *B9++       ;fill interleaved subblock 1 out
||  [B1]MV	    .L2	B14,    B2	     ;no wraparound
|| [!B1]SUB	    .S2X	B14,A15,B2	     ;yes. wraparound

***
***  FINISH BLOCK01 PROCESSING
***
        LDB     .D1     *A8,   A13       ;previous frame subblock 4 in
||      LDB     .D2     *B8,   B13       ;previous frame subblock 5 in
        LDB     .D1     *A4[A7],A12      ; current frame subblock 4 in
||      LDB     .D2     *B5[B7],B12      ; current frame subblock 5 in

	  NOP	     3

        STB     .D1     A13, *A9++       ;fill interleaved subblock 0 out
||      STB     .D2     B13, *B9++       ;fill interleaved subblock 1 out

        STB     .D1     A12, *A8++       ;save current frame subblock 4 out
||      STB     .D2     B12, *B8++       ;save current frame subblock 5 out
||	  CMPEQ   .L1	A5, 0, A2        ;test if A5=0, i.e., end first blocks 0,1
	
    [A2]B	    .S2	INIT

        MVK     .S1     342,  A7			;init current frame save indices
||      MVK     .S2     399,  B7
        MVK     .S1     114,  A5			;init current frame save indices
||      MVK     .S2     171,  B2
        MVK     .S1     578,  A8			;init current frame save indices
||      MVK     .S2     635,  B8
        MVK     .S1     232,  A9			;init current frame save indices
||      MVK     .S2     348,  B9
	  NOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        NOP
;        B	    .S2	INIT

;        MVK     .S1     228,  A7			;init current frame save indices
;||      MVK     .S2     285,  B7
;        MVK     .S1     0,    A5			;init current frame save indices
;||      MVK     .S2     57,   B2
;        MVK     .S1     464,  A8			;init current frame save indices
;||      MVK     .S2     521,  B8
;||      STW     .D1     A5,    *A4++                   ;init first dataword in
;        MVK     .S1     0,    A9			;init current frame save indices
;||      MVK     .S2     116,  B9
;||      STW     .D1     A5,    *A4--                   ;init first dataword in
;	  NOP
FINISH:
*** 
*** Fix 9/7/99...deinterleave steal flags..insert prev frame steal flags
***
	MVK      .S2     57,  B14               ; 692=464+228..prev frame subblock 6 addr ptr
      STB     .D2     A6,*B6[B14]             ; save current frame steal flag
||    MVK     .S2     116,  B9
	NOP
	ADD      .S2    B14, B9,  B14               ; 692=464+228..prev frame subblock 6 addr ptr
	STB     .D2     A6,*B6[B14]             ; save current frame steal flag
	NOP
	ADD      .S2    B14, B9,  B14               ; 692=464+228..prev frame subblock 6 addr ptr
	STB     .D2     A6,*B6[B14]             ; save current frame steal flag
	NOP
	ADD      .S2    B14, B9,  B14               ; 692=464+228..prev frame subblock 6 addr ptr
	STB     .D2     A6,*B6[B14]             ; save current frame steal flag
	NOP
	MVK      .S2     692,  B14               ; 692=464+228..prev frame subblock 6 addr ptr
	STB     .D2     B4,*B6[B14]             ; save current frame steal flag
	NOP

        LDW     .D2     *++B15, A14             ; pop A15 off stack
||      ADD     .L1X    0,      B15,    A1      ; copy stack pointer

        LDW     .D1     *++A1[2],       A15     ; pop A15 off stack
||      LDW     .D2     *++B15[2],      B14     ; pop B14 off stack

        LDW     .D1     *++A1[2],       A13     ; pop A13 off stack
||      LDW     .D2     *++B15[2],      B13     ; pop B13 off stack

        LDW     .D1     *++A1[2],       A12     ; pop A12 off stack
||      LDW     .D2     *++B15[2],      B12     ; pop B12 off stack

        LDW     .D1     *++A1[2],       A11     ; pop A11 off stack
||      LDW     .D2     *++B15[2],      B11     ; pop B11 off stack
||      B       .S2     B3                      ; return

        LDW     .D1     *++A1[2],       A10     ; pop A10 off stack
||      LDW     .D2     *++B15[2],      B10     ; pop B10 off stack

        NOP     4
*==============================================================================
*
*
*       CCH INTERLEAVER
*
*       Revision Date:  08/25/99
*
*
*       USAGE  This routine is C callable and can be called as
*
*            void intrleav4(Char *in, Bool steal, Int *bindx,Char out[][]);
*
*       DESCRIPTION
*
*==============================================================================

        .global _intrleav4
        .text
                        
*** BEGIN Benchmark Timing ***
_intrleav4:

;	  NOP
;	  NOP
;	  NOP
;	  NOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;        MVK     .S2     080000040h, B15
;        MVKH    .S2     080000040h, B15
;        MVK     .S1     080000100h, A4
;        MVKH    .S1     080000100h, A4
;        MVK     .S2     080000800h, B6
;        MVKH    .S2     080000800h, B6
; 	  MVK	    .S2	1,B4					;STEAL FLAG
;	  MVK	    .S1	01010101h,A8
;	  MVKH    .S1	01010101h,A8
;        STW     .D1     A8,    *A4++                   ;init first dataword in
;        STW     .D1     A8,    *A4--                   ;init first dataword in
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ADD     .L1X    -4,     B15,    A1      ; copy stack pointer
||      STW     .D2     A14,    *-B15[10]        ; push A14 on stack

        STW     .D1     A10,    *A1--[2]        ; push A10 on stack
||      STW     .D2     B10,    *B15--[2]       ; push B10 on stack

        STW     .D1     A11,    *A1--[2]        ; push A11 on stack
||      STW     .D2     B11,    *B15--[2]       ; push B11 on stack
||      MVK     .S1     0,    A9			;init interleaved frame destination addrs 
||      MVK     .S2     116,  B9

        STW     .D1     A12,    *A1--[2]        ; push A12 on stack
||      STW     .D2     B12,    *B15--[2]       ; push B12 on stack
||      MVK     .S1     228,  A7			;init current frame save indices
||      MVK     .S2     285,  B7

        STW     .D1     A13,    *A1--[2]        ; push A13 on stack
||      STW     .D2     B13,    *B15--[2]       ; push B13 on stack
||      MVK     .S1     0,    A5
||      MVK     .S2     57,   B2

        STW     .D1     A15,    *A1             ; push A15 on stack
||      STW     .D2     B14,    *B15--[3]       ; push B14 on stack
INITC:
        MVK     .S1     56,   A1                ; init loop index
||      MVK     .S2     64,   B0                ; init modulo 64 index
||      MV      .L1X    B6,   A3                ; A5 = intrlv buf base addr
||      MV      .L2X    A4,   B5                ; A5 = input buf base addr

        LDB     .D1     *A4[A5],A11             ; current frame subblock 0 in
||      LDB     .D2     *B5[B2],B11             ; current frame subblock 1 in
||      MVK     .S1     456,   A15			;init modulo 64 buffer edge
||      MV      .L1X    B0,   A0                ; A reg modulo 64 index

	  ADD	    .L1X	B6, A9, A9			 ;init ping-pong destination addr
||	  ADD	    .L2	B6, B9, B9

	  NOP	     3

        STB     .D1     A11,  *A9++      ;begin fill 1st frame out
||      STB     .D2     B11,  *B9++      ;begin fill 2nd frame out
||      ADD     .L1     A5,A0,A5         ; modulo 64 index update
||      ADD     .L2     B2,B0,B2 

BLOCK_O1_FILL_LOOPC:
        LDB     .D1     *A4[A7],A12      ; current frame subblock 4 in
||      LDB     .D2     *B5[B7],B12      ; current frame subblock 5 in
||      ADD     .L1     A7,A0,A10        ; modulo 64 index update
||      ADD     .L2     B7,B0,B10
||	  MVK	    .S2	27,  B1	     ; setup index for steal flag insert test
||	  MV	    .S1X	B4,  A2	     ; setup index for steal flag insert test

	  SUB	    .S1	A1,  1, A1	     ;decrement loop index

        LDB     .D1     *A4[A5],A11      ; current frame subblock 0 in
||      LDB     .D2     *B5[B2],B11      ; current frame subblock 1 in
||      ADD     .L1     A5,A0,A14        ; modulo 64 index update
||      ADD     .S2     B2,B0,B14
||  [A1]B	    .S1	BLOCK_O1_FILL_LOOPC
||	  CMPEQ   .L2X	A1,  B1, B1	     ; test index for steal flag insert

	  NOP					     ;needed for LDB 4 cycle latency
||  [B1]STB     .D1     A2,  *A9++       ;insert steal flags mid-frame
||  [B1]STB     .D2     B4,  *B9++       ;ditto

	  CMPLT   .L1	A10, A15, A2     ;wraparound test	
||	  CMPLT   .L2X	B10, A15, B1     ;wraparound test
||  [B1]STB     .D1     A2,  *A9++       ;insert steal flags mid-frame
||  [B1]STB     .D2     B4,  *B9++       ;ditto

        STB     .D1     A12, *A9++       ;fill interleaved subblock 0 out
||      STB     .D2     B12, *B9++       ;fill interleaved subblock 1 out
|| [!A2]SUB	    .S1	A10,A15,A7	     ;wraparound buffer
|| [!B1]SUB	    .S2X	B10,A15,B7
||  [A2]MPY     .M1	1,  A10,A7	     ;no wraparound buffer
||  [B1]MV	    .L2	B10,    B7	     ;no wraparound
||	  CMPLT   .L1	A14,A15,A2	     ;wraparound test

   [!A2]SUB	    .S1	A14, A15,A5	     ;wraparound buffer
||  [A2]MV	    .L1	A14,     A5	     ;no wraparound
||	  CMPLT   .L2X	B14, A15, B1     ;wraparound test

        STB     .D1     A11, *A9++       ;fill interleaved subblock 0 out
||      STB     .D2     B11, *B9++       ;fill interleaved subblock 1 out
||  [B1]MV	    .L2	B14,    B2	     ;no wraparound
|| [!B1]SUB	    .S2X	B14,A15,B2	     ;yes. wraparound

***
***  FINISH BLOCK01 PROCESSING
***
        LDB     .D1     *A4[A7],A12      ; current frame subblock 4 in
||      LDB     .D2     *B5[B7],B12      ; current frame subblock 5 in

	  NOP	     4

        STB     .D1     A12, *A9++       ;fill interleaved subblock 0 out
||      STB     .D2     B12, *B9++       ;fill interleaved subblock 1 out

	  CMPEQ   .L1	A5, 0, A2        ;test if A5=0, i.e., end first blocks 0,1
	
    [A2]B	    .S2	INITC

        MVK     .S1     342,  A7			;init current frame save indices
||      MVK     .S2     399,  B7
        MVK     .S1     114,  A5			;init current frame save indices
||      MVK     .S2     171,  B2
        MVK     .S1     232,  A9			;init current frame save indices
||      MVK     .S2     348,  B9
	  NOP	     2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        NOP
;        B	    .S2	INIT

;        MVK     .S1     228,  A7			;init current frame save indices
;||      MVK     .S2     285,  B7
;        MVK     .S1     0,    A5			;init current frame save indices
;||      MVK     .S2     57,   B2
;        MVK     .S1     464,  A8			;init current frame save indices
;||      MVK     .S2     521,  B8
;||      STW     .D1     A5,    *A4++                   ;init first dataword in
;        MVK     .S1     0,    A9			;init current frame save indices
;||      MVK     .S2     116,  B9
;||      STW     .D1     A5,    *A4--                   ;init first dataword in
;	  NOP
FINISHC:
*** END Benchmark Timing ***

        LDW     .D2     *++B15, A14             ; pop A15 off stack
||      ADD     .L1X    0,      B15,    A1      ; copy stack pointer

        LDW     .D1     *++A1[2],       A15     ; pop A15 off stack
||      LDW     .D2     *++B15[2],      B14     ; pop B14 off stack

        LDW     .D1     *++A1[2],       A13     ; pop A13 off stack
||      LDW     .D2     *++B15[2],      B13     ; pop B13 off stack

        LDW     .D1     *++A1[2],       A12     ; pop A12 off stack
||      LDW     .D2     *++B15[2],      B12     ; pop B12 off stack

        LDW     .D1     *++A1[2],       A11     ; pop A11 off stack
||      LDW     .D2     *++B15[2],      B11     ; pop B11 off stack
||      B       .S2     B3                      ; return

        LDW     .D1     *++A1[2],       A10     ; pop A10 off stack
||      LDW     .D2     *++B15[2],      B10     ; pop B10 off stack

        NOP     4

*==============================================================================
*
*
*       TCH DEINTERLEAVER
*
*       Revision Date:  08/27/99
*
*       USAGE  This routine is C callable and can be called as
*
*               void dntrleav8(&input[0][0], &output[0]) 
*
*
*==============================================================================

        .global _dntrleav8
        .text
                        
*** BEGIN Benchmark Timing ***
_dntrleav8:

;	  NOP
;	  NOP
;	  NOP
;	  NOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;        MVK     .S2     080000040h, B15
;        MVKH    .S2     080000040h, B15
;        MVK     .S1     080000100h, A4
;        MVKH    .S1     080000100h, A4
;        MVK     .S2     080000800h, B6
;        MVKH    .S2     080000800h, B6
;	  MVK	    .S2	1,B4					;STEAL FLAG
;	  MVK	    .S1	01010101h,A8
;	  MVKH    .S1	01010101h,A8
;        STW     .D1     A8,    *A4++                   ;init first dataword in
;        STW     .D1     A8,    *A4--                   ;init first dataword in
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	ADD     .L1X    -4,     B15,    A1      ; copy stack pointer
||      STW     .D2     A14,    *-B15[10]        ; push A14 on stack

	STW     .D1     A10,    *A1--[2]        ; push A10 on stack
||      STW     .D2     B10,    *B15--[2]       ; push B10 on stack
||      MVK     .S1     230,  A6                ;init interleaved frame destination addrs 
||      MVK     .S2     462,  B6

	STW     .D1     A11,    *A1--[2]        ; push A11 on stack
||      STW     .D2     B11,    *B15--[2]       ; push B11 on stack
||      MVK     .S1     0,    A9                ;init deinterleaved frame destination addrs 
||      MVK     .S2     2,    B9

	STW     .D1     A12,    *A1--[2]        ; push A12 on stack
||      STW     .D2     B12,    *B15--[2]       ; push B12 on stack
||      MVK     .S1     0  ,  A7                ;init current frame source indices for save
||      MVK     .S2     314,  B7                ;2*116 + 82 = 314

	STW     .D1     A13,    *A1--[2]        ; push A13 on stack
||      STW     .D2     B13,    *B15--[2]       ; push B13 on stack
||      MVK     .S1     51,    A5               ;init prev frame source addr
||      MVK     .S2     251,   B2

	STW     .D1     A15,    *A1             ; push A15 on stack
||      STW     .D2     B14,    *B15--[3]       ; push B14 on stack
||      MVK     .S1     464,  A8                        ; c oldest frame source/dest addr ptr..subblock 0
||      MVK     .S2     578,  B8                        ; 521=464+57+57..ditto..subblock 1
INIT1:
	MVK     .S1     57,   A1                ; init loop index
||      MVK     .S2     214,  B0                ; init modulo 49 index = 116 + 2 * 49
||      MV      .L2X    A4,   B5                ; A5 = input buf base addr

	MVK     .S1     114,   A15                      ;init modulo 49 buffer wraparound correction
||      MV      .L1X    B0,   A0                ; A reg modulo 64 index

	  ADD     .L1X    B4, A9, A9       ;init ping-pong destination addr
||      ADD     .L2     B4, B9, B9

	  ADD     .L1     A4, A8, A8       ;init next frame save addr dest
||      ADD     .L2X    A4, B8, B8
||	  SUB     .S1     A9, 7, A9        ; prime pre-decrement dest addr ptr
||      SUB     .S2     B9, 7, B9        ; 

BLOCK_O1_FILL_LOOPD:
	  LDB     .D1     *A8,   A13       ;previous frame c in
||      LDB     .D2     *B8,   B13       ;previous frame c in
||      SUB     .S1     A1,  1, A1           ;decrement loop index
||	  ADD     .L1     7, A9,A9         ; increment dest addr ptr
||      ADD     .L2     7, B9,B9         ; 

        LDB     .D1     *A4[A7],A12      ; current frame c in
||      LDB     .D2     *B5[B7],B12      ; current frame c in
||      ADD     .L1     A7,A0,A10        ; modulo 49 index update
||      ADD     .L2     B7,B0,B10

	  LDB     .D1     *A4[A5],A11      ; current frame p in
||      LDB     .D2     *B5[B2],B11      ; current frame p in
||      ADD     .L1     A5,A0,A14        ; modulo 49 index update
||      ADD     .S2     B2,B0,B14

	  NOP                                        ;needed for LDB 4 cycle latency

	  CMPLT   .L1   A10, A6,  A2     ;wraparound test       
||        CMPLT   .L2   B10, B6,  B1     ;wraparound test

	  STB     .D1     A13, *A9       ;fill deinterleaved subblock 0 out
||      STB     .D2     B13, *B9       ;fill deinterleaved subblock 1 out
|| [!A2]SUB         .S1 A10,A15,A7           ;wraparound buffer
|| [!B1]SUB         .S2X        B10,A15,B7
||  [A2]MPY     .M1     1,  A10,A7           ;no wraparound buffer
||  [B1]MV          .L2 B10,    B7           ;no wraparound
||        CMPLT   .L1   A14,A6, A2           ;wraparound test

	  STB     .D1     A12, *A8++       ;save current frame subblock 4 out
||      STB     .D2     B12, *B8++       ;save current frame subblock 5 out
||	  SUB	    .S2	B6,  2,  B6	     ;adjust for edge wrap test
|| [!A2]SUB     .S1     A14, A15,A5      ;wraparound buffer
||  [A2]MV      .L1     A14,     A5      ;no wraparound
||      CMPLT   .L2     B14, B6, B1      ;wraparound test

	  STB     .D1     A11, *A9[4]       ;fill interleaved subblock 0 out
||      STB     .D2     B11, *B9[4]       ;fill interleaved subblock 1 out
||	  SUB	    .L1	A6,  2,  A6	     ;adjust for edge wrap test
||  [B1]MV      .L2     B14,    B2           ;no wraparound
|| [!B1]SUB     .S2X    B14,A15,B2           ;yes. wraparound
||      MVK     .S1     328,   A15           ;increment modulo 49 buffer wraparound correction

BLOCK_23_FILL_SECTION:
	  LDB     .D1     *A8,   A13       ;previous frame c in
||      LDB     .D2     *B8,   B13       ;previous frame c in
||	  ADD     .S1     A0,  A6,A6         ; end buffer test..230,462 + 328
||      ADD     .S2X    A0,  B6,B6         ; ..for A7,B7
||      ADD     .L1     A7,A0,A10        ; modulo 49 index update
||      ADD     .L2     B7,B0,B10

        LDB     .D1     *A4[A7],A12      ; current frame c in
||      LDB     .D2     *B5[B7],B12      ; current frame c in
||      ADD     .S1     A5,A0,A14        ; modulo 49 index update
||      ADD     .S2     B2,B0,B14
||	  CMPEQ   .L1     A10, A6,  A2     ;wraparound test       
||      CMPEQ   .L2     B10, B6,  B1     ;wraparound test

        LDB     .D1     *A4[A5],A11      ; current frame p in
||      LDB     .D2     *B5[B2],B11      ; current frame p in
||	  ADD     .L1     1, A6,A6         ; for A5,B2 test
||      ADD     .L2     1, B6,B6         ; 
||  [A1]B       .S1     BLOCK_O1_FILL_LOOPD
||  [B1]MVK     .S2     232,   B7        ; yes. special wraparound

	  ADD     .D1     1, A9,A9         ; increment dest addr ptr
||      ADD     .D2     1, B9,B9         ; 
||  [A2]MVK     .S1     0,     A7        ;yes. special wraparount
|| [!B1]SUB     .S2X    B10,A15,B7
|| [!A2]SUB     .L1     A10,A15,A7           ;wraparound buffer

        CMPEQ   .L1     A14, A6, A2      ;wraparound test
||      CMPEQ   .L2     B14, B6, B1      ;wraparound test
||      ADD     .S1     1, A6, A6        ;restore for A7,B7
||      ADD     .S2     1, B6, B6        ;

	  STB     .D1     A13, *A9       ;fill deinterleaved subblock 0 out
||      STB     .D2     B13, *B9       ;fill deinterleaved subblock 1 out
||  [B1]MVK     .S2     233,    B2           ;yes special wraparound
|| [!B1]SUB     .L2X    B14,A15,B2           ;yes. wraparound

	  STB     .D1     A12, *A8++       ;save current frame subblock 4 out
||      STB     .D2     B12, *B8++       ;save current frame subblock 5 out
|| [!A2]SUB     .L1     A14, A15,A5          ;wraparound buffer
||      SUB     .L2X    B6, A0,  B6       ;restore b6 limit = 230,462
||  [A2]MVK     .S1     1,       A5       ;yes. special wraparound

	  STB     .D1     A11, *A9[4]       ;fill interleaved subblock 0 out
||      STB     .D2     B11, *B9[4]       ;fill interleaved subblock 1 out
||      SUB     .L1     A6, A0, A6       ;restore A6 limit = 230,462
||      MVK     .S1     114,   A15           ;increment modulo 49 buffer wraparound correction



FINISH1:
*** END Benchmark Timing ***

        LDW     .D2     *++B15, A14             ; pop A15 off stack
||      ADD     .L1X    0,      B15,    A1      ; copy stack pointer

        LDW     .D1     *++A1[2],       A15     ; pop A15 off stack
||      LDW     .D2     *++B15[2],      B14     ; pop B14 off stack

        LDW     .D1     *++A1[2],       A13     ; pop A13 off stack
||      LDW     .D2     *++B15[2],      B13     ; pop B13 off stack

        LDW     .D1     *++A1[2],       A12     ; pop A12 off stack
||      LDW     .D2     *++B15[2],      B12     ; pop B12 off stack

        LDW     .D1     *++A1[2],       A11     ; pop A11 off stack
||      LDW     .D2     *++B15[2],      B11     ; pop B11 off stack
||      B       .S2     B3                      ; return

        LDW     .D1     *++A1[2],       A10     ; pop A10 off stack
||      LDW     .D2     *++B15[2],      B10     ; pop B10 off stack

        NOP     4
*==============================================================================
*
*
*       CCH DEINTERLEAVER
*
*       Revision Date:  08/27/99
*
*       USAGE  This routine is C callable and can be called as
*
*               void dntrleav4(&input[0][0], &output[0]) 
*
*       DESCRIPTION
*
*
*==============================================================================

        .global _dntrleav4
        .text
                        
*** BEGIN Benchmark Timing ***
_dntrleav4:

;	  NOP
;	  NOP
;	  NOP
;	  NOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;        MVK     .S2     080000040h, B15
;        MVKH    .S2     080000040h, B15
;        MVK     .S1     080000100h, A4
;        MVKH    .S1     080000100h, A4
;        MVK     .S2     080000800h, B6
;        MVKH    .S2     080000800h, B6
;	  MVK	    .S2	1,B4					;STEAL FLAG
;	  MVK	    .S1	01010101h,A8
;	  MVKH    .S1	01010101h,A8
;        STW     .D1     A8,    *A4++                   ;init first dataword in
;        STW     .D1     A8,    *A4--                   ;init first dataword in
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	ADD     .L1X    -4,     B15,    A1      ; copy stack pointer
||      STW     .D2     A14,    *-B15[10]        ; push A14 on stack

	STW     .D1     A10,    *A1--[2]        ; push A10 on stack
||      STW     .D2     B10,    *B15--[2]       ; push B10 on stack
||      MVK     .S1     230,  A6                ;init interleaved frame destination addrs 
||      MVK     .S2     462,  B6

	STW     .D1     A11,    *A1--[2]        ; push A11 on stack
||      STW     .D2     B11,    *B15--[2]       ; push B11 on stack
||      MVK     .S1     0,    A9                ;init deinterleaved frame destination addrs 
||      MVK     .S2     2,    B9

	STW     .D1     A12,    *A1--[2]        ; push A12 on stack
||      STW     .D2     B12,    *B15--[2]       ; push B12 on stack
||      MVK     .S1     0  ,  A7                ;init current frame source indices for save
||      MVK     .S2     314,  B7                ;2*116 + 82 = 314

	STW     .D1     A13,    *A1--[2]        ; push A13 on stack
||      STW     .D2     B13,    *B15--[2]       ; push B13 on stack
||      MVK     .S1     51,    A5               ;init prev frame source addr
||      MVK     .S2     251,   B2

	STW     .D1     A15,    *A1             ; push A15 on stack
||      STW     .D2     B14,    *B15--[3]       ; push B14 on stack
INIT4:
	MVK     .S1     57,   A1                ; init loop index
||      MVK     .S2     214,  B0                ; init modulo 49 index = 116 + 2 * 49
||      MV      .L2X    A4,   B5                ; A5 = input buf base addr

	MVK     .S1     114,   A15                      ;init modulo 49 buffer wraparound correction
||      MV      .L1X    B0,   A0                ; A reg modulo 64 index

	  ADD     .L1X    B4, A9, A9       ;init ping-pong destination addr
||      ADD     .L2     B4, B9, B9

	  SUB     .S1     A9, 7, A9        ; prime pre-decrement dest addr ptr
||      SUB     .S2     B9, 7, B9        ; 

BLOCK_O1_FILL_LOOPD4:
        LDB     .D1     *A4[A7],A12      ; current frame c in
||      LDB     .D2     *B5[B7],B12      ; current frame c in
||      ADD     .L1     A7,A0,A10        ; modulo 49 index update
||      ADD     .L2     B7,B0,B10

	  LDB     .D1     *A4[A5],A11      ; current frame p in
||      LDB     .D2     *B5[B2],B11      ; current frame p in
||      ADD     .L1     A5,A0,A14        ; modulo 49 index update
||      ADD     .S2     B2,B0,B14

        SUB     .S1     A1,  1, A1           ;decrement loop index
||	  ADD     .L1     7, A9,A9         ; increment dest addr ptr
||      ADD     .L2     7, B9,B9         ; 

	  NOP                                        ;needed for LDB 4 cycle latency

	  CMPLT   .L1   A10, A6,  A2     ;wraparound test       
||        CMPLT   .L2   B10, B6,  B1     ;wraparound test

	  STB     .D1     A12, *A9       ;fill deinterleaved subblock 0 out
||      STB     .D2     B12, *B9       ;fill deinterleaved subblock 1 out
|| [!A2]SUB         .S1 A10,A15,A7           ;wraparound buffer
|| [!B1]SUB         .S2X        B10,A15,B7
||  [A2]MPY     .M1     1,  A10,A7           ;no wraparound buffer
||  [B1]MV          .L2 B10,    B7           ;no wraparound
||        CMPLT   .L1   A14,A6, A2           ;wraparound test

	  SUB	    .S2	B6,  2,  B6	     ;adjust for edge wrap test
|| [!A2]SUB     .S1     A14, A15,A5      ;wraparound buffer
||  [A2]MV      .L1     A14,     A5      ;no wraparound
||      CMPLT   .L2     B14, B6, B1      ;wraparound test

	  STB     .D1     A11, *A9[4]       ;fill interleaved subblock 0 out
||      STB     .D2     B11, *B9[4]       ;fill interleaved subblock 1 out
||	  SUB	    .L1	A6,  2,  A6	     ;adjust for edge wrap test
||  [B1]MV      .L2     B14,    B2           ;no wraparound
|| [!B1]SUB     .S2X    B14,A15,B2           ;yes. wraparound
||      MVK     .S1     328,   A15           ;increment modulo 49 buffer wraparound correction

BLOCK_23_FILL_SECTIOND4:
  	  ADD     .S1     A0,  A6,A6         ; end buffer test..230,462 + 328
||      ADD     .S2X    A0,  B6,B6         ; ..for A7,B7
||      ADD     .L1     A7,A0,A10        ; modulo 49 index update
||      ADD     .L2     B7,B0,B10

        LDB     .D1     *A4[A7],A12      ; current frame c in
||      LDB     .D2     *B5[B7],B12      ; current frame c in
||      ADD     .S1     A5,A0,A14        ; modulo 49 index update
||      ADD     .S2     B2,B0,B14
||	  CMPEQ   .L1     A10, A6,  A2     ;wraparound test       
||      CMPEQ   .L2     B10, B6,  B1     ;wraparound test

        LDB     .D1     *A4[A5],A11      ; current frame p in
||      LDB     .D2     *B5[B2],B11      ; current frame p in
||	  ADD     .L1     1, A6,A6         ; for A5,B2 test
||      ADD     .L2     1, B6,B6         ; 
||  [B1]MVK     .S2     232,   B7        ; yes. special wraparound
||  [A1]B       .S1     BLOCK_O1_FILL_LOOPD4

	  ADD     .D1     1, A9,A9         ; increment dest addr ptr
||      ADD     .D2     1, B9,B9         ; 
||  [A2]MVK     .S1     0,     A7        ;yes. special wraparount
|| [!B1]SUB     .L2X    B10,A15,B7
|| [!A2]SUB     .L1     A10,A15,A7           ;wraparound buffer

        CMPEQ   .L1     A14, A6, A2      ;wraparound test
||      CMPEQ   .L2     B14, B6, B1      ;wraparound test
||      ADD     .S1     1, A6, A6        ;restore for A7,B7
||      ADD     .S2     1, B6, B6        ;

	  NOP					     ;new filler for 4 deep cch

	  STB     .D1     A12, *A9       ;fill deinterleaved subblock 0 out
||      STB     .D2     B12, *B9       ;fill deinterleaved subblock 1 out
||  [B1]MVK     .S2     233,    B2           ;yes special wraparound
|| [!B1]SUB     .L2X    B14,A15,B2           ;yes. wraparound
|| [!A2]SUB     .L1     A14, A15,A5          ;wraparound buffer
||  [A2]MVK     .S1     1,       A5       ;yes. special wraparound

	  STB     .D1     A11, *A9[4]       ;fill interleaved subblock 0 out
||      STB     .D2     B11, *B9[4]       ;fill interleaved subblock 1 out
||      SUB     .L1     A6, A0, A6       ;restore A6 limit = 230,462
||      SUB     .L2X    B6, A0,  B6       ;restore b6 limit = 230,462
||      MVK     .S1     114,   A15           ;increment modulo 49 buffer wraparound correction



FINISH4:
*** END Benchmark Timing ***

        LDW     .D2     *++B15, A14             ; pop A15 off stack
||      ADD     .L1X    0,      B15,    A1      ; copy stack pointer

        LDW     .D1     *++A1[2],       A15     ; pop A15 off stack
||      LDW     .D2     *++B15[2],      B14     ; pop B14 off stack

        LDW     .D1     *++A1[2],       A13     ; pop A13 off stack
||      LDW     .D2     *++B15[2],      B13     ; pop B13 off stack

        LDW     .D1     *++A1[2],       A12     ; pop A12 off stack
||      LDW     .D2     *++B15[2],      B12     ; pop B12 off stack

        LDW     .D1     *++A1[2],       A11     ; pop A11 off stack
||      LDW     .D2     *++B15[2],      B11     ; pop B11 off stack
||      B       .S2     B3                      ; return

        LDW     .D1     *++A1[2],       A10     ; pop A10 off stack
||      LDW     .D2     *++B15[2],      B10     ; pop B10 off stack

        NOP     4

