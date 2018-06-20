;****************************************************************************
;   (c) Copyright Cisco 2000
;   All Rights Reserved
;****************************************************************************

;******************************************************************************
;* TMS320C6x ANSI C Codegen                                      Version 2.00 *
;* Date/Time created: Wed Feb  3 14:04:49 1999                                *
;******************************************************************************

;******************************************************************************
;* GLOBAL FILE PARAMETERS                                                     *
;*                                                                            *
;*   Architecture        : TMS320C6200                                        *
;*   Endian              : Little                                             *
;*   Interrupt Threshold : Disabled                                           *
;*   Memory Model        : Small                                              *
;*   Speculative Load    : Disabled                                           *
;*   Redundant Loops     : Enabled                                            *
;*   Pipelining          : Enabled                                            *
;*   Debug Info          : No Debug Info                                      *
;*                                                                            *
;******************************************************************************

FP	.set	A15
DP	.set	B14
SP	.set	B15


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; GMSK Modulator in linear assembly
;
; based on Frontier Design's table modulator
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;Description:
; Look-up-table based GSM GMSK 2 samples per bit modulator.
;
;Method:
; This function generates a GSM GMSK modulated waveform from a sequence
; of bits.  Power ramping is performed outside this routine by placing
; samples at the beginning and end of the burst buffer that aren't modified
; here.
;
; 148 bits are input: The whole burst including tail bits but excluding
; guard period bits.
;
; Output is 148 bits of the burst centered within the burst data buffer.
;
;Limitations: 
;
; The output buffer pointed to, and the modulator table, modLUT2x MUST
; be 32-bit aligned.
;
; It is necessary to unroll the inner loop over samples per bit in
; order to software pipeline the outer loop. This means that we can't
; easily use the SAMPLES_PER_BIT symbol to vary the number of bits
; per sample. Instead this code is written for 2 samples per bit
; specifically. SAMPLES_PER_BIT must be defined as 2 if this routine
; is called, or its associated sample table (mod2spb_tab.h) is used.
;
; Assuming that the number of bits modulated is always even leads
; to a significant optimization in the calculation of the samples
; and updating of the relative phase per bit. This means that both
; NB_BITS_TO_MOD and AB_BITS_TO_MOD must be even for this routine
; to work properly.
;
;Input/Output:
;
; The interface to this routine consists of a pointer to the output
; sample buffer, and a flag indicating the type of burst. Any value
; but RACH is modulated as a normal burst, RACH is an access burst.
; Since local arrays can't be created from linear assembly, a pointer
; to one created in the C stub that calls this routine is passed. In
; order to use the #define CONSTANTS in C, they are passed to this
; routine via a pointer to an int array.
;
; C call: GMSKModulate                      register names here
;         (
;           t_DLComplexSamp *outBurstPtr,       obptr
;           t_ChanSel chansel,                  chsel
;           UChar *diff,                        dfptr
;           Int *c_constants                    ccptr
;         )
;
; obptr -- Register containing the address samples are written to.
;          Two 12 bit samples, I and Q, are packed in the lower 24 bits
;          of a 32 bit word, with I the lowest 12 bits. For some, no
;          doubt good historical reason, the output buffer actually
;          consists of structures containing two 16 bit samples. It
;          is treated here like an array of Int.
;
; chsel -- Type of burst to modulate, if equal to RACH -- an access burst.
;          If equal to anything else, a normal burst. Controls the number
;          of bits that are modulated primarily.
;
; dfptr -- Pointer to an array to be filled with the differentially encoded
;          bits to be modulated. Need to create it in C because linear 
;          assembly does not allow the stack pointer to be manipulated to
;          create local variables.
;
; ccptr -- Pointer to an array filled with #define constants from C, in
;          order to avoid having to redefine them here. Avoids the danger
;          of being modified in C and left the same here.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

           .def _GMSKModLoop            ; declare the function global

           .ref _modLUT2x               ; import addresses from C
           .ref _g_DLBurstData

; define index names in array of constants passed from C
    
