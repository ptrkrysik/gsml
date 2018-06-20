*****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************
* OPTIMIZED ASSEMBLY ROUTINES USED IN THE "\BBDATAPROC\CHCODEC" SUBDIRECTORY
*
*===============================================================================
*==============================================================================
*
*
*       Shift Zeros into Shift Reg 
*
*       Revision Date:  6/1/00
*
*       USAGE  This routine is C callable and can be called as
*
*            void shiftZeros(Char *in, Int16 input_mask, 
*                 Int16 numbits,Int16 poly)
*			    
*       C CODE
*		This is the C equivalent of the Assembly Code. 
*	      Note that the assembly code is hand optimized.
*
*       for(i=0; i<numBits; i++)
*       {
*          output = regOut & 0x0001;
*          result = poly * output;
*          regIn = result ^ regOut;
*          regOut = regIn >> 1;
*       }
*==============================================================================
	.global _shiftZeros
	.text
_shiftZeros:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***
B_START1

	STW	.D2	B11,*B15--	; push register (for c-callable func)
||	MVK	.S2	024000h,B6	; init generator polynomial

	STW	.D2	B12,*B15--	; push register (for c-callable func)
||	MVKH	.S2	024000h,B6	; init generator polynomial

	MVK	.S1	2,A2   	; init poly shifter
	MVK	.S2	0154bh,B2	; init loop counter
|| 	MV	.L1	A3,A8

      AND	.L1	1,A4,A1     ; init feedback bit
LOOP:
  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,30,31,A1 ; strip next output bit
||[B2]SUB	.S2	B2,1,B2		; decrement outer loop counter

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,29,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,28,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,27,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,26,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,25,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,24,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,23,31,A1 ; strip next output bit
||[B2]B	.S2	LOOP		;* branch to loop

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,22,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,21,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,20,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||    EXTU	.S1	A4,19,31,A1 ; strip next output bit
||	MVK	.S2	024000h,B6	; init generator polynomial

      SHR	.S1	A4,12,A4    ; shift out last 12 lsbits
||	MVKH	.S2	024000h,B6	; init generator polynomial

LOOP_END:

	; eleven more shifts

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,30,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,29,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,28,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,27,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,26,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,25,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,24,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,23,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,22,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,21,31,A1 ; strip next output bit

  [A1]XOR	.L1X	A4,B6,A4	; poly ^ shiftreg
||	ADD	.L2	B6,B6,B6	; poly << 1
||    EXTU	.S1	A4,20,31,A1 ; strip next output bit

      SHR	.S1	A4,11,A4    ; shift out last 11 lsbits

B_END1:
*** END Benchmark Timing ***


	LDW	.D2	*++B15,B12	; pop register (for c-callable func)

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return

	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4
