*****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/

; declare encryptionCapability and set it to A5/1 and A5/2 (bits 1 and 0)
 .def _g_encryptionCapability
 .data
_g_encryptionCapability: .word 3     

*****************************************************************************
*
* Function: computeA52Asm
*
* Description:
*   Computes two 114-bit encryption blocks. Block 1 will be used to encrypt
*   the downlink. Block 2 will be used to decrypt the uplink.
*
* Inputs:
*   key			ptr to array containing 64-bit encryption key
*   count		GSM-specified count based on FN
*
* Outputs:
*   block1		ptr to array of 114 bits to encrypt downlink 
*   block2		ptr to array of 114 bits to decrypt uplink  
*
******************************************************************************/
*
*
*       Revision Date:  08/11/99: compute the output bit
*
*       USAGE  This routine is C callable and can be called as
*
*       void computeA52Asm(Char Inputs, Char Outputs)
*
*       C CODE
*               This is the C equivalent of the assembly code. Note that
*               the assembly code is hand optimized and restrictions may
*               apply.
*
*
*       DESCRIPTION
*
*
*==============================================================================

        .global _computeA52Asm
        .text
                        
*** BEGIN Benchmark Timing ***
_computeA52Asm:

        ADD     .L1X    -4,     B15,    A1      ; copy stack pointer
||      STW     .D2     A14,    *-B15[10]        ; push A14 on stack

        STW     .D1     A10,    *A1--[2]        ; push A10 on stack
||      STW     .D2     B10,    *B15--[2]       ; push B10 on stack

        STW     .D1     A11,    *A1--[2]        ; push A11 on stack
||      STW     .D2     B11,    *B15--[2]       ; push B11 on stack
||      MV      .L1     A8,     A0		; init loop index

        STW     .D1     A12,    *A1--[2]        ; push A12 on stack
||      STW     .D2     B12,    *B15--[2]       ; push B12 on stack

        STW     .D1     A13,    *A1--[2]        ; push A13 on stack
||      STW     .D2     B13,    *B15--[2]       ; push B13 on stack

        STW     .D1     A15,    *A1             ; push A15 on stack
||      STW     .D2     B14,    *B15--[3]       ; push B14 on stack
||      MVK     .S1     0200h,  A11
||      MVK     .S2     08000h, B12
INIT:

        MV      .L2X    A4, B10 			;input data buffer start addr
||      MV      .L1X    B4, A3 			;output data buffer start addr

        MVK     .S1     0, A4				;init reg  r1
||      MVK     .S2     0, B4				;init reg r2

        MVK     .S1     0, A6				;init reg r3
||      MVK     .S2     0, B6				;init reg r4

        MVK     .S1     86, A1			;init load_in loop index

        MVK     .S1     02000h, A13
||      MVK     .S2     00040h, B13
||      MPYSU   .M1X    8,  B12, A14            ; 1 << 18
||      MPYSU   .M2     2,  B12, B0             ; 1 << 16

        MVK     .S1     04000h, A12
||      MPY     .M1     A13, A4, A7             ; r1 >> 2
||      MPY     .M2X    A13, B4, B7             ; r2 >> 2

        MVK     .S1     00010h, A15
||      MVKH    .S2     00000h, B12

        SHL     .S1     A14, 3,A2               ; 1 << 21
||      SHL     .S2     B0, 6, B14              ; 1 << 22

STEPLOOP:
        EXTU    .S1     A4,26,16,A8             ; r1 >> 5
||      EXTU    .S2     B4,16,16,B8             ; r2 >> 15
||      MPYU    .M1     A12, A4, A9             ; r1 >> 1
||      MPYU    .M2X    A12, B4, B9             ; r2 >> 1
||      SUB     .D1     1,   A1 ,A1             ; decrement loop index
|| 	  LDB	    .D2	*B10++, B1			; load inval input as decision bit

;2
        EXTU    .S1     A4,31,16,A0             ; r1     
||      XOR     .L1     A7,  A8, A8             ; r1 ^
||      XOR     .L2     B7,  B8, B8             ; r2 ^ 
||      MPYU    .M1     A12, A6, A5             ; r3 >> 1
||      MPYU    .M2X    A11, B6, B5             ; start of r4 >> 5(need 1 more sfl)

;3
        EXTU    .S2     B4,31,16,B2             ; r2     