C_NUM_MOD_BITS_IN    .set    0  ; import symbol values from C
C_AB_BITS_TO_MOD     .set    1  ; values are integer indices in the
C_NB_BITS_TO_MOD     .set    2  ; ccptr array passed
C_RAMP_OFFSET        .set    3
C_RACH               .set    4
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _GMSKModLoop                                                *
;*                                                                            *
;*   Regs Modified     : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,B0,B1,B2,B4,B5,*
;*                           B6,B7,B8,B9,SP                                   *
;*   Regs Used         : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,B0,B1,B2,B3,B4,*
;*                           B5,B6,B7,B8,B9,SP                                *
;******************************************************************************
_GMSKModLoop:
;** --------------------------------------------------------------------------*
;
; _GMSKModLoop .cproc  obptr, chsel, dfptr, ccptr
;            .reg tbptr                   ; address of modulator table
;            .reg mtptr                   ; working modulator table pointer
;            .reg index                   ; current row in modulator table
;            .reg iloop                   ; index into differential bits array
;            .reg loopcnt                 ; loop iteration downcounter
;            .reg itemp                   ; i sample register, temp storage
;            .reg qtemp                   ; q sample register, temp storage
;            .reg itemp1                   ; i sample register, temp storage
;            .reg qtemp1                   ; q sample register, temp storage
;            .reg itemp2                  ; i sample register, temp storage
;            .reg qtemp2                  ; q sample register, temp storage
;            .reg itemp3                  ; i sample register, temp storage
;            .reg qtemp3                  ; q sample register, temp storage
;            .reg phtest                  ; 0 iff phase == -1
;            .reg nbits                   ; number of bits to modulate
;            .reg bdata                   ; pointer to burst data
           MV      .L1X    B6,A0        ; |94| 
           LDW     .D1T1   *+A0(16),A1  ; |125| 
           NOP             2
           MVK     .S1     _g_DLBurstData,A5 ; |118|  address modulator burst data buffer

           ZERO    .L1     A0           ; |134|  qtemp is 0
||         MVKH    .S1     _g_DLBurstData,A5 ; |119| 

           STB     .D1T1   A0,*A5++     ; |135|  clear burst tx flag, bdata->data
||         SUB     .L2X    B4,A1,B2     ; |125| 

           LDB     .D1T2   *A5,B4       ; |152|  qtemp is g_DLBurstData.data[0]
           LDW     .D2T1   *B6,A8       ; |157|  iloop = _NUM_MOD_BITS_IN
           NOP             2

           MVK     .S1     0x1,A0       ; |147|  qtemp is 1
||         MV      .L1     A6,A1        ; |126| 

           MV      .L1X    B4,A0        ; |152| 
||         STB     .D1T1   A0,*A1++     ; |148|  diff[0] = 1

           ADD     .L1     A8,A5,A0     ; |158|  qtemp -> g_DLBurstData[NUM_MOD...IN]
||         STB     .D1T1   A0,*A1       ; |153|  diff[1] = qtemp

           LDB     .D1T1   *-A0(1),A0   ; |159|  qtemp = g_DLBurstData[NUM_MOD...-1]
           NOP             3
           ADD     .L1     A1,A8,A1     ; |160|  itemp -> diff[NUM_MOD...+1]
           STB     .D1T1   A0,*A1       ; |161|  diff[NUM_MOD...+1] = qtemp
           LDB     .D1T1   *A5++,A1     ; |170|  itemp = g_DLBurstData[0]
           NOP             1

           STW     .D2T1   A11,*SP--(8) ; |94| 
||         ADD     .L1     0x2,A6,A4    ; |94| 
||         MV      .L2X    A4,B7        ; |94| 

           MV      .L2X    A4,B5        ; |172| 
||         LDW     .D2T2   *+B6(8),B8   ; |124|  normal bits to modulate    

           MV      .L2X    A8,B0        ; |168| 
||         STW     .D2T1   A10,*+SP(4)  ; |94| 
||         MVK     .S1     _modLUT2x,A7 ; |115|  address modulator lookup table

           MVC     .S2     CSR,B9       ; |172| 
||         MV      .L2X    A1,B4        ; |172| 
||         MV      .L1     A5,A3        ; |172| 
|| [!B2]   LDW     .D2T2   *+B6(4),B8   ; |127|  use access burst length
||         MVKH    .S1     _modLUT2x,A7 ; |116| 

           AND     .L2     -2,B9,B1     ; |172| 

           MVC     .S2     B1,CSR       ; |172| 
||         SUB     .L2     B0,6,B0      ; |172| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : diffloop
;*      Known Minimum Trip Count         : 100
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 1
;*      Unpartitioned Resource Bound     : 1
;*      Partitioned Resource Bound(*)    : 2
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     0        1     
;*      .S units                     1        1     
;*      .D units                     1        1     
;*      .M units                     0        0     
;*      .X cross paths               0        2*    
;*      .T address paths             1        1     
;*      Long read paths              0        1     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           0        1     (.L or .S unit)
;*      Addition ops (.LSD)          1        0     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1        2*    
;*      Bound(.L .S .D .LS .LSD)     1        2*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 2  Schedule found with 4 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 6
;*
;*----------------------------------------------------------------------------*
L2:        ; PIPED LOOP PROLOG
; diffloop:  .trip 100
           LDB     .D1T1   *A3++,A0     ; |176|  qtemp = g_DLBurstData[iloop]
   [ B0]   B       .S1     diffloop     ; |181|  test and loop
           LDB     .D1T1   *A3++,A0     ;@ |176|  qtemp = g_DLBurstData[iloop]
   [ B0]   B       .S1     diffloop     ;@ |181|  test and loop
           LDB     .D1T1   *A3++,A0     ;@@ |176|  qtemp = g_DLBurstData[iloop]
