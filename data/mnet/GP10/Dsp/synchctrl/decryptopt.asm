*****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
******************************************************************************
*
*
*       Revision Date:  01/12/01: fast decryption of uplink data bits using *encryptStream
*
*       USAGE  This routine is C callable and can be called as
*
*       void decryptBits(&g_ULBurstData.data[0],&encryptStream[0],NB_NUM_ENCRYPT_BITS_PER_SIDE - 1);
*
*       C CODE
*               This is the C equivalent of the assembly code. Note that
*               the assembly code is hand optimized and restrictions may
*               apply.
*
*        for ( data = g_ULBurstData.data ;
*              data < g_ULBurstData.data + NB_NUM_ENCRYPT_BITS_PER_SIDE - 1;
*              data++ )
*          {
*             if ( *encryptStream++ )  *data = -*data; 
*          }
*
*
*==============================================================================
	.global _decryptBits
	.text
_decryptBits:
	STW	.D2	B10,*B15--	; push register (for c-callable func)

*** BEGIN Benchmark Timing ***

	STW	.D2	B11,*B15--	; push register (for c-callable func)
|| 	SHR	.S1   A6,1,A1 	; init loop index = # input samples - 4 for latency

	STW	.D2	B12,*B15--	; push register (for c-callable func)
|| 	MV	.S1	A4,A8 	; init input data buffer ptr

	LDB	.D1	*A4++[1],A5	; input next data sample 
||	LDB	.D2	*B4++[1],B0	; input next data sample

	LDB	.D1	*A4++[1],A3	; input next data sample 
||	LDB	.D2	*B4++[1],B1	; input next data sample

      SUB	.S1	A1,1,A1     ; decrement/init loop counter
	NOP	 2

  [B0]SUB	.L1	0,A5,A7     ; decrement/init loop counter
||[!B0]MV	.S1	A5,A7     ; decrement/init loop counter

DECRYPT_LOOP:
	LDB	.D1	*A4++[1],A5	; input next data sample 
||	LDB	.D2	*B4++[1],B0	; input next data sample
||[A1]SUB	.S1	A1,1,A1     ; decrement/init loop counter
||[A1]B	.S2	DECRYPT_LOOP  ;* branch to loop

	STB	.D1	A7,*A8++[1]

	LDB	.D1	*A4++[1],A3	; input next data sample 
||	LDB	.D2	*B4++[1],B1	; input next data sample
||[B1]SUB	.L1	0,A3,A9     ; decrement/init loop counter
||[!B1]MV	.S1	A3,A9     ; decrement/init loop counter

	STB	.D1	A9,*A8++[1]

	NOP

  [B0]SUB	.L1	0,A5,A7     ; decrement/init loop counter
||[!B0]MV	.S1	A5,A7     ; decrement/init loop counter

DECRYPT_LOOP_END:
	NOP

  [B1]SUB	.L1	0,A3,A9     ; decrement/init loop counter
||[!B1]MV	.S1	A3,A9     ; decrement/init loop counter

	STB	.D1	A7,*A8++[1]

DECRYPT_END:
	LDW	.D2	*++B15,B12	; pop register (for c-callable func)

	LDW	.D2	*++B15,B11	; pop register (for c-callable func)
||	B	.S2	B3		; return


	LDW	.D2	*++B15,B10	; pop register (for c-callable func)

	NOP	4