||      XOR     .L1     A9,  A8, A8             ; r1 ^
||      XOR     .L2     B9,  B8, B8             ; r2 ^ 
||      MPYU    .M1X    B12, A6, A7             ; r3
||      MPYU    .M2     B12, B6, B7             ; r4 

;4
        XOR     .L1     A8,  A0, A8             ; r1 ^
||      XOR     .L2     B8,  B2, B8             ; r2 ^ 
||      MPY     .M2     2,   B5, B5             ; finish r4 >> 5(1 more sfl)

;5
        AND     .L1X    A8, B12, A8             ; (r1 ^) & 1
||      AND     .L2     B8, B12, B8             ; (r2 ^) & 1

;5''
    [A1]B       .S1     STEPLOOP   
||      SHL     .S2     B8, 8, B8               ; r2 feedback << 8
||      XOR     .L1     A7,  A5, A7             ; r3 ^
||      XOR     .L2     B7,  B5, B7             ; r4 ^ 
||      MPYU    .M1     A15, A8, A8             ; r1 feedback << 4

;5'
        AND     .L1X    A7, B12, A7             ; (r3 ^) & 1
||      AND     .L2     B7, B12, B7             ; (r4 ^) & 1

;6
        XOR     .L1     A4,  A8, A8             ; r1 semifinal
||      XOR     .L2     B4,  B8, B8             ; r2 semifinal
||      SHL     .S1     A7,  7,  A7             ; r3 feedback << 7
||      SHL     .S2     B7,  2,  B7             ; r4 feedback << 2

;7
        EXTU    .S1     A8, 1, 2,A4             ; r1 final
||      EXTU    .S2     B8, 1, 2,B4             ; r2 final
||      XOR     .L1     A6,  A7, A7             ; r3 semifinal
||      XOR     .L2     B6,  B7, B7             ; r4 semifinal
||      MPY     .M1X    8, B13,  A11            ; A11 = 0200h

;8
        EXTU    .S1     A7, 1, 2,A6             ; r3 final
||      EXTU    .S2     B7, 1, 2,B6             ; r4 final
||  [B1]XOR     .L1     A4, A14, A4             ; r1 ^= 1 << 18
||  [B1]XOR     .L2     B4, B14, B4             ; r2 ^= 1 << 22

;8'
        MPY     .M1     A13, A4, A7             ; r1 >> 2
||      MPY     .M2X    A13, B4, B7             ; r2 >> 2
||  [B1]XOR     .L1     A6,  A2, A6             ; r3 ^= 1 << 21
||  [B1]XOR     .L2     B6,  B0, B6             ; r4 ^= 1 << 16

ENDLOOP:

        MVK     .S1     020h, A10
||      MVK     .S2     010h, B2

         OR     .L1     8,  A4, A4              ; r1 |= 0x8
||       OR     .L2     B13, B6,B6              ; r4 |= 0x40
||       OR     .S1     A10, A6,A6              ; r3 |= 0x20
||       OR     .S2     B2, B4,B4               ; r2 |= 0x10

	  SUB	    .S1     A3, 1, A3			;adjust output data ptr for inplace calc
        MVK     .S1     0148h, A1			;init loop index

        MVK     .S1     0200h,  A11
||      MVK     .S2     08000h, B12

        MVK     .S1     02000h, A13
||      MVK     .S2     00040h, B13

        MVK     .S1     04000h, A12
||      MVK     .S2     02240h, B14
||      MV      .L1X    B14, A14
||      MPYU    .M1     A13, A4, A7             ; r1 >> 2
||      MPYU    .M2X    A13, B4, B7             ; r2 >> 2

        MVK     .S1     00010h, A15
||      MVKH    .S2     00000h, B12
||      MV      .L1X    B14,  A14               ; A14 = 02240h

        MVKH    .S1     08002000h, A13
        MVKH    .S1     04004000h, A12
        MVKH    .S1     02000010h, A15
STEPLOOP1:
        EXTU    .S1     A4,26,16,A8             ; r1 >> 5
||      EXTU    .S2     B4,16,16,B8             ; r2 >> 15
||      AND     .L1X    B6,  A14,A10            ; r4 & 02240h       
||      AND     .L2     B6,  B14,B10            ; r4 & 02240h       
||      MPYU    .M1     A12, A4, A9             ; r1 >> 1
||      MPYU    .M2X    A12, B4, B9             ; r2 >> 1
||      SUB     .D1     1,   A1 ,A1             ; decrement loop index
||	  MV	    .D2	B5,  B2			; save temp output temporary
;2
        EXTU    .S1     A4,31,16,A0             ; r1     