;** --------------------------------------------------------------------------*
diffloop:        ; PIPED LOOP KERNEL

           MV      .L1     A0,A0        ; |179|  save g_DLBurstData[iloop] for next
||         CMPEQ   .L2X    B4,A0,B4     ; ^ |177|  phtest = (itemp==qtemp)
|| [ B0]   B       .S1     diffloop     ;@@ |181|  test and loop

           MV      .L2X    A0,B4        ; ^ |233| 
||         STB     .D2T2   B4,*B5++     ; |178|  save differentially encoded bit
||         LDB     .D1T1   *A3++,A0     ;@@@ |176|  qtemp = g_DLBurstData[iloop]
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@@@ |180|  downcount

;** --------------------------------------------------------------------------*
L4:        ; PIPED LOOP EPILOG

           MV      .L1     A0,A0        ;@ |179|  save g_DLBurstData[iloop] for next
||         CMPEQ   .L2X    B4,A0,B4     ;@ ^ |177|  phtest = (itemp==qtemp)

           MV      .L2X    A0,B4        ;@ ^ |233| 
||         STB     .D2T2   B4,*B5++     ;@ |178|  save differentially encoded bit

           MV      .L1     A0,A0        ;@@ |179|  save g_DLBurstData[iloop] for next
||         CMPEQ   .L2X    B4,A0,B4     ;@@ ^ |177|  phtest = (itemp==qtemp)

           MV      .L2X    A0,B4        ;@@ ^ |233| 
||         STB     .D2T2   B4,*B5++     ;@@ |178|  save differentially encoded bit

           MV      .L1     A0,A0        ;@@@ |179|  save g_DLBurstData[iloop] for next
||         CMPEQ   .L2X    B4,A0,B4     ;@@@ ^ |177|  phtest = (itemp==qtemp)

           MV      .L2X    A0,B4        ;@@@ ^ |233| 
||         STB     .D2T2   B4,*B5++     ;@@@ |178|  save differentially encoded bit

;** --------------------------------------------------------------------------*

           MV      .L1X    B6,A0
||         MVC     .S2     B9,CSR       ; |172| 

           LDW     .D1T1   *+A0(12),A1  ; |204| 
           NOP             4

           ADD     .L2X    B7,A1,B7     ; |205| 
||         ADD     .L1     0x4,A6,A3    ; |219|  qtemp->diff[4]

           LDBU    .D1T1   *A3--,A1     ; |221|  itemp: diff[4]
           NOP             3
           ZERO    .L1     A0           ; |218| 
   [ A1]   ADD     .L1     0x1,A0,A0    ; |222| 
           LDBU    .D1T1   *A3--,A1     ; |223|  itemp: diff[3]
           NOP             4
   [ A1]   ADD     .L1     0x2,A0,A0    ; |224| 
           LDBU    .D1T1   *A3--,A1     ; |225|  itemp: diff[2]
           NOP             4
   [ A1]   ADD     .L1     0x4,A0,A0    ; |226| 
           LDBU    .D1T1   *A3--,A1     ; |227|  itemp: diff[1]
           NOP             4
   [ A1]   ADD     .L1     0x8,A0,A0    ; |228| 
           LDBU    .D1T1   *A3,A1       ; |229|  itemp: diff[0]
           NOP             4

   [ A1]   ADDAB   .D1     A0,16,A0     ; |230| 
||         SHR     .S2     B8,0x1,B1    ; |212|  loop iteration downcounter

           SHL     .S2X    A0,0x3,B2    ; |232|  index is 8 * row of modulator table
||         MVK     .S1     0x1,A3       ; |202|  init phasor as I=1, Q=0 for even bit
||         ADD     .L1     0x2,A6,A8    ; |211|  set i variable in loop

           MVC     .S2     CSR,B2       ; |233| 
