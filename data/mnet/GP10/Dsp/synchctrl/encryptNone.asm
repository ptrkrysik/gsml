*****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
*****************************************************************************
*
* Function: computeA52Asm
*
* Description:
*   Stub for A5/2 encryption. Intended for cases where A5/2 is not authorized.
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

*****************************************************************************
*
* Function: computeA51Asm
*
* Description:
*   Stub for A5/1 encryption. Intended for cases where A5/2 is not authorized.
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

