*****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************

*==============================================================================
*
*       TEXAS INSTRUMENTS ,INC.
*
*       VITERBI EQUALIZER - GSM
*
*       Revision Date:  06/12/97
*
*       Revision Date:  02/25/99: converted path transition outputs from 
*				  hard decision to soft-decision(hpines)
*
*       USAGE  This routine is C callable and can be called as
*
*               void viteq(int n, short new_s[], short old[],
*                          short sd[], short pr[], short trans[])
*
*               n       --- decoder input length
*               new_s   --- state metrics at current instant
*               old     --- state metrics at previous instant
*               sd      --- soft decision values of input bit stream
*               pr      --- probability metrics
*               trans   --- path transition of each state
*
*       C CODE
*               This is the C equivalent of the assembly code. Note that
*               the assembly code is hand optimized and restrictions may
*               apply.
*
*               void viteq(int n, short new_s[], short old[],
*                          short sd[], short pr[], short trans[])
*               {
*                int             i, j, k, a, b, alpha0, alpha1;
*                short          *tmp;
*
*                for (i = 0; i < n; i++) {
*                     for (j = 0; j < 8; j++) {
*                          alpha0 = pr[2 * j] - sd[i];
*                          alpha1 = pr[2 * j + 1] - sd[i];
*                          for (k = 0; k < 16; k += 8) {
*                               a = old[2 * j] + alpha0;
*                               b = old[2 * j + 1] + alpha1;
*                               new_s[j + k] = (b > a) ? b : a;
*                               trans[i][j+k] = (b - a); /* new 2/25/99 */
*                               alpha0 = -alpha0;
*                               alpha1 = -alpha1;
*                          }
*                     }
*                tmp = old;
*                old = new_s;
*                new_s = tmp;
*                }
*               }
*
*       DESCRIPTION
*
*               This routine is used for channel equalization for the
*               GSM full rate system, with Viterbi algorithm and soft
*               decision.  The code is of rate 1/2 and of constraint
*               length K = 4.  The equilization formula is
*
*                               ___ k = 3
*                        y(n) = \         H(k)x(n-k)
*                               /__ k = 0
*
*
*       TECHNIQUES
*
*               The k loop is completely unrolled.
*
*       ASSUMPTIONS
*               1.  Rate -> 1/2
*               2.  K = 4
*
*       MEMORY NOTE:
*
*               There are no memory hits.
*
*       CYCLES
*               55*n + 6
*
*==============================================================================

        .global _viteq
        .text
                        
*** BEGIN Benchmark Timing ***
_viteq:

        ADD     .L1X    -4,     B15,    A1      ; copy stack pointer
||      STW     .D2     A14,    *-B15[10]        ; push A14 on stack

        STW     .D1     A10,    *A1--[2]        ; push A10 on stack
||      STW     .D2     B10,    *B15--[2]       ; push B10 on stack

        STW     .D1     A11,    *A1--[2]        ; push A11 on stack
||      STW     .D2     B11,    *B15--[2]       ; push B11 on stack
||      MVK     .S2     3,      B0
||      MV      .L2X    A6,     B5
||      MV      .L1X    B8,     A3		; TRN index init

        STW     .D1     A12,    *A1--[2]        ; push A12 on stack
||      STW     .D2     B12,    *B15--[2]       ; push B12 on stack
||      MV      .L2     B6,     B8		; B8 now input samples

        MVK     .S1     40,     A14             ; 20 halfs
||      STW     .D1     A13,    *A1--[2]        ; push A13 on stack
||      STW     .D2     B13,    *B15--[2]       ; push B13 on stack
||      ADD     .S2X    14,     A3,     B6      ; trn+8 index init

        STW     .D1     A15,    *A1             ; push A15 on stack
||      STW     .D2     B14,    *B15--[3]       ; push B14 on stack


ILOOP:
        ADD     .L2     8,      B4,     B0      ; copy new_s
||      ADD     .L1X    2,      B5,     A6      ; copy old
||      MVK     .S1     1,      A13             ; enables outer loop branch
||      MVK     .S2     9,      B2              ; reload i
;||[!B0] STH     .D1     B8,     *A3++           ; trans[i] = tr
||      MV      .D2     B5,     B10

        ADD     .L2X    2,      A8,     B9      ; copy pr
||      LDH     .D2     *B8++,  A12             ; d = sd[i]
||      B       .S1     JLOOP                   ; for j
||      MVK     .S2     3,      B0              ; priming count
||      ADD     .L1X    B0,     8,      A11     ; copy new_s
||      SUB     .D1     A4,     1,      A4      ; i--