||      MVK     .S2     02200h, B11             ; case 0x2200 setup
||      XOR     .L1     A7,  A8, A8             ; r1 ^
||      XOR     .L2     B7,  B8, B8             ; r2 ^ 
||      MPYU    .M1     A12, A6, A5             ; r3 >> 1
||      MPYU    .M2X    A11, B6, B5             ; start of r4 >> 5(need 1 more sfl)
||      SUB     .D1     A10, A11,A2             ; test case 0x200
||      SUB     .D2     B10, B13,B0             ; test case 0x40 

;3
        EXTU    .S2     B4,31,16,B2             ; r2     
||      MVK     .S1     02040h, A11             ; case 0x2040 setup
||      XOR     .L1     A9,  A8, A8             ; r1 ^
||      XOR     .L2     B9,  B8, B8             ; r2 ^ 
||      MPYU    .M1X    B12, A6, A7             ; r3
||      MPYU    .M2     B12, B6, B7             ; r4 
||	  STH	    .D1	B2,  *A3[1]			; save preproc output temporary offchip
||  [B0]SUB     .D2     B10, B11,B0             ; test case 0x2200

;4
        XOR     .S1X    A0,  B2, A9             ; r1 ^ r2
||      MVK     .S2     0240H,  B11             ; case 0x240 setup
||      XOR     .L1     A8,  A0, A8             ; r1 ^
||      XOR     .L2     B8,  B2, B8             ; r2 ^ 
||      MPYHLU  .M1     A13, A4,A14             ; r1 >> 4
||      MPY     .M2     2,   B5, B5             ; finish r4 >> 5(1 more sfl)
||  [A2]SUB     .D1     A10, A11,A2             ; test case 0x2040

;5
        XOR     .S1     A7,  A9, A9             ; r1 ^ r2 ^ r3
||      MVK     .S2     02000h,  B2             ; case 0x2000 setup
||      AND     .L1X    A8, B12, A8             ; (r1 ^) & 1
||      AND     .L2     B8, B12, B8             ; (r2 ^) & 1
||      MPYHLU  .M1     A12, A6,A10             ; r3 >> 5
||      MPYU    .M2     B13, B4, B9             ; r2 >> 9
||	  ADD	    .D1	A12,A12,A11			; 08008000h into A11
||      SUB     .D2     B10, B11,B1             ; test case 0x240 

;5''
        XOR     .S1X    B12, A14,A14            ; r1 >> 4 ^ 1
||      SHL     .S2     B8, 8, B8               ; r2 feedback << 8
||      XOR     .L1     A7,  A5, A7             ; r3 ^
||      XOR     .L2     B7,  B5, B7             ; r4 ^ 
||      MPYU    .M1     A15, A8, A8             ; r1 feedback << 4
||      MPYHLU  .M2X    A13, B4, B2             ; r2 >> 4
||  [B1]SUB     .D2     B10, B2, B1             ; test case 0x2000

;5'
        XOR     .S1     A11,A10,A10             ; r3 >> 5 ^ 1
||      XOR     .S2     B12, B9, B9             ; r2 >> 9 ^ 1
||      AND     .L1X    A7, B12, A7             ; (r3 ^) & 1
||      AND     .L2     B7, B12, B7             ; (r4 ^) & 1
||      MPYHLU  .M1     A15,A4,  A0             ; r1 >> 6
||      MPY     .M2X    8,  A6, B10             ; r3 >> 12
||	  ADD	    .D2	B13,B13,B11			; 080h into B11
||	  ADD	    .D1	A13, A13,A5			; 1000h into hi A13

;6
        XOR     .L1     A4,  A8, A8             ; r1 semifinal
||      XOR     .L2     B4,  B8, B8             ; r2 semifinal
||      SHL     .S1     A7,  7,  A7             ; r3 feedback << 7
||      SHL     .S2     B7,  2,  B7             ; r4 feedback << 2
||      MPYU    .M1X    B11, A6,A11             ; r3 >> 8
||      MPYHLU  .M2X    A15, B4, B5             ; r2 >> 6
||	  LDH	    .D1	*A3[1], B0			; restore temp saved output for shift to lsbit

	  NOP


;7
    [A1]B       .S2     STEPLOOP1               ; move back later  