*
*==============================================================================
*
*
*       GPRS Convolutional Encoder Bit Puncturing
*
*       Revision Date:  4/7/00
*
*
*       USAGE  This routine is C callable and can be called as
*
*            void punctureGPRS(Char *in, Char *out, Const codeScheme);
*
*	         	in []	--- input  bit stream (input)
*	         	out[]	--- punctured bit stream (output)
*	         	codeScheme	--- CS2 or CS3 (input)
*       DESCRIPTION
*
*          For GPRS coding schemes CS-2 or CS-3, puncturing of the convolutionally
*       encoded bits as follows:
*
*         CS-2:  the punctured bits are numbered 4*i+3, i = 3,...,146, except for
*              i = 9,21,33,45,57,69,81,93,105,117,129,141. This pattern has to be 
*              adjusted to the format of the future new TRAU frame format to be used
*              on the Abis interface(i.e. more puncturing to allow RLC signalling
*
*         CS-3:  the punctured bits are numbered 6*i+3, and 6*i+5, i = 2,...,111 
*			    
*       C CODE
*		This is the C equivalent of the Assembly Code for the CS2 case. 
*	      Note that the assembly code is hand optimized.
*
*  		   j = 0;
*              ii = 15;
*    		   jj = 39;
*	         for(i=0; i < (2*NUM_GPRS_CS2_CNV_ENC_IN_BITS); i++)
*	         {
*                if(i != ii)
*                {
*	            cnvEncOutData[j] = cnvEncOutData[i];
*                 j++;
*                }
*                else
*                {
*		      ii += 4;
*                 if(ii == jj)
*                  {
*                    jj += 48;
*                    ii += 4;
*                  }
*		     }
*              }
*==============================================================================
	.global _punctureGPRS
	.text
_punctureGPRS:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***
B_STARTP

	STW	.D2	B11,*B15--	; push register (for c-callable func)
||	MVK	.S1	0, A5  	; init input sample buffer index offset
||	MVK	.S2	48,B6 	; init CS2 exception puncture index offset
|| 	SUB	.L2X	A6,1,B1	; CS2,CS3 flag(0,1) is conditional register

	STW	.D2	B12,*B15--	; push register (for c-callable func)
||	MVK	.S1	16,A3   	; init first puncture index
||[B1]MVK	.S2	18,B5	      ; init CS2 first puncture index

	MV	.L1	A4,A8   	; init input data buffer ptr
||	MVK	.S1	0,A9   	; init output data buffer offset
||[!B1]MVK	.S2	40,B5	      ; init CS2 exception first puncture index 
||    ADD	.L2X	1,A4,B8     ; init input data buffer ptr

	MV	.L1X	B4,A4   	; init output data buffer ptr base addr
||	MVK	.S1	1,A1   	; init puncture flag to "not puncture"
||	MVK	.S2	1,B9   	; init output data buffer offset

  [B1]MVK	.S2	0151h,B2	      ; init CS3 loop index
  [!B1]MVK	.S2	0125h,B2	      ; init CS2 loop index 

  [B1]B	.S2	CS3_PUNCTURE_INIT  ;* branch to CS3 puncturing if CS3
	NOP	5


CS2_PUNCTURE_LOOP:
      LDB   .D1   *A8++[2],A7  ; load next input
||    LDB   .D2   *B8++[2],B7  ; load next input
||[!A1]ADD	.L1	1,A9,A9	; punctured increment output addr ptr
||[!A1]ADD	.L2	1,B9,B9	; punctured increment output addr ptr
||	ADD	.S1	2,A5,A5      ; increment input sample buffer index offset
||[B2]B	.S2	CS2_PUNCTURE_LOOP  ;* branch to loop

      CMPGT	.L1	A3,A5,A1	; input sample index offset = puncture index?
||[B2]SUB	.S2	B2,1,B2	; decrement loop counter

 [!A1]ADD	.L1	4,A3,A3	; puncturing flag set...increment next puncture index

      CMPEQ	.L2X	A3,B5,B0	; puncture index exception?

	NOP
||[B0]ADD	.L1	4,A3,A3	; exception flag set...increment exception index
||[B0]ADD	.L2	B6,B5,B5	; exception flag set...increment exception index

      STB	.D1	A7,*A4[A9]	; always output odd alternate samples
||[A1]STB	.D2	B7,*B4[B9]	; conditionally puncture even alternate samples
||[A1]ADD	.S1	2,A9,A9	; non-punctured increment output addr ptr
||[A1]ADD	.S2	2,B9,B9	; non-punctured increment output addr ptr

CS2_PUNCTURE_LOOP_END:

      B	.S2	CS3_PUNCTURE_LOOP_END  ;* branch to end
	NOP	5

CS3_PUNCTURE_INIT:
	MVK	.S2	1,B1   	; init puncture flag to "not puncture"
CS3_PUNCTURE_LOOP:
      LDB   .D1   *A8++[2],A7  ; load next input
||    LDB   .D2   *B8++[2],B7  ; load next input
||[!B1]ADD	.L1	1,A9,A9	; punctured increment output addr ptr
||[!B1]ADD	.L2	1,B9,B9	; punctured increment output addr ptr
||	ADD	.S1	2,A5,A5      ; increment input sample buffer index offset
||[B2]B	.S2	CS3_PUNCTURE_LOOP  ;* branch to loop

      CMPGT	.L1	A3,A5,A1	; input sample index offset = puncture index?
||    CMPGT	.L2X	B5,A5,B1	; input sample index offset = puncture index?
||[B2]SUB	.S2	B2,1,B2	; decrement loop counter

 [!A1]ADD	.L1	6,A3,A3	; puncturing flag set...increment next puncture index
||[!B1]ADD	.L2	6,B5,B5	; puncturing flag set...increment next puncture index

	AND	.L2X	A1,B1,B1

	NOP

      STB	.D1	A7,*A4[A9]	; always output odd alternate samples
||[B1]STB	.D2	B7,*B4[B9]	; conditionally puncture even alternate samples
||[B1]ADD	.S1	2,A9,A9	; non-punctured increment output addr ptr
||[B1]ADD	.S2	2,B9,B9	; non-punctured increment output addr ptr

CS3_PUNCTURE_LOOP_END:


	LDW	.D2	*++B15,B12	; pop register (for c-callable func)

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return


	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4

*==============================================================================
*
*
*       GPRS Convolutional Decoder Bit Puncturing Restoration
*
*       Revision Date:  4/7/00
*
*       USAGE  This routine is C callable and can be called as
*
*            void restorePunctureGPRS(Char *in, Char *out, Const codeScheme);
*
*	         	in []	--- input punctured demod samples (input)
*	         	out[]	--- output demod samples with punctures 0 filled (output)
*	         	codeScheme	--- CS2 or CS3 (input)
*       DESCRIPTION
*
*          For GPRS coding schemes CS-2 or CS-3, the punctured demodulated samples
*        are replaced with 0 values to allow soft decision convolutional decoding
*        as follows:
*
*         CS-2:  the punctured bits are numbered 4*i+3, i = 3,...,146, except for
*              i = 9,21,33,45,57,69,81,93,105,117,129,141. This pattern has to be 
*              adjusted to the format of the future new TRAU frame format to be used
*              on the Abis interface(i.e. more puncturing to allow RLC signalling
*
*         CS-3:  the punctured bits are numbered 6*i+3, and 6*i+5, i = 2,...,111 
*			    
*       C CODE
*		This is the C equivalent of the Assembly Code for the CS2 case. 
*	      Note that the assembly code is hand optimized.
*
*  		   j = 0;
*              ii = 15;
*    		   jj = 39;
*	         for(i=0; i < (2*NUM_GPRS_CS2_CNV_ENC_IN_BITS); i++)
*	         {
*                if(i != ii)
*                {
*	            cnvEncOutTestData[i] = cnvDecInData[j];
*                 j++;
*                }
*                else
*                {
*	            cnvEncOutTestData[i] = 0;
*		      ii += 4;
*                 if(ii == jj)
*                  {
*                    jj += 48;
*                    ii += 4;
*                  }
*		     }
*              }
*==============================================================================
	.global _restorePunctureGPRS
	.text
_restorePunctureGPRS:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***
B_STARTRP

	STW	.D2	B11,*B15--	; push register (for c-callable func)
||	MVK	.S1	2, A5  	; init input sample buffer index offset
||	MVK	.S2	48,B6 	; init CS2 exception puncture index offset
|| 	SUB	.L2X	A6,1,B1	; CS2,CS3 flag(0,1) is conditional register

	STW	.D2	B12,*B15--	; push register (for c-callable func)
||	MVK	.S1	16,A3   	; init first puncture index
||[B1]MVK	.S2	18,B5	      ; init CS2 first puncture index

	MV	.L1	A4,A8   	; init input data buffer ptr
||	MVK	.S1	0,A9   	; init output data buffer offset
||[!B1]MVK	.S2	40,B5	      ; init CS2 exception first puncture index 
||    ADD	.L2X	0,A4,B8     ; init input data buffer ptr

	MV	.L1X	B4,A4   	; init output data buffer ptr base addr
||    ADD	.L2	1,B4,B4     ; init ouput data buffer ptr base addr
||	MVK	.S1	1,A1   	; init puncture flag to "not puncture"
||	MVK	.S2	1,B9   	; init output data buffer offset

  [B1]MVK	.S2	0151h,B2	      ; init CS3 loop index
  [!B1]MVK	.S2	0125h,B2	      ; init CS2 loop index 

  [B1]B	.S2	CS3_RESTORE_PUNCTURE_INIT  ;* branch to CS3 puncturing if CS3
	NOP	5


CS2_RESTORE_PUNCTURE_LOOP:
      LDB   .D1   *A8[A9],A7  ; load next input
||[A1]LDB   .D2   *B8[B9],B7  ; load next input
||	ADD	.S1	2,A5,A5      ; increment input sample buffer index offset
||[B2]B	.S2	CS2_RESTORE_PUNCTURE_LOOP  ;* branch to loop

	NOP
||[!A1]MVK	.S2	0,B7   	; puncturing flag set...insert 0 puncturing bit

 [!A1]ADD	.L1	4,A3,A3	; puncturing flag set...increment next puncture index
||[B2]SUB	.S2	B2,1,B2	; decrement loop counter

      CMPEQ	.L2X	A3,B5,B0	; puncture index exception?
||[A1]ADD	.S1	2,A9,A9	; non-punctured increment output addr ptr
||[A1]ADD	.S2	2,B9,B9	; non-punctured increment output addr ptr

  [B0]ADD	.L1	4,A3,A3	; exception flag set...increment exception index
||[B0]ADD	.L2	B6,B5,B5	; exception flag set...increment exception index
||[!A1]ADD	.S1	1,A9,A9	; punctured increment output addr ptr
||[!A1]ADD	.S2	1,B9,B9	; punctured increment output addr ptr

      STB	.D1	A7,*A4++[2]	; always output odd alternate samples
||    STB	.D2	B7,*B4++[2]	; conditionally puncture even alternate samples
||    CMPGT	.L1	A3,A5,A1	; input sample index offset = puncture index?

CS2_RESTORE_PUNCTURE_LOOP_END:

      B	.S2	CS3_RESTORE_PUNCTURE_LOOP_END  ;* branch to end
	NOP	5

CS3_RESTORE_PUNCTURE_INIT:
	MVK	.S2	1,B1   	; init puncture flag to "not puncture"
;||	MVK	.S1	0, A5  	; init input sample buffer index offset
CS3_RESTORE_PUNCTURE_LOOP:
      LDB   .D1   *A8[A9],A7  ; load next input
||[B1]LDB   .D2   *B8[B9],B7  ; load next input
||	ADD	.S1	2,A5,A5      ; increment input sample buffer index offset
||[B2]B	.S2	CS3_RESTORE_PUNCTURE_LOOP  ;* branch to loop

	NOP
||[!B1]MVK	.S2	0,B7   	; puncturing flag set...insert 0 puncturing bit

  [!B1]ADD	.S1	1,A9,A9	; punctured increment output addr ptr
||[!B1]ADD	.S2	1,B9,B9	; punctured increment output addr ptr

  [B1]ADD	.S1	2,A9,A9	; non-punctured increment output addr ptr
||[B1]ADD	.S2	2,B9,B9	; non-punctured increment output addr ptr

  [B2]SUB	.S2	B2,1,B2	; decrement loop counter
||    CMPGT	.L1	A3,A5,A1	; input sample index offset = puncture index?
||    CMPGT	.L2X	B5,A5,B1	; input sample index offset = puncture index?

	AND	.L2X	A1,B1,B1
||[!A1]ADD	.S1	6,A3,A3	; puncturing flag set...increment next puncture index
||[!B1]ADD	.S2	6,B5,B5	; puncturing flag set...increment next puncture index
||    STB	.D1	A7,*A4++[2]	; always output odd alternate samples
||    STB	.D2	B7,*B4++[2]	; conditionally puncture even alternate samples

CS3_RESTORE_PUNCTURE_LOOP_END:


	LDW	.D2	*++B15,B12	; pop register (for c-callable func)

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return


	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4

*==============================================================================
*
*
*       CrcEncoder16 Shift Reg 
*
*       Revision Date:  5/1/00
*
*       USAGE  This routine is C callable and can be called as
*
*            void crcShiftReg(Char *in, Int16 numbits,
*                 Int16 input_mask, Int16 poly)
*
*       C CODE
*		This is the C equivalent of the Assembly Code. 
*	      Note that the assembly code is hand optimized.
*
*       for(i=0; i<numSigBits + numParityBits; i++)
*       {
*          output = regOut & 0x0001;
*          result = poly * output;
*          regIn = result ^ regOut;
*          regOut = regIn >> 1;
*          bit = input[i+1] << numParityBits;
*          regOut |= bit;
*       }
*==============================================================================
	.global _crcShiftReg
	.text
_crcShiftReg:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***
B_STARTSR

	STW	.D2	B11,*B15--	; push register (for c-callable func)
|| 	ADD	.S1X	0,B4,A1 	; init loop index = # input samples/2

	STW	.D2	B12,*B15--	; push register (for c-callable func)
|| 	SHR	.S1	A1,1,A1 	; init loop index = # input samples/2

	LDB	.D1	*A4,A2   	; input first data sample 
||    ADD	.L1	1,A4,A8     ; init input data buffer ptr
||    ADD	.L2X	2,A4,B8     ; init input data buffer ptr
||    SUB	.S1	A1,1,A1	 ; decrement/init loop counter

	NOP	4

  	MVK	.S1	0,A4   	; init shift reg = "regout" 
||    MV	.L2X	A2,B0   	; save input as conditional

      LDB   .D1   *A8++[2],A2  ; load next input
||    LDB   .D2   *B8++[2],B7  ; load next input

	NOP	4


shiftReg_LOOP:
      LDB   .D1   *A8++[2],A7  ; load next input
||[B0]OR	.L1	A6,A4,A4     ; regout |= input 
||	AND	.L2X	1,A4,B1      ; regout & 1
||[A1]SUB	.S1	A1,1,A1	 ; decrement loop counter
||[A1]B	.S2	shiftReg_LOOP  ;* branch to loop

   [B1]XOR	.L1X	A4,B6,A5	; regin = poly ^ regout
||[!B1]MV	.S1	A4,A5   	; ..or regin = regout
||     MV	.S2	B7,B0   	; save input as conditional

      EXTU	.S1	A5,0,1,A4    ; regout = regin >> 1
||    LDB   .D2   *B8++[2],B7  ; load next input

	AND	.L2X	1,A4,B1      ; regout & 1
||[A2]OR	.L1	A6,A4,A4     ; regout |= input 

   [B1]XOR	.L1X	A4,B6,A5	; regin = poly ^ regout
||[!B1]MV	.S1	A4,A5   	; ..or regin = regout

       EXTU	.S1	A5,0,1,A4    ; regout = regin >> 1
||     MV	.L1	A7,A2   	; save input as conditional

shiftReg_LOOP_END:

	AND	.L2	1,B4,B2      ; test if num inputs is even
||[B0]OR	.L1	A6,A4,A4     ; regout |= input 

 [!B2]B	.S2	shiftReg_END  ;* branch to loop
	NOP	3

	AND	.L2X	1,A4,B1      ; regout & 1

   [B1]XOR	.L1X	A4,B6,A5	; regin = poly ^ regout
||[!B1]MV	.S1	A4,A5   	; ..or regin = regout

      EXTU	.S1	A5,0,1,A4    ; regout = regin >> 1

shiftReg_END:
	LDW	.D2	*++B15,B12	; pop register (for c-callable func)

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return


	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4

*==============================================================================
*
*
*       CrcEncoderCCH Long Shift Reg 
*
*       Revision Date:  6/1/00
*
*       USAGE  This routine is C callable and can be called as
*
*            void crcLongShiftReg(Char *in, Int16 input_mask, 
*                 Int16 numbits,Int16 poly)
*			    
*       C CODE
*		This is the C equivalent of the Assembly Code. 
*	      Note that the assembly code is hand optimized.
*
*       for(i=0; i<numBits; i++)
*       {
*          output = regOut & 0x0001;
*          result = poly * output;
*          regIn = result ^ regOut;
*          regOut = regIn >> 1;
*          bit = input[i+1] << numParityBits;
*          regOut |= bit;
*       }
*==============================================================================
	.global _crcLongShiftReg
	.text
_crcLongShiftReg:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***

	STW	.D2	B11,*B15--	; push register (for c-callable func)
|| 	ADD	.S1	0,A6,A1 	; init loop index = # input samples/2

	STW	.D2	B12,*B15--	; push register (for c-callable func)
|| 	SHR	.S1	A1,1,A1 	; init loop index = # input samples/2
||    MV	.S2X	A8,B10   	; save poly[1]

	LDB	.D1	*A4,A2   	; input first data sample 
||    ADD	.L1	1,A4,A8     ; init input data buffer ptr
||    SUB	.S1	A1,1,A1     ; decrement/init loop counter

	MVK	.S1	1,A0
	SHL	.S1	A0,31,A0	; A0 = 1 << 31
	MV	.S2	B8,B11	;save regout destination addr for later
      ADD	.L2X	2,A4,B8     ; init input data buffer ptr

  	MVK	.S1	0,A4   	; init shift reg = regout[0] 
  	MVK	.S2	0,B5   	; init shift reg = regout[1]
||    MV	.L2X	A2,B0   	; save input as conditional

      LDB   .D1   *A8++[2],A2  ; load next input
||    LDB   .D2   *B8++[2],B7  ; load next input

  	MVK	.S2	0,B2   	; init regout[0]conditional
	NOP	3


longShiftReg_LOOP:
      LDB   .D1   *A8++[2],A7  ; load next input
||[B0]OR	.S2	B5,B4,B5     ; regout[1] |= input 
||[B2]OR	.L1	A0,A4,A4     ; regout[0] |= (regin[1] << 31) 
||	AND	.L2X	1,A4,B1      ; regout & 1
||[A1]B	.S1	longShiftReg_LOOP  ;* branch to loop

   [B1]XOR	.L1X	A4,B6,A5	; regin[0] = poly[0] ^ regout[0]
||[!B1]ADD	.D1	0,A4,A5   	; ..or regin[0] = regout[0]
|| [B1]XOR	.L2	B5,B10,B9	; regin[1] = poly[1] ^ regout[1]
||[!B1]MV	.S2	B5,B9   	; ..or regin[1] = regout[1]
||     ADD	.D2	0,B7,B0   	; save input as conditional

      EXTU	.S1	A5,0,1,A4    ; regout[0] = regin[0] >> 1
||    EXTU	.S2	B9,0,1,B5    ; regout[1] = regin[1] >> 1
||    LDB   .D2   *B8++[2],B7  ; load next input
||	AND	.L2	1,B9,B2      ; setup (regin[1] << 31) as conditional
||[A1]SUB	.L1	A1,1,A1	 ; decrement loop counter

	AND	.L2X	1,A4,B1      ; regout & 1
||[A2]OR	.S2	B5,B4,B5     ; regout[1] |= input 
||[B2]OR	.L1	A0,A4,A4     ; regout[0] |= (regin[1] << 31) 

   [B1]XOR	.L1X	A4,B6,A5	; regin[0] = poly[0] ^ regout[0]
||[!B1]ADD	.D1	0,A4,A5   	; ..or regin[0] = regout[0]
|| [B1]XOR	.L2	B5,B10,B9	; regin[1] = poly[1] ^ regout[1]
||[!B1]MV	.S2	B5,B9   	; ..or regin[1] = regout[1]

      EXTU	.S1	A5,0,1,A4    ; regout[0] = regin[0] >> 1
||    EXTU	.S2	B9,0,1,B5    ; regout[1] = regin[1] >> 1
||	AND	.L2	1,B9,B2      ; setup (regin[1] << 31) as conditional
||    ADD	.D1	0,A7,A2      ; save input as conditional

longShiftReg_LOOP_END:

	AND	.L2X	1,A6,B2      ; test if num inputs is even
||[B0]OR	.S2	B5,B4,B5     ; regout[1] |= input 
||[B2]OR	.L1	A0,A4,A4     ; regout[0] |= (regin[1] << 31) 

 [!B2]B	.S2	longShiftReg_END  ;* branch to loop
	NOP	2

	AND	.L2X	1,A4,B1      ; regout & 1
	MV	.S1X	B11,A8       ; restore regout[0] destination address

   [B1]XOR	.L1X	A4,B6,A5	; regin[0] = poly[0] ^ regout[0]
||[!B1]ADD	.D1	0,A4,A5   	; ..or regin[0] = regout[0]
|| [B1]XOR	.L2	B5,B10,B9	; regin[1] = poly[1] ^ regout[1]
||[!B1]MV	.S2	B5,B9   	; ..or regin[1] = regout[1]

      EXTU	.S1	A5,0,1,A4    ; regout[0] = regin[0] >> 1
||    EXTU	.S2	B9,0,1,B5    ; regout[1] = regin[1] >> 1

longShiftReg_END:
	LDW	.D2	*++B15,B12	; pop register (for c-callable func)
||	STW	.D1	A4,*A8++[1]	; write regout[0] off chip
||	MV	.S1X	B5,A5 	; move regout[1] to a-reg

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	STW	.D1	A5,*A8++[1]	; write regout[1] off chip
||	B	.S2	B3		; return


	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4
*==============================================================================
*       Copy Bytes from input buffer to output buffer  
*
*       Revision Date:  6/1/00
*
*       USAGE  This routine is C callable and can be called as
*
*            void copyByteSamples(Char *in, Char *out, const numbits) 
*
*       C CODE
*		This is the C equivalent of the Assembly Code. 
*	      Note that the assembly code is hand optimized.
*
*               for(i=0; i<numbits; i++)
*               {
*                *out++ = *in++;
*               }
*
*==============================================================================
	.global _copyByteSamples
	.text
_copyByteSamples:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***

	STW	.D2	B11,*B15--	; push register (for c-callable func)
|| 	SUB	.S1   A6,6,A1 	; init loop index = # input samples - 6

	STW	.D2	B12,*B15--	; push register (for c-callable func)
|| 	MV	.S1	A4,A8 	; init input data buffer ptr

  [A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||	LDB	.D1	*A8++[1],A5	; input first data sample 

  [A1]B	.S2	COPY_BYTE_LOOP  ;* branch to loop
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||	LDB	.D1	*A8++[1],A5	; input next data sample 

  [A1]B	.S2	COPY_BYTE_LOOP  ;* branch to loop
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||	LDB	.D1	*A8++[1],A5	; input next data sample 

  [A1]B	.S2	COPY_BYTE_LOOP  ;* branch to loop
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||	LDB	.D1	*A8++[1],A5	; input next data sample 

  [A1]B	.S2	COPY_BYTE_LOOP  ;* branch to loop
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||	LDB	.D1	*A8++[1],A5	; input next data sample 

  [A1]B	.S2	COPY_BYTE_LOOP  ;* branch to loop
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||	LDB	.D1	*A8++[1],A5	; input next data sample 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter


COPY_BYTE_LOOP:
	STB	.D2	B5,*B4++[1]	; copy to output buffer 
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||	LDB	.D1	*A8++[1],A5	; input next data sample 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter
||[A1]B	.S2	COPY_BYTE_LOOP  ;* branch to loop


COPY_BYTE_LOOP_END:
	STB	.D2	B5,*B4++[1]	; copy to output buffer 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter

	STB	.D2	B5,*B4++[1]	; copy to output buffer 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter

	STB	.D2	B5,*B4++[1]	; copy to output buffer 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter

	STB	.D2	B5,*B4++[1]	; copy to output buffer 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter

	STB	.D2	B5,*B4++[1]	; copy to output buffer 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter

	STB	.D2	B5,*B4++[1]	; copy to output buffer 
||    ADD	.L2X	0,A5,B5     ; decrement/init loop counter

COPY_BYTE_END:
	LDW	.D2	*++B15,B12	; pop register (for c-callable func)

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return


	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4


*==============================================================================
*       Compute Total (Channel) Errors  
*
*       Revision Date:  6/1/00
*
*       USAGE  This routine is C callable and can be called as
*
*            void computeTotalErrors(Char *in, Char *out,  Int16 input_mask, const numbits) 
*
*       C CODE
*		This is the C equivalent of the Assembly Code. 
*	      Note that the assembly code is hand optimized.
*
*               for(i=0; i<NUM_CCH_CNV_ENC_OUT_BITS; i++)
*               {
*                  if((cnvDecInData[i]<0) != cnvEncOutTestData[i])
*                  {
*                     totalErrors += 1;
*                  }
*               }
*
*==============================================================================
	.global _computeTotalErrors
	.text
_computeTotalErrors:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***

	STW	.D2	B11,*B15--	; push register (for c-callable func)
|| 	SUB	.S1   A6,4,A1 	; init loop index = # input samples - 4 for latency

	STW	.D2	B12,*B15--	; push register (for c-callable func)
|| 	MV	.S1	A4,A8 	; init input data buffer ptr
|| 	MV	.S2	B4,B8 	; init input data buffer ptr

	LDB	.D1	*A8++[1],A5	; input next data sample 
||	LDB	.D2	*B8++[1],B5	; input next data sample
|| 	MVK	.S1	0,A6 	      ; init total error counter

  [A1]B	.S2	TOTAL_ERROR_LOOP  ;* branch to loop
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter

	LDB	.D1	*A8++[1],A5	; input next data sample 
||	LDB	.D2	*B8++[1],B5	; input next data sample

  [A1]B	.S2	TOTAL_ERROR_LOOP  ;* branch to loop
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter

	LDB	.D1	*A8++[1],A5	; input next data sample 
||	LDB	.D2	*B8++[1],B5	; input next data sample

TOTAL_ERROR_LOOP:
      XOR	.L1	A5,1,A7     ; decrement/init loop counter
||    CMPLT	.L2	B5,0,B0     ; decrement/init loop counter
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||[A1]B	.S2	TOTAL_ERROR_LOOP  ;* branch to loop

	LDB	.D1	*A8++[1],A5	; input next data sample 
||	LDB	.D2	*B8++[1],B5	; copy to output buffer 
||[B0]ADD	.S1	A6,A7,A6     ; decrement/init loop counter
||[!B0]ADD	.L1	A6,A5,A6     ; decrement/init loop counter


TOTAL_ERROR_LOOP_END:

      XOR	.L1	A5,1,A7     ; decrement/init loop counter
||    CMPLT	.L2	B5,0,B0     ; decrement/init loop counter

  [B0]ADD	.S1	A6,A7,A6     ; decrement/init loop counter
||[!B0]ADD	.L1	A6,A5,A6     ; decrement/init loop counter

      XOR	.L1	A5,1,A7     ; decrement/init loop counter
||    CMPLT	.L2	B5,0,B0     ; decrement/init loop counter

  [B0]ADD	.S1	A6,A7,A6     ; decrement/init loop counter
||[!B0]ADD	.L1	A6,A5,A6     ; decrement/init loop counter

      XOR	.L1	A5,1,A7     ; decrement/init loop counter
||    CMPLT	.L2	B5,0,B0     ; decrement/init loop counter

  [B0]ADD	.S1	A6,A7,A6     ; decrement/init loop counter
||[!B0]ADD	.L1	A6,A5,A6     ; decrement/init loop counter

TOTAL_ERROR_END:
	LDW	.D2	*++B15,B12	; pop register (for c-callable func)
|| 	MV	.S1	A6,A4 	; return the function argument

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return


	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4