JLOOP:
        ADD     .S1     A0,     A5,     A9      ; a0 = old0 + alpha0,
||      SUB     .L1     A0,     A5,     A7      ; a8 = old0 - alpha0,
||      ADD     .D2     B11,    B5,     B13     ; b0 = old1 + alpha1,
||      SUB     .L2     B11,    B5,     B7      ; b8 = old1 - alpha1,
||[!B0] STH     .D1     B7,     *A11++          ; new_s[j + 8] = a8,
||[B2]  SUB     .S2     B2,     1,      B2      ; j++
                                                ; outerloop branch

        LDH     .D1     *A8++[2],       A10     ;** pr0 = pr[2*j],
||      LDH     .D2     *B9++[2],       B12     ;** pr1 = pr[2* j+1],
||      SUB     .L1X    B13,    A9,     A15     ; (b0 - a0),
||      SUB     .L2X    B7,     A7,     B14     ; (b8 - a8),
||      MPY     .M1     0,      A2,     A2      ; if innerloop prevent
;||      SHL     .S2     B8,     2,      B8      ; tr <<= 2,
condition
        CMPGT   .L1X    B13,    A9,     A1      ; t0 = (b0 > a0),
||      CMPGT   .L2X    B7,     A7,     B1      ; t8 = (b8 > a8),
||      LDH     .D2     *B10++[2],      A0      ;** old0 = old[2*j],
||      LDH     .D1     *A6++[2],       B11     ;** old1 = old[2*j+1],
||[B0]  SUB     .S2     B0,     1,      B0      ; priming count
||[A2]  B       .S1     ILOOP                   ; for i
||[!B2] MPY     .M1     A13,    A4,     A2      ; outer loop branch

***
***  following extra instruction packet required to handle the soft
***  decision outputs...2/25/99..hpines
***
  [!B0] STH     .D1     A15,    *A3++           ; trn(0)  = a15
||[!B0] STH     .D2     B14,    *++B6           ; trn(8)  = b14
||[A1]  MV      .L1X    B13,    A9              ; a0 = (t0) ? b0 : a0,
||[!B1] MPY     .M2X    1,      A7,     B7      ; b8 = (!t8) ? a8 : b8,

        SUB     .D1     A10,    A12,    A5      ;* alpha0 = pr0 - d,
||      SUB     .L2X    B12,    A12,    B5      ;* alpha1 = pr1 - d,
||[!B0] STH     .D2     A9,     *B4++           ; new_s[j]     = a0,
;||[A1]  OR      .S2     2,      B8,     B8      ; tr |= t0 << 1,
||[B2]  B       .S1     JLOOP                   ; for j
||[A2]  MPY     .M1     0,      A13,    A13     ; disables double outer
                                                ; loop branch

JLOOP_END:
        STH     .D1     B7,     *A11++          ; new_s[j + 8] = a8,
||      SUB     .L2X    B10,    A14,    B4      ; reset/swap old & new_s
||      ADD     .S2     -16,    B4,     B5      ;
||      SUB     .L1     A8,     A14,    A8      ; reset pr
;||      ADD     .S1     14,     A3,     A3      ; update trn[0]

ILOOP_END:
;        STH     .D1     A15,    *A3++           ; trn(0)  = a15
;||      STH     .D2     B14,    *++B6           ; trn(8)  = b14

	NOP

        ADDK    .S1     16,     A3              ; update trn[0]
||      ADDAH   .D2     B6,      8,     B6      ; update trn[8]

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