||         ADD     .L1X    A7,B2,A4     ; |233|  mtptr points to index-th row now
||         MV      .S1     A7,A3        ; |233| 
||         MV      .L2X    A3,B0        ; |233| 
||         ZERO    .D2     B8           ; |212| 
||         ZERO    .D1     A5           ; |212| 

           AND     .L2     -2,B2,B4     ; |233| 

           MVC     .S2     B4,CSR       ; |233| 
||         SUB     .L2     B1,3,B1      ; |233| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : modloop
;*      Known Minimum Trip Count         : 50
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 7
;*      Unpartitioned Resource Bound     : 11
;*      Partitioned Resource Bound(*)    : 11
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     4        2     
;*      .S units                    11*      11*    
;*      .D units                     8        4     
;*      .M units                     1        0     
;*      .X cross paths               1        5     
;*      .T address paths             8        4     
;*      Long read paths              1        3     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           1        1     (.L or .S unit)
;*      Addition ops (.LSD)          5        3     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             8        7     
;*      Bound(.L .S .D .LS .LSD)    10        7     
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 11 Schedule found with 4 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 6
;*
;*----------------------------------------------------------------------------*
L6:        ; PIPED LOOP PROLOG
; modloop:   .trip 50
           LDBU    .D1T1   *+A8(3),A2   ; |291|  nbits is diff[i+3]
           LDBU    .D1T1   *+A8(4),A2   ; |353|  nbits is diff[i+3] for i+1
           SHL     .S1     A0,0x1,A10   ; ^ |292|  index shifted left 1
           NOP             2
           ADD     .L1     A10,A2,A6    ; ^ |293|  put new bit in at LSB
           CLR     .S1     A6,0x5,0x1f,A6 ; ^ |294|  clear upper bits
           NOP             1
           MPY     .M1     0x2,A6,A9    ; ^ |354|  index shifted left 1
           SHL     .S1     A6,0x3,A7    ; |295|  make nbits index mod table rows
           ADD     .D1     A9,A2,A1     ; ^ |355|  put new bit in at LSB
           LDBU    .D1T1   *+A8(5),A2   ;@ |291|  nbits is diff[i+3]

           CLR     .S1     A1,0x5,0x1f,A0 ; ^ |356|  clear upper bits
||         LDBU    .D1T1   *+A8(6),A2   ;@ |353|  nbits is diff[i+3] for i+1

           LDW     .D1T1   *A4++,A1     ; |262|  qtemp hi: q sample qtemp lo: i sample
||         SHL     .S1     A0,0x1,A10   ;@ ^ |292|  index shifted left 1

           LDW     .D1T2   *A4++,B9     ; |272|  qtemp1 hi: q sample qtemp1 lo: i sample

           LDB     .D1T1   *-A8(2),A1   ; |286|  chsel is diff[i-2]
||         ADD     .L1     A3,A7,A4     ; |296|  mtptr addresses index-th row

           LDW     .D1T1   *A4++,A6     ; |320|  qtemp2 hi: q samp qtemp2 lo: i samp
||         ADD     .L1     A10,A2,A6    ;@ ^ |293|  put new bit in at LSB

           LDW     .D1T1   *A4++,A2     ; |332|  qtemp3 hi: q sample qtemp3 lo: i samp
||         CLR     .S1     A6,0x5,0x1f,A6 ;@ ^ |294|  clear upper bits

           SHL     .S2X    A0,0x3,B6    ; |357|  make nbits index mod table rows
|| [!B0]   SUB2    .S1     A5,A1,A1     ; |263|  negate both upper and lower halfword

           SHR     .S2X    A1,0x8,B5    ; |265|  qtemp aligned for 24 bit packing
||         EXTU    .S1     A1,0x10,0x14,A0 ; |264|  itemp is zero ext 12 bits for isamp
||         MPY     .M1     0x2,A6,A9    ;@ ^ |354|  index shifted left 1

           LDB     .D1T1   *-A8(1),A1   ; |348|  qtemp is diff[i-2] for i+1
|| [!B0]   SUB2    .S2     B8,B9,B9     ; |273|  negate both upper and lower
||         SHL     .S1     A6,0x3,A7    ;@ |295|  make nbits index mod table rows

           EXTU    .S2     B9,0x10,0x14,B4 ; |274|  itemp1 is zero ext low 12 bits
||         ADD     .L1X    A3,B6,A4     ; |358|  mtptr points to index-th row now
||         EXT     .S1     A6,0x10,0x10,A9 ; |321|  itemp2 is bits 4-15 of i, sgn ext  
|| [!A1]   SUB     .L2     0x1,B0,B0    ; |287|  if diff[i-2] is 0, flip phase sign
||         ADD     .D1     A9,A2,A1     ;@ ^ |355|  put new bit in at LSB

           CLR     .S2     B5,0x0,0xb,B5 ; |266|  prepare to combine