||  [B0]EXTU    .S1     A8, 1, 2,A4             ; r1 final
||      XOR     .L2X    A11,B10,B10             ; y ^ z (r3)
||      AND     .L1X    A11,B10,A11             ; y & z (r3)
||      MPYHLU  .M1     A5,  A4, A5             ; r1 >> 3

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        XOR     .S1     A14, A0, A0             ; y ^ z (r1)
||      XOR     .S2     B5,  B9, B5             ; y ^ z (r2)
||      AND     .L1     A14, A0,A14             ; y & z (r1)
||      AND     .L2     B5,  B9, B9             ; y & z (r1)

        AND     .L1     A5,  A0, A0             ; r1 done
||      AND     .L2     B5,  B2, B5             ; r2 done
||      AND     .S1X    A10,B10,A10             ; r3 done
||  [A2]EXTU    .S2     B8, 1, 2,B4             ; r2 final

        XOR     .S1     A14, A0, A0             ; y' ^ z' (r1)
||      XOR     .S2     B6,  B7,B10             ; r4 semifinal
||      XOR     .L2     B5,  B9, B5             ; y' ^ z' (r2)
||      XOR     .L1     A11,A10,A10             ; y' ^ z' (r3)
||      MPYU    .M2X    A13, B4, B7             ; r2 >> 2

        XOR     .L1     A9,  A0, A0             ; r0 ^ r1 
||      XOR     .L2X    A10, B5, B5             ; r2 ^ r3
||      XOR     .S1     A6,  A7, A7             ; r3 semifinal
||      EXTU    .S2     B0,16,31,B0             ; move output bit to lsb
||      MPY     .M1X    8, B13,  A11            ; A11 = 0200h..move up later
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;8
    [B1]EXTU    .S1     A7, 1, 2,A6             ; r3 final
||      EXTU    .S2     B10,1, 2,B6             ; r4 final
||      MV      .L1X    B14, A14                ; restore 02240h to A14...move up later
||      XOR     .L2X    A0,  B5, B5             ; output
||      MPYU    .M1     A13, A4, A7             ; r1 >> 2
||	  STB	    .D1	B0,  *A3++			; save final output offchip

        EXTU    .S2     B5,16,31,B5             ; move output bit to lsb
 	  STB	    .D1	B5,  *A3++			; save final output offchip

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

*****************************************************************************
*
* Function: computeA51Asm
*
* Description:
*   Computes two 114-bit encryption blocks. Block 1 will be used to encrypt
*   the downlink. Block 2 will be used to decrypt the uplink.
*
* Inputs:
*   key			ptr to array containing 64-bit encryption key
*   count		GSM-specified count based on FN
*
* Outputs:
*   block1		ptr to array of 114 bits to encrypt downlink 
*   block2		ptr to array of 114 bits to decrypt uplink  
*
******************************************************************************/
*
*
*       Revision Date:  01/27/00: compute the output bit
*
*       USAGE  This routine is C callable and can be called as
*
*       void computeA51Asm(Char Inputs, Char Outputs)
*
*       C CODE
*               This is the C equivalent of the assembly code. Note that
*               the assembly code is hand optimized and restrictions may
*               apply.
*
*
*       DESCRIPTION
*
*
*==============================================================================

        .global _computeA51Asm
        .text
                        
*** BEGIN Benchmark Timing ***
_computeA51Asm:

        ADD     .L1X    -4,     B15,    A1      ; copy stack pointer
||      STW     .D2     A14,    *-B15[10]        ; push A14 on stack

        STW     .D1     A10,    *A1--[2]        ; push A10 on stack
||      STW     .D2     B10,    *B15--[2]       ; push B10 on stack

        STW     .D1     A11,    *A1--[2]        ; push A11 on stack
||      STW     .D2     B11,    *B15--[2]       ; push B11 on stack
||      MV      .L1     A8,     A0		; init loop index

        STW     .D1     A12,    *A1--[2]        ; push A12 on stack
||      STW     .D2     B12,    *B15--[2]       ; push B12 on stack

        STW     .D1     A13,    *A1--[2]        ; push A13 on stack
||      STW     .D2     B13,    *B15--[2]       ; push B13 on stack

        STW     .D1     A15,    *A1             ; push A15 on stack
||      STW     .D2     B14,    *B15--[3]       ; push B14 on stack
||      MVK     .S1     0200h,  A11
||      MVK     .S2     08000h, B12