*===============================================================================
*
*	TEXAS INSTRUMENTS, INC.
*
*	FIR4
*
*	Revision Date: 4/17/97
*
*	USAGE	This routine is C Callable and can be called as:
*
*		void fir(short *x, short *h, short *y, int N, int M)
*		
*		x = input array
*		h = coefficient array
*		y = output array
*		N = number of coefficients (MULTIPLE of 4 >= 8)
*		M = number of output samples (M EVEN >= 2)
*
*		If routine is not to be used as a C callable function
*		then all instructions relating to stack should be removed.
*		Refer to comments of individual instructions.  You will also
*		need to initialize values for all of the values passed as these
*		are assumed to be in registers as defined by the calling 
*		convention of the compiler, (refer to the C compiler reference
*		guide).
*
*	C Code 	This is the C equivalent of the Assembly Code without 
*		restrictions.
*
*		Note that the assembly code is hand optimized and restrictions 
*		may apply
*
*		void fir4(short x[], short h[], short y[], int N, int M)
*		{
*			int i, j, sum;
*			
*			for (j = 0; j < M; j++) {
*				sum = 0;
*				for (i = 0; i < N; i++)
*					sum += x[i + j] * h[i];
*				y[j] = sum >> 15;
*			}
*		}
*
*	DESCRIPTION
*		This FIR assumes the number of filter coeficients is a multiple
*		of 4 and the number of output samples is a multiple of 2.  It
*		operates on 16-bit data with a 32-bit accumulate.  This
*		routine has no memory hits regardless of where x, h, and y 
*		arrays are located in memory.  The filter is M output samples
*		and N coefficients.  The assembly routine performs 2 output
*		samples at a time.
*
*
*	TECHNIQUES
*		The inner loop is unrolled four times thus the number of 
*		filter coefficients must be a multiple of four.  The outer
*		loop is unrolled twice so the number of output samples must
*		be a multiple of 2.
*
*		If an odd number of output samples is needed or possible, the
*		final store can either be removed or conditionally executed 
*		depending on whether M is even or odd.  This code would have to 
*		be added to the existing code.
*
*		The outer loop, like the inner loop, is software pipelined as
*		well.  e, o, and p in the comments of the individual 
*		instructions correspond to the epilogue, outer loop, and 
*		prologue respectively.  
*
*		Refer to FIR example in the optimizing assembly chapter of
*		the programmer's guide for more information.
*
*
*	ASSUMPTIONS
*		N MULTIPLE of 4 >= 8
*		M EVEN >= 2
*
*
*	MEMORY NOTE
*		This code has no memory hits regardless of where x and h are
*		located in memory.
*
*	CYCLES	M*(N+8)/2+6
*
*===============================================================================
	.global _firLoPass
	.text
_firLoPass:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***
B_START

	STW	.D2	B11,*B15--	; push register (for c-callable func)
||	SHR	.S1	A8,1,A2		; set up outer loop counter
||	SHL	.S2	B6,1,B10	; used to rst h pointer each outer loop

	STW	.D2	B12,*B15--	; push register (for c-callable func)
||	ADD	.L1X	B10,10,A3	; used to rst x pointer each outer loop
||	ADD	.S2	B10,8,B10	; used to rst h pointer each outer loop
||	ADD	.L2X	A6,4,B11	; set up pointer to y[1], offset 4 since comples

	LDH	.D1	*A4++,B8	; x0 = x[j]
||	ADD	.L2X	A4,4,B1		; set up pointer to x[j+2]
||	ADD	.L1X	B4,2,A8		; set up pointer to h[1]
||	SHR	.S2	B6,2,B12	; set up inner loop counter
||[A2]	SUB	.S1	A2,1,A2		; decrement outer loop counter

  	LDH	.D2	*B1++[2],B0	; x2 = x[j+i+2]
||	LDH	.D1	*A4++[2],A0	; x1 = x[j+i+1]

  	LDH	.D1	*A8++[2],B6	; h1 = h[i+1]
||	LDH	.D2	*B4++[2],A1	; h0 = h[i]

  	LDH	.D1	*A4++[2],A5	; x3 = x[j+i+3]
||	LDH	.D2	*B1++[2],B5	; x0 = x[j+i+4]

OUTLOOP:
  	LDH	.D2	*B4++[2],A7	; h2 = h[i+2]
||	LDH	.D1	*A8++[2],B8	; h3 = h[i+3]
||	ZERO	.L1	A9		; zero out sum0
||	ZERO	.L2	B9		; zero out sum1

  	LDH	.D2	*B1++[2],B0	;* x2 = x[j+i+2]
||	LDH	.D1	*A4++[2],A0	;* x1 = x[j+i+1]
||	SUB	.S2	B12,2,B2	; set up inner loop counter

	LDH	.D1	*A8++[2],B6	;* h1 = h[i+1]
||	LDH	.D2	*B4++[2],A1	;* h0 = h[i]

  	MPY	.M1X	B8,A1,A0	; x0 * h0
||	MPY	.M2X	A0,B6,B6	; x1 * h1
||	LDH	.D1	*A4++[2],A5	;* x3 = x[j+i+3]
||	LDH	.D2	*B1++[2],B5	;* x0 = x[j+i+4]

  [B2]	B	.S1	LOOP		; branch to loop
||	MPY	.M2	B0,B6,B7	; x2 * h1
||	MPY	.M1	A0,A1,A1	; x1 * h0
||	LDH	.D2	*B4++[2],A7	;* h2 = h[i+2]
||	LDH	.D1	*A8++[2],B8	;* h3 = h[i+3]
||[B2]	SUB	.S2	B2,1,B2		;* decrement loop counter

  	ADD	.L1	A0,A9,A9	; sum0 += x0 * h0