|| [ B0]   NEG     .L1     A2,A2        ; |335|  if phase 1, negate q sample
||         EXT     .S1     A2,0x10,0x10,A11 ; |333|  itemp3 is bits 4-15 of i, sgn ext  
||         LDBU    .D1T1   *+A8(7),A2   ;@@ |291|  nbits is diff[i+3]

;** --------------------------------------------------------------------------*
modloop:        ; PIPED LOOP KERNEL

           ADD     .L2X    B5,A0,B5     ; |267|  packed 12 bit samples in itemp
||         SHR     .S2     B9,0x8,B6    ; |275|  qtemp1 aligned for 24 bit packing
|| [!B0]   NEG     .L1     A11,A11      ; |334|  if phase -1, negate i sample
||         CLR     .S1     A1,0x5,0x1f,A0 ;@ ^ |356|  clear upper bits
||         LDBU    .D1T1   *+A8(8),A2   ;@@ |353|  nbits is diff[i+3] for i+1

           CLR     .S2     B6,0x0,0xb,B6 ; |276|  prepare to combine
|| [!B0]   NEG     .L1     A9,A9        ; |322|  if phase -1, negate i sample
||         LDW     .D1T1   *A4++,A1     ;@ |262|  qtemp hi: q sample qtemp lo: i sample
||         SHL     .S1     A0,0x1,A10   ;@@ ^ |292|  index shifted left 1

           ADD     .L2     B6,B4,B4     ; |277|  packed 12 bit samples in itemp1
||         SHL     .S2X    A9,0x8,B6    ; |325| 
|| [ B0]   NEG     .L1     A6,A6        ; |323|  if phase 1, negate q sample
||         SHRU    .S1     A2,0x14,A9   ; |336|  qtemp3 bits 0-11 now q sample msb
||         LDW     .D1T2   *A4++,B9     ;@ |272|  qtemp1 hi: q sample qtemp1 lo: i sample

           CLR     .S2     B6,0x0,0xb,B6 ; |326| 
||         SHL     .S1     A11,0x8,A7   ; |337| 
||         LDB     .D1T1   *A8,A1       ;@ |286|  chsel is diff[i-2]
||         ADD     .L1     A3,A7,A4     ;@ |296|  mtptr addresses index-th row

   [ B1]   SUB     .L2     B1,0x1,B1    ; |363|  downcount loop counter
||         SHRU    .S2X    A6,0x14,B9   ; |324|  qtemp2 bits 0-11 now q sample msb
||         CLR     .S1     A7,0x0,0xb,A7 ; |338| 
||         LDW     .D1T1   *A4++,A6     ;@ |320|  qtemp2 hi: q samp qtemp2 lo: i samp
||         ADD     .L1     A10,A2,A6    ;@@ ^ |293|  put new bit in at LSB

           STW     .D2T2   B5,*B7++(16) ; |268|  store it at output pointer
|| [ B1]   B       .S2     modloop      ; |364| 
|| [ A1]   SUB     .L2     0x1,B0,B0    ; |349|  if diff[i-2] is 1, flip phase sign
||         ADD     .L1     A7,A9,A7     ; |339|  itemp3 now packed 12 bit
||         LDW     .D1T1   *A4++,A2     ;@ |332|  qtemp3 hi: q sample qtemp3 lo: i samp
||         CLR     .S1     A6,0x5,0x1f,A6 ;@@ ^ |294|  clear upper bits

           STW     .D2T1   A7,*-B7(4)   ; |340|  store at output ptr
||         ADD     .L2     B6,B9,B5     ; |327|  itemp2 now packed 12 bit
||         SHL     .S2X    A0,0x3,B6    ;@ |357|  make nbits index mod table rows
|| [!B0]   SUB2    .S1     A5,A1,A1     ;@ |263|  negate both upper and lower halfword

           STW     .D2T2   B5,*-B7(8)   ; |328|  store at output ptr
||         SHR     .S2X    A1,0x8,B5    ;@ |265|  qtemp aligned for 24 bit packing
||         EXTU    .S1     A1,0x10,0x14,A0 ;@ |264|  itemp is zero ext 12 bits for isamp
||         MPY     .M1     0x2,A6,A9    ;@@ ^ |354|  index shifted left 1

           ADD     .L1     0x2,A8,A8    ; |362|  advance pointer in diff array 2 bits