INITA51:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        MV      .L2X    A4, B10 			;input data buffer start addr
||      MV      .L1X    B4, A3 			;output data buffer start addr

        MVK     .S1     0, A4				;init regs r1,r2,r3
||      MVK     .S2     0, B4

        MVK     .S1     0, A6
||      MVK     .S2     0, B6

        MVK     .S1     86, A8			;init loop counter
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
;  first load in key and count, 86 bits, and stepregs with clock = 0
;
        MVK     .S1     00040h, A13
||      MVK     .S2     00040h, B13

        MVK     .S1     04000h, A12
        MVK     .S2     0, B9
||      MPYU    .M1     A11, A4, A7             ; r1 >> 13
||      MPYU    .M2     B12, B4, B7             ; r2 >> 7

        MVK     .S1     00010h, A15
||      MVKH    .S2     00000h, B12
||      MV      .D1     A8,   A1                ; init loop index

STEPLOOPA51:
        EXTU    .S1     A4,13,9,A8              ; r1 >> 18
||      EXTU    .S2     B9,14,9, B6             ; r1 >> 17     
||	  ADD	    .D1	A6, A6, A5  		; r3 >> 21   
||	  ADD	    .D2	B4, B4, B8 		       ; r2 >> 21   

;2
        EXTU    .S1     A4,15,9,A9              ; r1 >> 16
||      XOR     .L1     A7,  A8, A8             ; r1 (13 ^ 18)
||      XOR     .L2     B7,  B8, B8             ; r2 (7 ^ 21)
||	  ADD	    .S2	B8, B8, B9 		      ; r2 >> 20   
|| 	  LDB	    .D2	*B10++, B1			; load inval input as decision bit
||      SUB     .D1     1,   A1 ,A1             ; decrement loop index

;3
    [A1]B       .S1     STEPLOOPA51   
||      EXTU    .S2     B4,10,9, B2             ; r2 << 1    
||      XOR     .L1     A9,  A8, A8             ; r1 ^ 16
||      XOR     .L2     B9,  B8, B8             ; r2 ^ 20
||	  ADD	    .D1	A5, A5, A7  		; r3 >> 20   

;4
        EXTU    .S1     A6,11,10,A6             ; r3 << 1    
||      XOR     .L1X    A8,  B6, A8             ; r1 ^ 17
||      XOR     .L2     B8,  B4, B8             ; r2 ^ 22
||	  ADD	    .D1	A4, A4, A14 		; r1 << 1   
;||      EXTU    .S2X    A4,14,13, B5            ; r1 << 1     

;5
        EXTU    .S2     B8,9,31, B8             ; r2 feedback done     
||      EXTU    .S1     A8,9,31, A8             ; r1 feedback done     
||      XOR     .L1     A5,  A7, A5             ; r3 (21 ^ 22)

;6
        XOR     .L1     A14,  A8, A4            ; r1 final
||      XOR     .L2     B2,  B8, B4             ; r2 final
||      EXTU    .S1     A5,9,31, A5             ; r3 feedback done     

;7
        XOR     .S1     A5,  A6, A6             ; r3 final
||  [B1]XOR     .L1     A4, 1,   A4             ; r1 ^= 1
||  [B1]XOR     .L2     B4, 1,   B4             ; r2 ^= 1

;8
        MPYU    .M1     A11, A4, A7             ; r1 >> 13
||      MPYU    .M2     B12, B4, B7             ; r2 >> 7
||  [B1]XOR     .S1     A6, 1,   A6             ; r3 ^= 1
||      ADD     .L2X    0,A4,   B9              ; r1 into breg


ENDLOOPA51:
;
;  next perform stepregs runup of r1,r2,r3 100 times with clock = 1
;  finally output 228 bits, stepregs with clock = 1
;

        MVK     .S1     329, A1			;init loop counter
||      MVK     .S2     228, B11			;init output buffer loop index

        MVK     .S1     00040h, A13

        MVK     .S2     0, B9
||      MPYU    .M1     A11, A4, A7             ; r1 >> 13
||      MPYU    .M2     B12, B4, B7             ; r2 >> 7

        MVK     .S1     00010h, A15
||      MVKH    .S2     00000h, B12
;
;  init clock bits
;
;0
        EXTU    .S2     B4,21,31,B10            ; (r2 & 0x400) >> 6 .. start y11     