||	MPY	.M2X	A5,B8,B8	; x3 * h3
||	MPY	.M1X	B0,A7,A5	; x2 * h2
||	LDH	.D2	*B1++[2],B0	;** x2 = x[j+i+2]
||	LDH	.D1	*A4++[2],A0	;** x1 = x[j+i+1]

LOOP:
  	ADD	.L2X	A1,B9,B9	; sum1 += x1 * h0
||	ADD	.L1X	B6,A9,A9	; sum0 += x1 * h1
||	MPY	.M2	B5,B8,B7	; x0 * h3
||	MPY	.M1	A5,A7,A7	; x3 * h2
||	LDH	.D1	*A8++[2],B6	;** h1 = h[i+1]
||	LDH	.D2	*B4++[2],A1	;** h0 = h[i]

  	ADD	.L2	B7,B9,B9	; sum1 += x2 * h1
||	ADD	.L1	A5,A9,A9	; sum0 += x2 * h2
||	MPY	.M1X	B5,A1,A0	;* x0 * h0
||	MPY	.M2X	A0,B6,B6	;* x1 * h1
||	LDH	.D1	*A4++[2],A5	;** x3 = x[j+i+3]
||	LDH	.D2	*B1++[2],B5	;** x0 = x[j+i+4]

  	ADD	.L2X	A7,B9,B9	; sum1 += x3 * h2
||	ADD	.L1X	B8,A9,A9	; sum0 += x3 * h3
||[B2]	B	.S1	LOOP		;* branch to loop
||	MPY	.M2	B0,B6,B7	;* x2 * h1
||	MPY	.M1	A0,A1,A1	;* x1 * h0
||	LDH	.D2	*B4++[2],A7	;** h2 = h[i+2]
||	LDH	.D1	*A8++[2],B8	;** h3 = h[i+3]
||[B2]	SUB	.S2	B2,1,B2		;** decrement loop counter

  	ADD	.L2	B7,B9,B9	; sum1 += x0 * h3
||	ADD	.L1	A0,A9,A9	;* sum0 += x0 * h0
||	MPY	.M2X	A5,B8,B8	;* x3 * h3
||	MPY	.M1X	B0,A7,A5	;* x2 * h2
||	LDH	.D2	*B1++[2],B0	;*** x2 = x[j+i+2]
||	LDH	.D1	*A4++[2],A0	;*** x1 = x[j+i+1]
	; inner loop branch occurs here

  	ADD	.L2X	A1,B9,B9	;e sum1 += x1 * h0
||	ADD	.L1X	B6,A9,A9	;e sum0 += x1 * h1
||	MPY	.M2	B5,B8,B7	;e x0 * h3
||	MPY	.M1	A5,A7,A7	;e x3 * h2
||	SUB	.D1	A4,A3,A4	;o reset x pointer to x[j]
||	SUB	.D2	B4,B10,B4	;o reset h pointer to h[0]
||[A2]	B	.S1	OUTLOOP		;o branch to outer loop

  	ADD	.D2	B7,B9,B9	;e sum1 += x2 * h1
||	ADD	.L1	A5,A9,A9	;e sum0 += x2 * h2
||	LDH	.D1	*A4++,B8	;p x0 = x[j]
||	ADD	.L2X	A4,4,B1		;p set up pointer to x[j+2]
||	ADD	.S1X	B4,2,A8		;p set up pointer to h[1]

  	ADD	.L2X	A7,B9,B9	;e sum1 += x3 * h2
||	ADD	.L1X	B8,A9,A9	;e sum0 += x3 * h3
||  	LDH	.D2	*B1++[2],B0	;p x2 = x[j+i+2]
||	LDH	.D1	*A4++[2],A0	;p x1 = x[j+i+1]
||[A2]	SUB	.S1	A2,1,A2		;o decrement outer loop counter

  	ADD	.L2	B7,B9,B9	;e sum1 += x0 * h3
||	SHR	.S1	A9,15,A9	;e sum0 >> 15
||	LDH	.D1	*A8++[2],B6	;p h1 = h[i+1]
||	LDH	.D2	*B4++[2],A1	;p h0 = h[i]

	SHR	.S2	B9,15,B9	;e sum1 >> 15
||	LDH	.D1	*A4++[2],A5	;p x3 = x[j+i+3]
||	LDH	.D2	*B1++[2],B5	;p x0 = x[j+i+4]

	STH	.D1	A9,*A6++[4]	;e y[j] = sum0 >> 15, offset 4 since complex
||	STH	.D2	B9,*B11++[4]	;e y[j+1] = sum1 >> 15
	; outer loop branch occurs here
B_END:
*** END Benchmark Timing ***


	LDW	.D2	*++B15,B12	; pop register (for c-callable func)

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return

	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4