||         STW     .D2T2   B4,*-B7(12)  ; |278|  store it at output pointer
||         LDB     .D1T1   *+A8(1),A1   ;@ |348|  qtemp is diff[i-2] for i+1
|| [!B0]   SUB2    .S2     B8,B9,B9     ;@ |273|  negate both upper and lower
||         SHL     .S1     A6,0x3,A7    ;@@ |295|  make nbits index mod table rows

           EXTU    .S2     B9,0x10,0x14,B4 ;@ |274|  itemp1 is zero ext low 12 bits
||         ADD     .L1X    A3,B6,A4     ;@ |358|  mtptr points to index-th row now
||         EXT     .S1     A6,0x10,0x10,A9 ;@ |321|  itemp2 is bits 4-15 of i, sgn ext  
|| [!A1]   SUB     .L2     0x1,B0,B0    ;@ |287|  if diff[i-2] is 0, flip phase sign
||         ADD     .D1     A9,A2,A1     ;@@ ^ |355|  put new bit in at LSB

           CLR     .S2     B5,0x0,0xb,B5 ;@ |266|  prepare to combine
|| [ B0]   NEG     .L1     A2,A2        ;@ |335|  if phase 1, negate q sample
||         EXT     .S1     A2,0x10,0x10,A11 ;@ |333|  itemp3 is bits 4-15 of i, sgn ext  
||         LDBU    .D1T1   *+A8(7),A2   ;@@@ |291|  nbits is diff[i+3]

;** --------------------------------------------------------------------------*
L8:        ; PIPED LOOP EPILOG

           ADD     .L2X    B5,A0,B5     ;@ |267|  packed 12 bit samples in itemp
||         SHR     .S2     B9,0x8,B6    ;@ |275|  qtemp1 aligned for 24 bit packing
|| [!B0]   NEG     .L1     A11,A11      ;@ |334|  if phase -1, negate i sample
||         CLR     .S1     A1,0x5,0x1f,A0 ;@@ ^ |356|  clear upper bits
||         LDBU    .D1T1   *+A8(8),A2   ;@@@ |353|  nbits is diff[i+3] for i+1

           CLR     .S2     B6,0x0,0xb,B6 ;@ |276|  prepare to combine
|| [!B0]   NEG     .L1     A9,A9        ;@ |322|  if phase -1, negate i sample
||         LDW     .D1T1   *A4++,A1     ;@@ |262|  qtemp hi: q sample qtemp lo: i sample
||         SHL     .S1     A0,0x1,A10   ;@@@ ^ |292|  index shifted left 1

           ADD     .L2     B6,B4,B4     ;@ |277|  packed 12 bit samples in itemp1
||         SHL     .S2X    A9,0x8,B6    ;@ |325| 
|| [ B0]   NEG     .L1     A6,A6        ;@ |323|  if phase 1, negate q sample
||         SHRU    .S1     A2,0x14,A9   ;@ |336|  qtemp3 bits 0-11 now q sample msb
||         LDW     .D1T2   *A4++,B9     ;@@ |272|  qtemp1 hi: q sample qtemp1 lo: i sample

           CLR     .S2     B6,0x0,0xb,B6 ;@ |326| 
||         SHL     .S1     A11,0x8,A7   ;@ |337| 
||         LDB     .D1T1   *A8,A1       ;@@ |286|  chsel is diff[i-2]
||         ADD     .L1     A3,A7,A4     ;@@ |296|  mtptr addresses index-th row

           SHRU    .S2X    A6,0x14,B9   ;@ |324|  qtemp2 bits 0-11 now q sample msb
||         CLR     .S1     A7,0x0,0xb,A7 ;@ |338| 
||         LDW     .D1T1   *A4++,A6     ;@@ |320|  qtemp2 hi: q samp qtemp2 lo: i samp
||         ADD     .L1     A10,A2,A6    ;@@@ ^ |293|  put new bit in at LSB

           STW     .D2T2   B5,*B7++(16) ;@ |268|  store it at output pointer
|| [ A1]   SUB     .L2     0x1,B0,B0    ;@ |349|  if diff[i-2] is 1, flip phase sign
||         ADD     .L1     A7,A9,A7     ;@ |339|  itemp3 now packed 12 bit
||         LDW     .D1T1   *A4++,A2     ;@@ |332|  qtemp3 hi: q sample qtemp3 lo: i samp
||         CLR     .S1     A6,0x5,0x1f,A6 ;@@@ ^ |294|  clear upper bits

           STW     .D2T1   A7,*-B7(4)   ;@ |340|  store at output ptr