||      EXTU    .S1     A6,21,31,A10            ; (r3 & 0x400) >> 10 .. finish z11 
||      ADD     .L2X    0,A4,   B9              ; r1 into breg
    
        EXTU    .S2     B9,23,29,B5             ; (r1 & 0x100) >> 16 .. start x9     
||	  ADD	    .D2	B10,B10,B10		      ; finish y11   

STEPLOOPA512:
;1
        EXTU    .S1     A4,13,9,A8              ; r1 >> 18
||      EXTU    .S2     B9,14,9, B6             ; r1 >> 17     
||	  ADD	    .D1	A6, A6, A5  		; r3 >> 21   
||	  ADD	    .D2	B4, B4, B8 		       ; r2 >> 21   
||      OR      .L1X    A10,B10,A10             ; y11 | z11 clock bits
||	  AND	    .L2	4,B5,B5			; finish x9 clock bit

	  OR	    .L1X	A10,B5,A10			; finish clock...x9 | y11 | z11
||	  OR	    .L2X	A10,B5,B10			; finish clock...x9 | y11 | z11
||      XOR     .S1     A5, A8,A13              ; (r1 >> 18) ^ (r3 >> 21)

;2
        EXTU    .S1     A4,15,9,A9              ; r1 >> 16
||      XOR     .L1     A7,  A8, A8             ; r1 (13 ^ 18)
||      XOR     .L2     B7,  B8, B8             ; r2 (7 ^ 21)
||	  ADD	    .S2	B8, B8, B9 		      ; r2 >> 20   
||      SUB     .D1     1,   A1 ,A1             ; decrement loop index
||      SUB     .D2     3, B10, B0              ; case 3 for r1

;3
    [A1]B       .S1     STEPLOOPA512   
||      EXTU    .S2     B4,10,9,B14             ; r2 << 1    
||      XOR     .L1     A9,  A8, A8             ; r1 ^ 16
||      XOR     .L2     B9,  B8, B8             ; r2 ^ 20
||	  ADD	    .D1	A5, A5, A7  		; r3 >> 20   
||  [B0]SUB     .D2     4, B10, B0              ; case 4 for r1

;4
        EXTU    .S1     A6,11,10,A12            ; r3 << 1    
||      XOR     .S2X    A13, B4, B13            ; (r1 >> 18) ^ (r3 >> 21) ^ (r2 >> 22)
||      XOR     .L1X    A8,  B6, A8             ; r1 ^ 17
||      XOR     .L2     B8,  B4, B8             ; r2 ^ 22
||	  ADD	    .D1	A4, A4, A14 		; r1 << 1   
||      SUB     .D2     2, B10, B1              ; case 2 for r2

;5
        EXTU    .S2     B8,9,31, B8             ; r2 feedback done     
||      EXTU    .S1     A8,9,31, A8             ; r1 feedback done     
||      XOR     .L1     A5,  A7, A5             ; r3 (21 ^ 22)
||      CMPLT   .L2X    A1, B11, B2             ; loop index > 101 (post-run-up)?
||      SUB     .D1     1, A10 ,A2              ; case 1 for r3
||  [B1]SUB     .D2     5, B10, B1              ; case 5 for r2

;6
    [B0]XOR     .L1     A14, A8, A4             ; r1 final
||  [B1]XOR     .L2     B14,  B8, B4            ; r2 final
||      EXTU    .S1     A5,9,31, A5             ; r3 feedback done     
||      EXTU    .S2     B13,9,31,B13            ; shift output bit to lsb    
||  [A2]SUB     .D1     6, A10 ,A2              ; case 6 for r3

;7
    [A2]XOR     .S1     A5, A12, A6             ; r3 final
||      ADD     .L2X    0,A4,   B9              ; r1 into breg
||      EXTU    .S2     B4,21,31,B10            ; (r2 & 0x400) >> 6 .. start y11     
||  [B2]STB	    .D1	B13, *A3++			; buffer the output bit after runup
;8
        MPYU    .M1     A11, A4, A7             ; r1 >> 13
||      MPYU    .M2     B12, B4, B7             ; r2 >> 7
||      EXTU    .S1     A6,21,31,A10            ; (r3 & 0x400) >> 10 .. finish z11     
||      EXTU    .S2     B9,23,29,B5             ; (r1 & 0x100) >> 16 .. start x9     
||	  ADD	    .D2	B10,B10,B10		      ; finish y11   


ENDLOOPA512:

	  NOP

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