||         ADD     .L2     B6,B9,B5     ;@ |327|  itemp2 now packed 12 bit
||         SHL     .S2X    A0,0x3,B6    ;@@ |357|  make nbits index mod table rows
|| [!B0]   SUB2    .S1     A5,A1,A1     ;@@ |263|  negate both upper and lower halfword

           STW     .D2T2   B5,*-B7(8)   ;@ |328|  store at output ptr
||         SHR     .S2X    A1,0x8,B5    ;@@ |265|  qtemp aligned for 24 bit packing
||         EXTU    .S1     A1,0x10,0x14,A0 ;@@ |264|  itemp is zero ext 12 bits for isamp
||         MPY     .M1     0x2,A6,A9    ;@@@ ^ |354|  index shifted left 1

           ADD     .L1     0x2,A8,A8    ;@ |362|  advance pointer in diff array 2 bits
||         STW     .D2T2   B4,*-B7(12)  ;@ |278|  store it at output pointer
||         LDB     .D1T1   *+A8(1),A1   ;@@ |348|  qtemp is diff[i-2] for i+1
|| [!B0]   SUB2    .S2     B8,B9,B9     ;@@ |273|  negate both upper and lower
||         SHL     .S1     A6,0x3,A7    ;@@@ |295|  make nbits index mod table rows

           EXTU    .S2     B9,0x10,0x14,B4 ;@@ |274|  itemp1 is zero ext low 12 bits
||         ADD     .L1X    A3,B6,A4     ;@@ |358|  mtptr points to index-th row now
||         EXT     .S1     A6,0x10,0x10,A9 ;@@ |321|  itemp2 is bits 4-15 of i, sgn ext  
|| [!A1]   SUB     .L2     0x1,B0,B0    ;@@ |287|  if diff[i-2] is 0, flip phase sign
||         ADD     .D1     A9,A2,A1     ;@@@ ^ |355|  put new bit in at LSB

           CLR     .S2     B5,0x0,0xb,B5 ;@@ |266|  prepare to combine
|| [ B0]   NEG     .L1     A2,A2        ;@@ |335|  if phase 1, negate q sample
||         EXT     .S1     A2,0x10,0x10,A11 ;@@ |333|  itemp3 is bits 4-15 of i, sgn ext  

           ADD     .L2X    B5,A0,B5     ;@@ |267|  packed 12 bit samples in itemp
||         SHR     .S2     B9,0x8,B6    ;@@ |275|  qtemp1 aligned for 24 bit packing
|| [!B0]   NEG     .L1     A11,A11      ;@@ |334|  if phase -1, negate i sample
||         CLR     .S1     A1,0x5,0x1f,A0 ;@@@ ^ |356|  clear upper bits

           CLR     .S2     B6,0x0,0xb,B6 ;@@ |276|  prepare to combine
|| [!B0]   NEG     .L1     A9,A9        ;@@ |322|  if phase -1, negate i sample
||         LDW     .D1T1   *A4++,A1     ;@@@ |262|  qtemp hi: q sample qtemp lo: i sample

           ADD     .L2     B6,B4,B4     ;@@ |277|  packed 12 bit samples in itemp1
||         SHL     .S2X    A9,0x8,B6    ;@@ |325| 
|| [ B0]   NEG     .L1     A6,A6        ;@@ |323|  if phase 1, negate q sample
||         SHRU    .S1     A2,0x14,A9   ;@@ |336|  qtemp3 bits 0-11 now q sample msb
||         LDW     .D1T2   *A4++,B9     ;@@@ |272|  qtemp1 hi: q sample qtemp1 lo: i sample

           CLR     .S2     B6,0x0,0xb,B6 ;@@ |326| 
||         SHL     .S1     A11,0x8,A7   ;@@ |337| 
||         LDB     .D1T1   *A8,A1       ;@@@ |286|  chsel is diff[i-2]
||         ADD     .L1     A3,A7,A4     ;@@@ |296|  mtptr addresses index-th row

           SHRU    .S2X    A6,0x14,B9   ;@@ |324|  qtemp2 bits 0-11 now q sample msb
||         CLR     .S1     A7,0x0,0xb,A7 ;@@ |338| 
||         LDW     .D1T1   *A4++,A6     ;@@@ |320|  qtemp2 hi: q samp qtemp2 lo: i samp

           STW     .D2T2   B5,*B7++(16) ;@@ |268|  store it at output pointer
|| [ A1]   SUB     .L2     0x1,B0,B0    ;@@ |349|  if diff[i-2] is 1, flip phase sign
||         ADD     .L1     A7,A9,A7     ;@@ |339|  itemp3 now packed 12 bit
||         LDW     .D1T1   *A4++,A2     ;@@@ |332|  qtemp3 hi: q sample qtemp3 lo: i samp

           STW     .D2T1   A7,*-B7(4)   ;@@ |340|  store at output ptr
||         ADD     .L2     B6,B9,B5     ;@@ |327|  itemp2 now packed 12 bit
||         SHL     .S2X    A0,0x3,B6    ;@@@ |357|  make nbits index mod table rows
|| [!B0]   SUB2    .S1     A5,A1,A1     ;@@@ |263|  negate both upper and lower halfword

           STW     .D2T2   B5,*-B7(8)   ;@@ |328|  store at output ptr
||         SHR     .S2X    A1,0x8,B5    ;@@@ |265|  qtemp aligned for 24 bit packing
||         EXTU    .S1     A1,0x10,0x14,A0 ;@@@ |264|  itemp is zero ext 12 bits for isamp

           ADD     .L1     0x2,A8,A8    ;@@ |362|  advance pointer in diff array 2 bits
||         STW     .D2T2   B4,*-B7(12)  ;@@ |278|  store it at output pointer
||         LDB     .D1T1   *+A8(1),A1   ;@@@ |348|  qtemp is diff[i-2] for i+1
|| [!B0]   SUB2    .S2     B8,B9,B9     ;@@@ |273|  negate both upper and lower

           EXTU    .S2     B9,0x10,0x14,B4 ;@@@ |274|  itemp1 is zero ext low 12 bits
||         ADD     .L1X    A3,B6,A4     ;@@@ |358|  mtptr points to index-th row now
||         EXT     .S1     A6,0x10,0x10,A9 ;@@@ |321|  itemp2 is bits 4-15 of i, sgn ext  
|| [!A1]   SUB     .L2     0x1,B0,B0    ;@@@ |287|  if diff[i-2] is 0, flip phase sign

           CLR     .S2     B5,0x0,0xb,B5 ;@@@ |266|  prepare to combine
|| [ B0]   NEG     .L1     A2,A2        ;@@@ |335|  if phase 1, negate q sample
||         EXT     .S1     A2,0x10,0x10,A11 ;@@@ |333|  itemp3 is bits 4-15 of i, sgn ext  

           ADD     .L2X    B5,A0,B5     ;@@@ |267|  packed 12 bit samples in itemp
||         SHR     .S2     B9,0x8,B6    ;@@@ |275|  qtemp1 aligned for 24 bit packing
|| [!B0]   NEG     .L1     A11,A11      ;@@@ |334|  if phase -1, negate i sample

           CLR     .S2     B6,0x0,0xb,B6 ;@@@ |276|  prepare to combine
|| [!B0]   NEG     .L1     A9,A9        ;@@@ |322|  if phase -1, negate i sample

           ADD     .L2     B6,B4,B4     ;@@@ |277|  packed 12 bit samples in itemp1
||         SHL     .S2X    A9,0x8,B6    ;@@@ |325| 
|| [ B0]   NEG     .L1     A6,A6        ;@@@ |323|  if phase 1, negate q sample
||         SHRU    .S1     A2,0x14,A9   ;@@@ |336|  qtemp3 bits 0-11 now q sample msb

           CLR     .S2     B6,0x0,0xb,B6 ;@@@ |326| 
||         SHL     .S1     A11,0x8,A7   ;@@@ |337| 

           SHRU    .S2X    A6,0x14,B9   ;@@@ |324|  qtemp2 bits 0-11 now q sample msb
||         CLR     .S1     A7,0x0,0xb,A7 ;@@@ |338| 

           STW     .D2T2   B5,*B7++(16) ;@@@ |268|  store it at output pointer
|| [ A1]   SUB     .L2     0x1,B0,B0    ;@@@ |349|  if diff[i-2] is 1, flip phase sign
||         ADD     .L1     A7,A9,A7     ;@@@ |339|  itemp3 now packed 12 bit

           STW     .D2T1   A7,*-B7(4)   ;@@@ |340|  store at output ptr
||         ADD     .L2     B6,B9,B5     ;@@@ |327|  itemp2 now packed 12 bit

           STW     .D2T2   B5,*-B7(8)   ;@@@ |328|  store at output ptr

           ADD     .L1     0x2,A8,A8    ;@@@ |362|  advance pointer in diff array 2 bits
||         STW     .D2T2   B4,*-B7(12)  ;@@@ |278|  store it at output pointer

           NOP             2
;** --------------------------------------------------------------------------*
           MVC     .S2     B2,CSR       ; |233| 

           B       .S2     B3
||         LDW     .D2T1   *+SP(4),A10

           LDW     .D2T1   *++SP(8),A11
           NOP             4
           ; BRANCH OCCURS


;            .endproc
