;****************************************************************************
;   (c) Copyright Cisco 2000
;   All Rights Reserved
;****************************************************************************

;******************************************************************************
;* TMS320C6x ANSI C Codegen                                      Version 2.00 *
;* Date/Time created: Wed Feb 24 14:41:23 1999                                *
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






;/**************************************************************************
;**
;**  GSM Demod w/ A|RT Library implemtation ported to C6 scalar assembly
;**
;** This file contains all of the assembly subroutines for the demod
;**
;**************************************************************************/


;**************
;
; Declarations
;
;*************************************************************************

    ; Declare the routines coded here to be globally visible.
    ; They are all C-callable, with prototypes in dem_protos.h.

 .def _offsetDerot   ; offset decompensation/derotation     
 .def _impulse       ; compute channel impulse response         
 .def _Energy        ; find energy, highest energy window
 .def _normEnergy    ; normalize Energy, get mantissa and exponent
 .def _normCir       ; normalize the cir
 .def _calcRho       ; autocorrelate cir with normed cir
 .def _matchFilter   ; filter samples by normed cir
 .def _expX          ; calculate exponent/mantissa
 .def _mag2DB        ; calculate power

;*************************************************************************

    ; externally declare the C variables we need here

 .ref _logTable      ;const Short logTable[]
 .ref _g_burstType   ;t_chanSel g_burstType
 .ref _g_BConsts_pt  ;t_burstConsts *g_BConsts_pt
 .ref _c_asm_consts  ;int c_asm_consts[]

;*************************************************************************

    ; C constant and structure offset interface

    ; offsets in _c_asm_consts to values of offsets to members of the
    ; t_burstConsts structure. The values in c_asm_consts are shown
    ; below along with the current burst constant values associated.
    ; The values in _c_asm_consts are calculated at run-time in order
    ; that the t_burstConsts structure and values may be modified w/o
    ; change here, except where noted in the Limitations below. Mainly
    ; numDataBits is assumed even.
    ;
    ; NOTE: these symbols must be aligned with the definition of
    ; _c_asm_consts in demTop.c
    ;
                           ; val  NB   AB   SB          meaning
                           ;---------------------------------------------------
o_tseqBaseAddr      .set 0 ;  0                ptr to training sequence
o_cirNumTseqBits    .set 1 ;  4   16   41   64 num training sequence bits
o_cirNumTseqBitsInv .set 2 ;  6 4096 1598 1024 2^16/(number training bits)
o_impulseTseqOffset .set 3 ;  8    0    3    ? offset in training sequence
o_cirLength         .set 4 ;  9    9   64   50 length of impulse response
o_cirStartIndex     .set 5 ; 10   68    8    ? offs impulse response in burst
o_toaMin            .set 6 ; 11   -2    0    ? timing advance (moving mobile)
o_numBits           .set 7 ; 12  142   77  142 bits in a burst
o_numDataBits       .set 8 ; 13  116   36   78 (is even!) data bits in a burst
o_numTailBits       .set 9 ; 14    3    8    3 number of initial tail bits 

    ; offsets in _c_asm_consts to values of symbols created with 
    ; #define in C. Loaded at run-time so that values may be changed
    ; without editing the assembly routines, except where noted in
    ; the Limitations description for individual routines. Mainly
    ; CIR_SIZE and CIR_BEST_LENGTH cannot be changed without affecting
    ; this module.
    ;
    ; NOTE: these symbols must be aligned with the definition of
    ; _c_asm_consts in demTop.c
                                       ;val         meaning
                                       ;------------------------------------
v_OFF_NUM_OFFSET_SAMPS         .set 10 ;128 pwr 2 num samps offset calced on
v_OFF_SHIFT_VAL                .set 11 ;  7 shift right count for average off
v_GSM_NUM_SAMPLES_PER_BURST    .set 12 ;156 samples in a demod burst
v_MAX_CIR_LENGTH               .set 13 ; 64 longest impulse response
v_CIR_SIZE                     .set 14 ;  5 (must=5!) cir window size 
v_CIR_BEST_LENGTH              .set 15 ;  5 (must=5!) normed cir length
v_WORD_SIZE                    .set 16 ; 16 number of bits in halfword (16)
v_ACCUM_GUARD_BITS             .set 17 ;  8 extra bits in 40 bit word (8)
v_ENERGY_THRESHOLD_PERCENT     .set 18 ; 13 thresh energy as pct of total
v_E_THRESH_FORMAT              .set 19 ;  8 shift count for formatting thresh
v_NB_NUM_ENCRYPT_BITS_PER_SIDE .set 20 ; 58 num bits before training 
v_NB_TRAIN_SIZE                .set 21 ; 26 size of nb training sequence
v_AB_TRAIN_SIZE                .set 22 ; 41 size of ab training sequence
v_TCHF                         .set 23 ;  1 enum symbol for TCHF burst type
v_RACH                         .set 24 ; 10 enum symbol for RACH burst type
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _offsetDerot                                                *
;*                                                                            *
;*   Regs Modified     : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,  *
;*                           B7,B8,B9                                         *
;*   Regs Used         : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,  *
;*                           B7,B8,B9                                         *
;******************************************************************************
_offsetDerot:
;** --------------------------------------------------------------------------*
;
;*************************************************************************
;
;
;**********************
;
; Optimization status
;
;*************************************************************************
;
; This software was written to optimize the C code provided by Frontier
; Design. Below are tables summarizing the optimization status of the
; code, in terms of number of execution packets required for each function.
; May be useful if further optimization is required. Note the C was compiled
; at -o because -o2 was unstable at the time this was written.
;
;                                Execution Packets
; Estimated      --------------------------------------------------------
;  Timing              Assembly (-o2)                    C (-o)           
;  Summary       --------------------------------------------------------
;                  NB   %   AB    %  SB          NB    %   AB      SB     
;                --------------------------------------------------------
; matchFilter    ; 754  30  429   8  754         4141  28  2256    4141
; offsetDerot    ; 477  20  477   9  477         1637  11  1637    1637
; impulse        ; 280  12  3475 65  3953        1057  7   12182   12972
; normEnergy     ; 203  8   224   4  247         358   2   408     454        
; weight         ; 200  11  98    2  182         2057  14  675     1431
; Energy         ; 68   7   343   8  273         243   2   1013    817         
; quantize       ; 130  5   50    1  91          1992  14  632     1346    
;                --------------------------------------------------------
; equalize       ; 88   4   56    1  88          171   1   106     171
; calcRho        ; 58   2   58    1  58          544   4   544     544
; normCir        ; 20   1   20    0  20          108   1   108     108
;                --------------------------------------------------------
; total            2375     5336     6223        14726     19561   23621
; -------------------------------------------
; packets saved   12351    14225    17398
; ratio             6.1      3.7      3.8
;
;                                Loop details (cl6x -o2 -mw -k)
;                --------------------------------------------------------
;                 Pipelined loops
;                -------------------------------------------------------------
;                 ovrhd = code outside pipelined loops
;                 pro   = number of packets in prolog
;                 epi   = number of packets in epilog
;                 k     = number of packets in pipelined kernel
;                 cnt   = value determining loop count
;                 d     = number of values per loop
;                 pec   = number fewer kernel loops due to pro/epilog            NB   AB    SB
;                -------------------------------------------------------------
;                 ovrhd pro epi  k  cnt/d  pec       pro epi k  cnt/d  pec
;                -------------------------------------------------------------
; matchFilter    ;  25+ (24+25+  5*(NMB/1 - 6))
; offsetDerot    ;  31+ (13+16+  8*(GSB/4 - 2))+    ( 5+ 6+  1*(NOS/1 -12))       
; impulse (inner);  12+ ( 7+ 7+  1*(NTS/1 -13))
; weight         ;  24+ ( 8+ 8+  2*(NDB/2 - 7))+    ( 6+ 6+  1*(NBE/1 -12)) 
; normEnergy (1) ; 104+ ( 5+ 8+  4*(BCL/1 - 3))+    ( 5+ 6+  2*(BCL/1 - 3))+
; normEnergy (2) ;      ( 7+ 7+  1*(NTS/1 - 7))+    ( 6+ 0+  1*(ESB/1 - 1))
; Energy         ;  20+ ( 8+ 9+  3*(CWL/1 - 3))     (10+10+  2*(CRL/1 - 5))         
; quantize       ;  14+ ( 6+ 6+  1*(NDB/1 -12))
;                --------------------------------------------------------
; equalize       ;  16+ ( 7+ 7+  1*(NMB/2 -13))
; normCir        ;   8+ ( 4+ 6+  2*(BCL/1 - 4))
;                --------------------------------------------------------
;
;                --------------------------------------------------------
;                 No loops, or not pipelined
;                --------------------------------------------------------
; impulse (outer);  19+ (12+inner loop)*CRL (or 19+(13+NTS)*CRL packets)
; calcRho        ;  58 
;                -------------------------------------------------------------
; Values:                                                        NB   AB   SB
;                                                             ----------------
; NOS = OFF_NUM_OFFSET_SAMPS                                  =  128  128  128
; GSB = GSM_NUM_SAMPLES_PER_BURST - g_BConsts_pt->numTailBits =  153  148  153
; BCL = BEST_CIR_LENGTH = CIR_SIZE                            =    5    5    5
; CRL = g_BConsts_pt->cirLength                               =    9   64   50
; CWL = CRL - BCL                                             =    4   59   45
; NTS = g_BConsts_pt->cirNumTseqBits                          =   16   41   64
; NMB = g_BConsts_pt->numBits                                 =  142   77  142
; NDB = g_BConsts_pt->numDataBits                             =  116   36   78
; NBE = NB_NUM_ENCRYPT_BITS_PER_SIDE if TCHF else NDB         =   58   36   78
; ESB = number of sign bits in energy mantissa max: 31
;
; Comments on further optimization
;
;  0) For NB, over half the time is spent in matchFilter and offsetDerot
;     and those are nearly optimal for the current algorithms. For AB and
;     SB, impulse dominates the timing on its own, due to the longer
;     training sequence and cir length. Its also nearly optimal.
;
;  1) matchFilter does ten multiplies per bit, so five execution packets
;     per bit is minimum (only two multiply units). NB performance can
;     only be improved by 19 clocks out of 710 minimum, so its done.
;
;  2) offsetDerot's comp loop isn't bad at 2 clocks per sample. It can be 
;     gotten as low as 1.25 clocks per sample if overflow is ignored instead
;     of saturated, so improvement may be possible. The offset accumulation
;     loop is about optimal, so best case improvement would be about 100 
;     clocks.
;
;  3) impulse has an ignorable outer overhead of 19. The inner loop
;     timing is 1+NTS, about as good as you can do (two multiplies = at
;     least one clock). The inner overhead is 12, including 8 for a complex
;     int by short multiply. If you save n clocks in the inner overhead the
;     result is cir length * n clocks saved, 9*n, 41*n, and 64*n clocks for
;     NB, AB, and SB, resp. For a 10% improvement you'd need 3, 10, and 7
;     clocks resp. so the cases where impulse matters, AB, and SB, are
;     pretty hard to improve.
;
;  4) The huge difference in weight and quantize is simply that the C compiler
;     on -o, the highest safe setting for v2.00, failed to pipeline the loop
;     due to complexity from conditionals.
;
; NOTE:
;     Its important that pec < min(cnt/d), or one of two things will happen.
;     If pec is as big as the trip count given, a very slow non-pipelined
;     loop will occur. If pec is larger than the loop count, and the trip
;     directive specifies a larger value yet, the program will crash. Its
;     *very* important that the trip counts be maintained as min(cnt/d) as 
;     the code is modified. Using trip counts saves code space and time by not
;     having to generate and branch around non-piped loops. It also makes the
;     .ASM file a lot easier to read. If you remove trip counts the
;     code will still run, but be larger, and a little slower. It won't
;     crash from inaccurate trip counts due to changes to constants or code.
;
;*************************************************************************
;
;*****************
;
; Demod routines
;
;/*************************************************************************
;*
;* offsetDerot() :
;* -------------
;*   Offset computation + Offset compensation + Derotation
;*
;*   The average DC offset in each channel is computed over the first 128
;*   samples, so that averaging is a shift right by 7.
;*
;*   The number of samples processed (a multiple of 4) is:
;*     4 * ( (GSM_NUMBER_SAMPLES_PER_BURST-3-g_BConsts_pt->numTailBits) / 4 )
;*
;*   The first sample processed is the g_BConsts_pt->numTailBits-th.
;*
;*   The average offset is subtracted from all these samples, removing
;*   any DC bias. The nth sample processed is rotated -90*n degrees at
;*   the same time.
;*
;* Input data       . samples[MAX_NUM_DATA]{I,Q}
;*                  . threshold (positive number)
;*
;* Output data      . samples[MAX_NUM_DATA]{I,Q}
;* 
;***************************************************************************/
;void offsetDerot
;( 
;   t_ULComplexSamp     samples[BURST_SIZE_UL_MAX], 
;   Word                threshold
;);
;
;
; _offsetDerot    .cproc      samptr,thresh
;     .reg ntailb         ; const: g_BConsts_pt->numTailBits
;     .reg nsamps         ; const: OFF_NUM_OFFSET_SAMPS
;     .reg offshv         ; const: OFF_SHIFT_VAL
;     .reg nsplpb         ; const: GSM_NUMBER_SAMPLES_PER_BURST                       
;     .reg lohalf         ; const: 0x0000ffff
;     .reg hihalf         ; const: 0xffff0000
;     .reg round          ; const: rounding for offset accumulation
;     .reg offIacc        ; accumulator for I samples
;     .reg offQacc        ; accumulator for Q samples
;     .reg offsetI        ; 16bit average DC over 128 I samples
;     .reg offsetQ        ; 16bit average DC over 128 Q samples
;     .reg sp1            ; general pointer storage
;     .reg sp2            ;   "        "      "
;     .reg iloop          ; pointer to samples, in loops
;     .reg temp           ; general storage
;     .reg itemp
;     .reg qtemp
;     .reg temp1          ;   "        "    
;     .reg nthresh        ; negative of thresh
;     .reg loopcnt        ; counts loop iterations
;     .reg cond1          ; general condition register
;     .reg bcp            ; constant interface pointers
;     .reg ccp
           MVK     .S1     _c_asm_consts,A5 ; |297| 

           MV      .L2X    A5,B5        ; |297| 
||         MVK     .S1     _g_BConsts_pt,A3 ; |294| 

           MVKH    .S2     _c_asm_consts,B5 ; |298|  load address of C constants
||         MVKH    .S1     _g_BConsts_pt,A3 ; |295|  load address of pointer

           LDW     .D1T1   *A3,A3       ; |296|  load pointer to burst constants
||         MV      .L1X    B5,A5        ; |298| 

           LDW     .D1T1   *+A5(36),A0  ; |300| 
           NOP             2
           LDW     .D1T1   *+A5(44),A9  ; |303| 
           NOP             1
           LDBU    .D1T2   *+A3[A0],B7  ; |301| 

           MVK     .S2     0xffff,B4    ; |306|  mask for lower halfword
||         MV      .L1X    B4,A7        ; |259| 

           MVK     .S2     0x1,B1       ; |310| 

           MV      .L2X    A9,B5        ; |307| 
||         LDW     .D1T2   *+A5(40),B0  ; |302| 
||         MVKH    .S2     0xffff,B4    ; |307|  (lohalf was sign extended!)

           SHL     .S2     B1,B5,B1     ; |312|  temp = 1<<OFF_SHIFT_VAL = 1.0
||         MV      .L1X    B3,A2        ; |259| 
||         MV      .L2X    A4,B8        ; |259| 

           ADDAW   .D2     B8,B7,B5     ; |347| 
||         SHR     .S2     B1,0x1,B1    ; |313|  temp = 1<<OFF_SHIFT_VAL-1 = .5
||         NOT     .L1X    B4,A5        ; |308|  mask for upper halfword
||         LDW     .D1T2   *+A5(48),B9  ; |304| 

           MVC     .S2     CSR,B2       ; |347| 
||         MV      .L1X    B5,A0        ; |347| 
||         MV      .L2     B5,B6        ; |347| 
||         ZERO    .D2     B4           ; |328| 
||         ZERO    .D1     A3           ; |327| 
||         NEG     .S1     A7,A8        ; |315| 

           AND     .L2     -2,B2,B5     ; |347| 

           MVC     .S2     B5,CSR       ; |347| 
||         ADD     .L2     2,B6,B6      ; |347| 
||         SUB     .D2     B0,12,B0     ; |347| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : offcalcloop
;*      Known Minimum Trip Count         : 128
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 1
;*      Partitioned Resource Bound(*)    : 1
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     0        0     
;*      .S units                     1*       0     
;*      .D units                     1*       1*    
;*      .M units                     0        0     
;*      .X cross paths               0        0     
;*      .T address paths             1*       1*    
;*      Long read paths              0        0     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           0        0     (.L or .S unit)
;*      Addition ops (.LSD)          1        2     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1*       0     
;*      Bound(.L .S .D .LS .LSD)     1*       1*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 1  Schedule found with 7 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 32
;*
;*----------------------------------------------------------------------------*
L2:        ; PIPED LOOP PROLOG

;** --------------------------------------------------------------------------*
offcalcloop:        ; PIPED LOOP KERNEL

           ADD     .L1     A3,A4,A3     ; |354|  32-bit accumulator won't ovf
||         ADD     .S2     B4,B5,B4     ; |356| 
||         LDH     .D1T1   *A0++(4),A4  ;@@@@@ |353|  as long as nsamps < 32768
||         LDH     .D2T2   *B6++(4),B5  ;@@@@@ |355|  adding 16 bit inputs
|| [ B0]   B       .S1     offcalcloop  ;@@@@@ |359| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@@@@@@ |358| 

;** --------------------------------------------------------------------------*
L4:        ; PIPED LOOP EPILOG
;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : comploop
;*      Known Minimum Trip Count         : 32
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 7
;*      Partitioned Resource Bound(*)    : 8
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     3        5     
;*      .S units                     3        7     
;*      .D units                     7        1     
;*      .M units                     0        0     
;*      .X cross paths               6        7     
;*      .T address paths             5        3     
;*      Long read paths              3        1     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           8        2     (.L or .S unit)
;*      Addition ops (.LSD)          1        2     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             7        7     
;*      Bound(.L .S .D .LS .LSD)     8*       6     
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 8  Schedule found with 3 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 32
;*
;*----------------------------------------------------------------------------*
L6:        ; PIPED LOOP PROLOG
;** --------------------------------------------------------------------------*
comploop:        ; PIPED LOOP KERNEL

;** --------------------------------------------------------------------------*
L8:        ; PIPED LOOP EPILOG

;** --------------------------------------------------------------------------*
           MVC     .S2     B3,CSR       ; |445| 
           B       .S2X    A2
           NOP             5
           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _impulse                                                    *
;*                                                                            *
;*   Regs Modified     : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B4,B5,B6,B7,B8,  *
;*                           B9                                               *
;*   Regs Used         : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,B0,B1,B3,B4,B5,B6,B7,  *
;*                           B8,B9                                            *
;******************************************************************************
_impulse:
;** --------------------------------------------------------------------------*
;
;
;/*************************************************************************
;*                                                                               
;* impulse() :                                                               
;* ---------                                                                 
;*   Compute the channel impulse response                            
;*                                                                         
;*                                                                         
;* Input data      . Training Sequence : tseq[TRAIN_SIZE]                    
;*                 . Derotated Samples : samples[MAX_NUM_DATA]{I,Q}               
;*
;* Output data     . Channel impulse response : cir[CIR_LENGTH]{I,Q}            
;* 
;************************************************************************
;*
;*  An illustration of "impulse()" operation is shown below for the NB:
;*  NB impulse starts at an "offset" because it is desirable to correlate
;*  the tseq bits against what is probably tseq bits of the samples.  If
;*  the samples were shifted because the mobile moved then the offset of 3
;*  will insure that the correlation is still in the tseq bits.  Also, with
;*  the diagram below the demod can find toa's from -2 to 2.
;*
;* Assumes: cirLength = 9, cirNumTseqBits = 16, offset = 3, toaMin = -2
;*
;*                             <-- cirNumTseqBits ->
;*                           offset              offset+cirNumTseqBits-1
;*                             |                   |
;*                             v                   v
;*                    ------------------------------
;*  tseq             |0  1  2  3   tseq values    18|   
;*                    ------------------------------ 
;*                           cirStartIndex
;*                             | 
;*              0       60 61  v                            
;*   i = 0       -------------------------------------      
;*  samples[]   |            |62   samples[]      77|   MAC --> cir[0]
;*               --------------------------------------     
;*
;*   i = 1                    ----------------------
;*  samples[]                |63   samples[]      78|   MAC --> cir[1]       
;*    ...                     ----------------------    ...
;*
;*   i = 8 = cirLength-1      ----------------------
;*  samples[]                |70   samples[]      85|   MAC --> cir[8]      
;*                            ---------------------- 
;*                            ^
;*                            |
;*                          cirStartIndex+cirLength-1
;*
;*                           j=0              j=cirNumTseqBits-1
;*          numTseqBits-1 
;*              ---       
;*              \
;*    cir[i] =  /  tseq[offset+j] * samples[cirstartIndex+i+j]
;*              ---
;*              j=0
;*
;***************************************************************************/
;void impulse
;(
;   t_ULComplexSamp     samples[BURST_SIZE_UL_MAX],
;   const t_TseqWord    tseq[MAX_TSEQ_SIZE], 
;   t_ULCirSamp         cir[MAX_CIR_LENGTH]
;);
;
; _impulse        .cproc samptr,tseqptr,cirptr
;     .reg start          ; const: g_BConsts_pt->cirStartIndex
;     .reg offset         ; const: g_BConsts_pt->impulseTseqOffset
;     .reg length         ; const: g_BConsts_pt->cirLength
;     .reg ntbits         ; const: g_BConsts_pt->cirNumTseqBits
;     .reg nbinv          ; const: g_BConsts_pt->cirNumTseqBitsInv
;     .reg maxcirlen      ; const: MAX_CIR_LENGTH
;     .reg wsize          ; const: WORD_SIZE
;     .reg resI           ; inner loop accumulators
;     .reg resQ           ;
;     .reg result         ; inner/outer loop storage/accumulator
;     .reg result1        ; inner/outer loop storage/accumulator
;     .reg temp           ; temporary storage
;     .reg temp1          ; temporary storage
;     .reg qtemp          ;    "        "
;     .reg itemp          ;    "        "
;     .reg qtemp1         ;    "        "
;     .reg itemp1         ;    "        "
;     .reg sptr           ; loop sample pointer storage
;     .reg sptr1          ; 
;     .reg tptr           ; loop training sequence pointer storage
;     .reg icount         ; outer loop downcounter
;     .reg jcount         ; inner loop downcounter
;     .reg round          ; 1 means round up, zero otherwize
;     .reg round1         ; 1 means round up, zero otherwize
;     .reg bcp
;     .reg ccp
           MVK     .S2     _c_asm_consts,B5 ; |631| 

           MVKH    .S2     _c_asm_consts,B5 ; |632|  load address of C constants
||         MVK     .S1     _g_BConsts_pt,A5 ; |628| 

           LDW     .D2T1   *+B5(20),A0  ; |636| 
||         MV      .L1X    B5,A3        ; |629| 
||         MVKH    .S1     _g_BConsts_pt,A5 ; |629|  load address of pointer

           LDW     .D1T2   *A5,B5       ; |630|  load pointer to burst constants
           NOP             4

           MV      .L2X    A3,B5
||         MV      .L1X    B5,A5        ; |630| 
||         MV      .S2     B5,B6        ; |630| 
||         LDW     .D1T1   *+A3(4),A8   ; |640| 

           LDW     .D2T1   *+B5(8),A2   ; |642| 
           LDW     .D2T2   *+B5(12),B5  ; |634| 
           NOP             1
           LDW     .D1T2   *+A3(16),B0  ; |638| 
           LDBU    .D1T1   *+A5[A0],A0  ; |637| 
           LDBU    .D1T2   *+A5[A8],B9  ; |641| 

           LDBU    .D2T1   *+B6[B5],A8  ; |635| 
||         MV      .L2X    A6,B8        ; |590| 

           ADDAB   .D1     A5,A2,A3     ; |643| 
||         MV      .L2X    A5,B4        ; |590| 
||         MV      .L1X    B4,A7        ; |590| 

           MV      .L2X    A3,B4        ; |639| 
||         LDBU    .D2T1   *+B4[B0],A1  ; |639| 

           LDHU    .D2T1   *B4,A6       ; |655| 
||         ADDAW   .D1     A4,A0,A9     ; |655|  sptr => samptr[start]

;** --------------------------------------------------------------------------*
impouterloop:        
; impouterloop:   .trip 9                     ;not pipelined, 19 packets to here
           MV      .L2X    A9,B4        ; |665|  sptr1 => samptr[start+i]

           MVC     .S2     CSR,B1       ; |667| 
||         ZERO    .L1     A3           ; |664| 
||         ZERO    .L2     B7           ; |663|  zero accumulators each i
||         ADDAB   .D1     A7,A8,A5     ; |667|  tptr => tseq[offset]
||         MV      .D2     B9,B0        ; |666|  set inner loop length

           AND     .L2     -2,B1,B5     ; |667| 

           MVC     .S2     B5,CSR       ; |667| 
||         SUB     .L2     B0,13,B0     ; |667| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : impinnerloop
;*      Known Minimum Trip Count         : 16
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 1
;*      Partitioned Resource Bound(*)    : 1
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     0        0     
;*      .S units                     1*       0     
;*      .D units                     1*       1*    
;*      .M units                     1*       1*    
;*      .X cross paths               1*       1*    
;*      .T address paths             1*       1*    
;*      Long read paths              0        0     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           0        0     (.L or .S unit)
;*      Addition ops (.LSD)          1        2     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1*       0     
;*      Bound(.L .S .D .LS .LSD)     1*       1*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 1  Schedule found with 8 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 32
;*
;*----------------------------------------------------------------------------*
L13:        ; PIPED LOOP PROLOG
; impinnerloop:   .trip 16                    ;7+7/13 kernel:1 

           LDB     .D1T1   *A5++,A0     ; |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ; |692|  load Q:I packed in a word

           LDB     .D1T1   *A5++,A0     ;@ |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ;@ |692|  load Q:I packed in a word

   [ B0]   B       .S1     impinnerloop ; |699| 
||         LDB     .D1T1   *A5++,A0     ;@@ |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ;@@ |692|  load Q:I packed in a word

   [ B0]   B       .S1     impinnerloop ;@ |699| 
||         LDB     .D1T1   *A5++,A0     ;@@@ |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ;@@@ |692|  load Q:I packed in a word

   [ B0]   B       .S1     impinnerloop ;@@ |699| 
||         LDB     .D1T1   *A5++,A0     ;@@@@ |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ;@@@@ |692|  load Q:I packed in a word

           MPY     .M2X    B6,A0,B5     ; |693| 
||         MPYHL   .M1X    B6,A0,A4     ; |695|   += s[start+i+j].I*tseq[offset+j]
|| [ B0]   B       .S1     impinnerloop ;@@@ |699| 
||         LDB     .D1T1   *A5++,A0     ;@@@@@ |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ;@@@@@ |692|  load Q:I packed in a word

           MPY     .M2X    B6,A0,B5     ;@ |693| 
||         MPYHL   .M1X    B6,A0,A4     ;@ |695|   += s[start+i+j].I*tseq[offset+j]
|| [ B0]   B       .S1     impinnerloop ;@@@@ |699| 
||         LDB     .D1T1   *A5++,A0     ;@@@@@@ |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ;@@@@@@ |692|  load Q:I packed in a word

;** --------------------------------------------------------------------------*
impinnerloop:        ; PIPED LOOP KERNEL

           ADD     .L2     B7,B5,B7     ; |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ; |696|  resQ[j]
||         MPY     .M2X    B6,A0,B5     ;@@ |693| 
||         MPYHL   .M1X    B6,A0,A4     ;@@ |695|   += s[start+i+j].I*tseq[offset+j]
|| [ B0]   B       .S1     impinnerloop ;@@@@@ |699| 
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@@@@@@ |698|  inner loop (cirNumTseqBits)
||         LDB     .D1T1   *A5++,A0     ;@@@@@@@ |686|  temp is max +/- 128
||         LDW     .D2T2   *B4++,B6     ;@@@@@@@ |692|  load Q:I packed in a word

;** --------------------------------------------------------------------------*
L15:        ; PIPED LOOP EPILOG

           ADD     .L2     B7,B5,B7     ;@ |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ;@ |696|  resQ[j]
||         MPY     .M2X    B6,A0,B5     ;@@@ |693| 
||         MPYHL   .M1X    B6,A0,A4     ;@@@ |695|   += s[start+i+j].I*tseq[offset+j]

           ADD     .L2     B7,B5,B7     ;@@ |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ;@@ |696|  resQ[j]
||         MPY     .M2X    B6,A0,B5     ;@@@@ |693| 
||         MPYHL   .M1X    B6,A0,A4     ;@@@@ |695|   += s[start+i+j].I*tseq[offset+j]

           ADD     .L2     B7,B5,B7     ;@@@ |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ;@@@ |696|  resQ[j]
||         MPY     .M2X    B6,A0,B5     ;@@@@@ |693| 
||         MPYHL   .M1X    B6,A0,A4     ;@@@@@ |695|   += s[start+i+j].I*tseq[offset+j]

           ADD     .L2     B7,B5,B7     ;@@@@ |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ;@@@@ |696|  resQ[j]
||         MPY     .M2X    B6,A0,B5     ;@@@@@@ |693| 
||         MPYHL   .M1X    B6,A0,A4     ;@@@@@@ |695|   += s[start+i+j].I*tseq[offset+j]

           ADD     .L2     B7,B5,B7     ;@@@@@ |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ;@@@@@ |696|  resQ[j]
||         MPY     .M2X    B6,A0,B5     ;@@@@@@@ |693| 
||         MPYHL   .M1X    B6,A0,A4     ;@@@@@@@ |695|   += s[start+i+j].I*tseq[offset+j]

           ADD     .L2     B7,B5,B7     ;@@@@@@ |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ;@@@@@@ |696|  resQ[j]

           ADD     .L2     B7,B5,B7     ;@@@@@@@ |694|  resI[j] 
||         ADD     .L1     A3,A4,A3     ;@@@@@@@ |696|  resQ[j]

;** --------------------------------------------------------------------------*

           MV      .L2X    A6,B4
||         MVC     .S2     B1,CSR       ; |667| 

           MPYU    .M2     B4,B7,B0     ; |722|  itemp = nbinv*lo(resI)    
||         MPYLH   .M1X    A6,B7,A0     ; |724|  qtemp = nbinv*hi(resI)
||         SUB     .L1     A1,0x1,A1    ; |743|  outer loop (cirLength)

   [ A1]   B       .S1     impouterloop ; |744| 
||         MPYU    .M1X    B4,A3,A2     ; |734|  itemp1 = nbinv*lo(resQ)    
||         MV      .L2X    A6,B5        ; |724| 

           SHRU    .S2     B0,0x10,B6   ; |725|  result = low part
||         MPYLH   .M2X    B5,A3,B5     ; |736|  qtemp = nbinv*hi(resQ)

           EXTU    .S2     B0,0x10,0x1f,B4 ; |723|  itemp bit 15 in round
||         SHRU    .S1     A2,0x10,A3   ; |737|  result1 = low part
||         ADD     .L2X    B6,A0,B6     ; |725| 

           ADD     .L2     B6,B4,B6     ; |727|  round for word shift
||         ADD     .L1X    A3,B5,A3     ; |737| 
||         EXTU    .S1     A2,0x10,0x1f,A0 ; |735|  itemp1 bit 15 in round1

           ADD     .L1     A3,A0,A3     ; |739|  round for word shift
||         STH     .D2T2   B6,*B8++     ; |728|  save in cir[i].I

           STH     .D2T1   A3,*B8++     ; |740|  save in cir[i].Q
||         ADDAW   .D1     A9,1,A9      ; |742|  sptr => samptr[start+i]

           ; BRANCH OCCURS ; |744| 
;** --------------------------------------------------------------------------*
           B       .S2     B3
           NOP             5
           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _Energy                                                     *
;*                                                                            *
;*   Regs Modified     : A0,A1,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B4,B5,B6,B7,B8,  *
;*                           B9                                               *
;*   Regs Used         : A0,A1,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,  *
;*                           B8,B9                                            *
;******************************************************************************
_Energy:
;** --------------------------------------------------------------------------*
;
;/*************************************************************************
;*                                                                               
;* Energy() :                                                        
;* --------                                                                  
;*   Compute the energy of the impulse response, the array which is
;*   the complex magnitude of each cir entry.
;*               
;*   Find the highest energy window of length CIR_SIZE                   
;*
;* Input data:      The channel impulse response: cir[2][CIR_LENGTH]        
;*                                                                         
;* Output data:     Energy of the impulse response: leng[CIR_LENGTH]        
;*                  Position of the best: returned by the function    
;*
;***************************************************************************/
;Char  Energy
;( 
;   t_ULCirSamp cir[MAX_CIR_LENGTH], 
;   LWord energy_L[MAX_CIR_LENGTH] 
;);
;
; _Energy         .cproc cirptr,nrgptr    ;returns Char
;     .reg length             ; const: length of cir
;     .reg maxcirlen          ; const: length of longest possible cir
;     .reg cirsize            ; const: width of NB cir window
;     .reg cptr0              ; temp pointers to cir
;     .reg cptr1              ;
;     .reg nptr0              ; temp pointers to energy array
;     .reg nptr1
;     .reg itemp              ; temp storage
;     .reg qtemp
;     .reg result2
;     .reg curidx             ; current idx+1 in find best window
;     .reg res1:res0          ; long accumulators
;     .reg key1:key0
;     .reg tmp1:tmp0
;     .reg loopcnt            ; holds loop counter
;     .reg best               ; return register for energy window
;     .reg smaller            ; condition registers
;     .reg uprdiff
;     .reg oddcir
;     .reg new,new1,new2,new3
;     .reg bcp
;     .reg ccp
           MVK     .S2     _g_BConsts_pt,B5 ; |800| 

           MVKH    .S2     _g_BConsts_pt,B5 ; |801|  load address of pointer
||         MVK     .S1     _c_asm_consts,A3 ; |803| 

           LDW     .D2T2   *B5,B5       ; |802|  load pointer to burst constants
||         MVKH    .S1     _c_asm_consts,A3 ; |804|  load address of C constants

           LDW     .D1T1   *+A3(16),A0  ; |806| 
           NOP             3
           MV      .L1X    B5,A5
           LDBU    .D1T2   *+A5[A0],B8  ; |807| 
           NOP             4
           CMPGTU  .L1X    B8,5,A1      ; |828| 
   [ A1]   B       .S1     L22          ; |828| 
           NOP             4

           MV      .L2     B8,B0        ; |828| 
||         MV      .L1X    B4,A0        ; |771| 
||         LDW     .D1T2   *+A3(56),B9  ; |809| 
||         MV      .S2     B4,B1        ; |771| 

           ; BRANCH OCCURS ; |828| 
;** --------------------------------------------------------------------------*
L20:        
; nrgcomploop:    .trip 4                         ; 8+8/7 kernel:2 
           LDW     .D1T2   *A4++,B5     ; |833| 
           NOP             3
           SUB     .L2     B0,0x1,B0    ; |839|  do this cirLength times

   [ B0]   B       .S1     L20          ; |840| 
||         MV      .L1X    B5,A5

           MV      .L2     B5,B4
||         MV      .L1X    B5,A3

           SMPYH   .M2     B5,B4,B5     ; |835|  new2 = cir[i+1].Q*cir[i+1].Q
||         SMPY    .M1     A3,A5,A3     ; |834|  new1 = cir[i+1].I*cir[i+1].I

           NOP             1
           ADD     .L1X    B5,A3,A3     ; |836| 
           STW     .D1T1   A3,*A0++     ; |837|  energy_L[i]= |cir[i+1]|^2
           ; BRANCH OCCURS ; |840| 
;** --------------------------------------------------------------------------*
           B       .S1     L28          ; |840| 

           MV      .L1X    B1,A0        ; |859|  nptr0 => energy_L[0]
||         ZERO    .L2     B7:B6        ; |856|  zero accumulators, best idx

           MV      .S1X    B6,A6        ; |861| 
||         SUB     .L2     B8,B9,B0     ; |861|  cirLength-CIR_SIZE loops
||         ADDAW   .D2     B1,B9,B5     ; |860|  nptr1 => energy_L[CIR_SIZE]
||         ZERO    .L1     A5:A4        ; |855|  prepare find max energy loop

           MVC     .S2     CSR,B8       ; |861| 
||         MV      .L2X    A4,B6        ; |861| 
||         MV      .D1     A4,A3        ; |861| 
||         MV      .L1X    B7,A7        ; |861| 
||         MVK     .S1     0x1,A8       ; |858|  init return index value
||         ZERO    .D2     B4           ; |857| 

           AND     .L2     -2,B8,B7     ; |861| 

           MVC     .S2     B7,CSR       ; |861| 
||         SUB     .L2     B0,3,B0      ; |861| 

           ; BRANCH OCCURS ; |840| 
;** --------------------------------------------------------------------------*
L22:        

           MVC     .S2     CSR,B2       ; |840| 
||         MV      .L2X    A0,B7        ; |840| 

           AND     .L2     -2,B2,B4     ; |840| 

           MVC     .S2     B4,CSR       ; |840| 
||         SUB     .L2     B0,5,B0      ; |840| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : nrgcomploop
;*      Known Minimum Trip Count         : 4
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 1
;*      Partitioned Resource Bound(*)    : 2
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     0        0     
;*      .S units                     1        0     
;*      .D units                     1        1     
;*      .M units                     1        1     
;*      .X cross paths               1        2*    
;*      .T address paths             1        1     
;*      Long read paths              0        1     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           1        2     (.L or .S unit)
;*      Addition ops (.LSD)          0        1     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1        1     
;*      Bound(.L .S .D .LS .LSD)     1        2*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 2  Schedule found with 6 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 32
;*
;*----------------------------------------------------------------------------*
L23:        ; PIPED LOOP PROLOG
           LDW     .D1T1   *A4++,A3     ; |833|  new = cir[i+1].Q:cir[i+1].I
           NOP             1
           LDW     .D1T1   *A4++,A3     ;@ |833|  new = cir[i+1].Q:cir[i+1].I
           NOP             1
           LDW     .D1T1   *A4++,A3     ;@@ |833|  new = cir[i+1].Q:cir[i+1].I

   [ B0]   SUB     .S2     B0,0x1,B0    ; |839|  do this cirLength times
||         SMPY    .M1     A3,A3,A0     ; |834|  new1 = cir[i+1].I*cir[i+1].I
||         MV      .L2X    A3,B5        ; |861| 

   [ B0]   B       .S1     nrgcomploop  ; |840| 
||         LDW     .D1T1   *A4++,A3     ;@@@ |833|  new = cir[i+1].Q:cir[i+1].I

           SMPYH   .M2     B5,B5,B6     ; |835|  new2 = cir[i+1].Q*cir[i+1].Q
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@ |839|  do this cirLength times
||         SMPY    .M1     A3,A3,A0     ;@ |834|  new1 = cir[i+1].I*cir[i+1].I
||         MV      .L2X    A3,B5        ;@ |861| 

           MV      .L1     A0,A5        ; |861| 
|| [ B0]   B       .S1     nrgcomploop  ;@ |840| 
||         LDW     .D1T1   *A4++,A3     ;@@@@ |833|  new = cir[i+1].Q:cir[i+1].I

           ADD     .L1X    B6,A5,A3     ; |836|  new3 = square mag cir[i+1]
||         SMPYH   .M2     B5,B5,B6     ;@ |835|  new2 = cir[i+1].Q*cir[i+1].Q
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@@ |839|  do this cirLength times
||         SMPY    .M1     A3,A3,A0     ;@@ |834|  new1 = cir[i+1].I*cir[i+1].I
||         MV      .L2X    A3,B5        ;@@ |861| 

;** --------------------------------------------------------------------------*
nrgcomploop:        ; PIPED LOOP KERNEL

           MV      .L2X    A3,B4        ; |861| 
||         MV      .L1     A0,A5        ;@ |861| 
|| [ B0]   B       .S1     nrgcomploop  ;@@ |840| 
||         LDW     .D1T1   *A4++,A3     ;@@@@@ |833|  new = cir[i+1].Q:cir[i+1].I

           STW     .D2T2   B4,*B7++     ; |837|  energy_L[i]= |cir[i+1]|^2
||         ADD     .L1X    B6,A5,A3     ;@ |836|  new3 = square mag cir[i+1]
||         SMPYH   .M2     B5,B5,B6     ;@@ |835|  new2 = cir[i+1].Q*cir[i+1].Q
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@@@ |839|  do this cirLength times
||         SMPY    .M1     A3,A3,A0     ;@@@ |834|  new1 = cir[i+1].I*cir[i+1].I
||         MV      .L2X    A3,B5        ;@@@ |861| 

;** --------------------------------------------------------------------------*
L25:        ; PIPED LOOP EPILOG

           MV      .L2X    A3,B4        ;@ |861| 
||         MV      .L1     A0,A5        ;@@ |861| 

           STW     .D2T2   B4,*B7++     ;@ |837|  energy_L[i]= |cir[i+1]|^2
||         ADD     .L1X    B6,A5,A3     ;@@ |836|  new3 = square mag cir[i+1]
||         SMPYH   .M2     B5,B5,B6     ;@@@ |835|  new2 = cir[i+1].Q*cir[i+1].Q
||         SMPY    .M1     A3,A3,A0     ;@@@@ |834|  new1 = cir[i+1].I*cir[i+1].I
||         MV      .L2X    A3,B5        ;@@@@ |861| 

           MV      .L2X    A3,B4        ;@@ |861| 
||         MV      .L1     A0,A5        ;@@@ |861| 

           STW     .D2T2   B4,*B7++     ;@@ |837|  energy_L[i]= |cir[i+1]|^2
||         ADD     .L1X    B6,A5,A3     ;@@@ |836|  new3 = square mag cir[i+1]
||         SMPYH   .M2     B5,B5,B6     ;@@@@ |835|  new2 = cir[i+1].Q*cir[i+1].Q
||         SMPY    .M1     A3,A3,A0     ;@@@@@ |834|  new1 = cir[i+1].I*cir[i+1].I
||         MV      .L2X    A3,B5        ;@@@@@ |861| 

           MV      .L2X    A3,B4        ;@@@ |861| 
||         MV      .L1     A0,A5        ;@@@@ |861| 

           STW     .D2T2   B4,*B7++     ;@@@ |837|  energy_L[i]= |cir[i+1]|^2
||         ADD     .L1X    B6,A5,A3     ;@@@@ |836|  new3 = square mag cir[i+1]
||         SMPYH   .M2     B5,B5,B6     ;@@@@@ |835|  new2 = cir[i+1].Q*cir[i+1].Q

           MV      .L2X    A3,B4        ;@@@@ |861| 
||         MV      .L1     A0,A5        ;@@@@@ |861| 

           STW     .D2T2   B4,*B7++     ;@@@@ |837|  energy_L[i]= |cir[i+1]|^2
||         ADD     .L1X    B6,A5,A3     ;@@@@@ |836|  new3 = square mag cir[i+1]

           MV      .L2X    A3,B4        ;@@@@@ |861| 
           STW     .D2T2   B4,*B7++     ;@@@@@ |837|  energy_L[i]= |cir[i+1]|^2
;** --------------------------------------------------------------------------*
           MVC     .S2     B2,CSR       ; |840| 
;** --------------------------------------------------------------------------*
L27:        

           MV      .L1X    B1,A0        ; |859|  nptr0 => energy_L[0]
||         ZERO    .L2     B7:B6        ; |856|  zero accumulators, best idx

           MV      .S1X    B6,A6        ; |861| 
||         SUB     .L2     B8,B9,B0     ; |861|  cirLength-CIR_SIZE loops
||         ADDAW   .D2     B1,B9,B5     ; |860|  nptr1 => energy_L[CIR_SIZE]
||         ZERO    .L1     A5:A4        ; |855|  prepare find max energy loop

           MVC     .S2     CSR,B8       ; |861| 
||         MV      .L2X    A4,B6        ; |861| 
||         MV      .D1     A4,A3        ; |861| 
||         MV      .L1X    B7,A7        ; |861| 
||         MVK     .S1     0x1,A8       ; |858|  init return index value
||         ZERO    .D2     B4           ; |857| 

           AND     .L2     -2,B8,B7     ; |861| 

           MVC     .S2     B7,CSR       ; |861| 
||         SUB     .L2     B0,3,B0      ; |861| 

;** --------------------------------------------------------------------------*
L28:        
;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : findbestloop
;*      Known Minimum Trip Count         : 4
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 3
;*      Unpartitioned Resource Bound     : 2
;*      Partitioned Resource Bound(*)    : 3
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     2        1     
;*      .S units                     1        1     
;*      .D units                     1        1     
;*      .M units                     0        0     
;*      .X cross paths               1        3*    
;*      .T address paths             1        1     
;*      Long read paths              2        0     
;*      Long write paths             2        0     
;*      Logical  ops (.LS)           0        2     (.L or .S unit)
;*      Addition ops (.LSD)          2        1     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             2        2     
;*      Bound(.L .S .D .LS .LSD)     2        2     
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 3  Schedule found with 4 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 32
;*
;*----------------------------------------------------------------------------*
L29:        ; PIPED LOOP PROLOG
; findbestloop:   .trip 4                         ;7+7/1 kernel:7

           LDW     .D1T1   *A0++,A9     ; |869|  energy_L[i+CIR_SIZE] -    
||         LDW     .D2T2   *B5++,B7     ; |870|   energy_L[i]

           NOP             2

           LDW     .D1T1   *A0++,A9     ;@ |869|  energy_L[i+CIR_SIZE] -    
||         LDW     .D2T2   *B5++,B7     ;@ |870|   energy_L[i]

   [ B0]   SUB     .S2     B0,0x1,B0    ; |892|  downcount and loop

   [ B0]   B       .S2     findbestloop ; |893| 
||         SUB     .L1X    B7,A9,A5     ; |871| 

           ADD     .L1     A5,A7:A6,A7:A6 ; |872|  accumlated into res1:res0 
||         LDW     .D1T1   *A0++,A9     ;@@ |869|  energy_L[i+CIR_SIZE] -    
||         LDW     .D2T2   *B5++,B7     ;@@ |870|   energy_L[i]

           SHR     .S1     A7:A6,0x8,A5:A4 ; |878| 
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@ |892|  downcount and loop

;** --------------------------------------------------------------------------*
findbestloop:        ; PIPED LOOP KERNEL

           CMPLT   .L2X    A4,B6,B1     ; ^ |879| 
|| [ B0]   B       .S2     findbestloop ;@ |893| 
||         SUB     .L1X    B7,A9,A5     ;@ |871| 

   [!B1]   MV      .L2X    A8,B4        ; |889|  save best index+1 so far
|| [!B1]   MV      .S1     A4,A3        ; ^ |888|  if result exceeds key, replace
||         ADD     .L1     A5,A7:A6,A7:A6 ;@ |872|  accumlated into res1:res0 
||         LDW     .D1T1   *A0++,A9     ;@@@ |869|  energy_L[i+CIR_SIZE] -    
||         LDW     .D2T2   *B5++,B7     ;@@@ |870|   energy_L[i]

           ADD     .L1     0x1,A8,A8    ; |891| 
|| [!B1]   MV      .L2X    A3,B6        ; ^ |771| 
||         SHR     .S1     A7:A6,0x8,A5:A4 ;@ |878| 
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@@ |892|  downcount and loop

;** --------------------------------------------------------------------------*
L31:        ; PIPED LOOP EPILOG

           CMPLT   .L2X    A4,B6,B1     ;@ ^ |879| 
||         SUB     .L1X    B7,A9,A5     ;@@ |871| 

   [!B1]   MV      .L2X    A8,B4        ;@ |889|  save best index+1 so far
|| [!B1]   MV      .S1     A4,A3        ;@ ^ |888|  if result exceeds key, replace
||         ADD     .L1     A5,A7:A6,A7:A6 ;@@ |872|  accumlated into res1:res0 

           ADD     .L1     0x1,A8,A8    ;@ |891| 
|| [!B1]   MV      .L2X    A3,B6        ;@ ^ |771| 
||         SHR     .S1     A7:A6,0x8,A5:A4 ;@@ |878| 

           CMPLT   .L2X    A4,B6,B1     ;@@ ^ |879| 
||         SUB     .L1X    B7,A9,A5     ;@@@ |871| 

   [!B1]   MV      .L2X    A8,B4        ;@@ |889|  save best index+1 so far
|| [!B1]   MV      .S1     A4,A3        ;@@ ^ |888|  if result exceeds key, replace
||         ADD     .L1     A5,A7:A6,A7:A6 ;@@@ |872|  accumlated into res1:res0 

           ADD     .L1     0x1,A8,A8    ;@@ |891| 
|| [!B1]   MV      .L2X    A3,B6        ;@@ ^ |771| 
||         SHR     .S1     A7:A6,0x8,A5:A4 ;@@@ |878| 

           CMPLT   .L2X    A4,B6,B1     ;@@@ ^ |879| 

   [!B1]   MV      .L2X    A8,B4        ;@@@ |889|  save best index+1 so far
|| [!B1]   MV      .S1     A4,A3        ;@@@ ^ |888|  if result exceeds key, replace

           ADD     .L1     0x1,A8,A8    ;@@@ |891| 
|| [!B1]   MV      .L2X    A3,B6        ;@@@ ^ |771| 

;** --------------------------------------------------------------------------*
           MVC     .S2     B8,CSR       ; |861| 
;** --------------------------------------------------------------------------*
           B       .S2     B3
           NOP             4
           MV      .L1X    B4,A4        ; |897| 
           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _normEnergy                                                 *
;*                                                                            *
;*   Regs Modified     : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A11,A12,A13,A14,A15,B0,*
;*                           B1,B2,B3,B4,B5,B6,B7,B8,B9,B10,B11,B12,B13,SP    *
;*   Regs Used         : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,   *
;*                           A15,B0,B1,B2,B3,B4,B5,B6,B7,B8,B9,B10,B11,B12,   *
;*                           B13,SP                                           *
;******************************************************************************
_normEnergy:
;** --------------------------------------------------------------------------*
;
;/*************************************************************************
;*
;* normEnergy :  REQUIRED C-stub, (no calls allowed from linear assembly)
;* ---------      calls energyAccumed, thresholdCir, mag2DB, expX
;*
;*   Threshold the lowest energies of the window found in Energy()
;*   and we compute the sum of the energies of this window
;*   and split it into mantissa * 2^exponent
;*
;* Input data:     - Derotated samples : samples[2][MAX_NUM_DATA]
;*                 - Channel impulse response : cir[2][CIR_LENGTH
;*                 - Energy of the impulse response : energy_L[CIR_LENGTH]
;*                 - Position of the best .. : indexBest
;*
;* Output data:    - Thresholded Cir : cir[2][CIR_LENGTH]
;*                 - First non zero Cir (or begin of the burst) : toa
;*                 - average power in dB: power                                           
;*                 - exponent
;*
;* Returned data   : mantInv = 1/Mantissa, (1 < mantInv <= 2)          
;*
;*
;* "toa" is returned for measurement reporting only.
;*  If "toa" is not equal to indexBest, for many bursts,
;*  then this indicates that the mobile should adjust it's 
;*  timing advance.
;*
;***************************************************************************/
;UWord2  normEnergy
;(
;   t_ULComplexSamp samples[BURST_SIZE_UL_MAX],
;   t_ULCirSamp     cir[MAX_CIR_LENGTH],
;   t_ULCirSamp     cirTemp[MAX_CIR_LENGTH],
;   LWord2          energy_L[MAX_CIR_LENGTH], 
;   Char            indexBest,
;   Word            *toa,
;   Word14          *power, 
;   Short           *exponent
;);
; _normEnergy     .cproc samptr,cirptr,ocirptr,nrgptr,best,toaptr,pwrptr,expptr ; returns UWord2
;      .reg ntbits       ; const: g_BConsts_pt->cirNumTseqBits
;      .reg nbinv        ; const: g_BConsts_pt->cirNumTseqBitsInv (65536/ntbits)
;      .reg cirstrt      ; const: g_BConsts_pt->cirStartIndex
;      .reg nsignb       ; const: g_BConsts_pt->numTailBits (3/8/3)
;      .reg mintoa       ; const: g_BConsts_pt->toaMin (-2/0/0)
;      .reg guardb       ; const: ACCUM_GUARD_BITS (8)
;      .reg wsize        ; const: WORD_SIZE (16)
;      .reg lbits        ; const: 2*WORD_SIZE+ACCUM_GUARDBITS (40)
;      .reg nrgpct       ; const: ENERGY_THRESHOLD_PERCENT
;      .reg thrfmt       ; const: E_THRESH_FORMAT
;      .reg cbstlen      ; const: CIR_BEST_LENGTH
;      .reg maxcir       ; const: MAX_CIR_LENGTH
;      .reg sptr         ; sample/energy pointer
;      .reg cptr         ; cir pointer
;      .reg ocptr        ; saved cir pointer
;      .reg acc1:acc0    ; one of two long accumulators
;      .reg res1:res0    ; the other long accumulator
;      .reg result       ; 32 bit storage
;      .reg result1      ;  "  "     "
;      .reg temp
;      .reg itemp        ;  "  "     "
;      .reg itemp1       ;  "  "     "
;      .reg qtemp        ;  "  "     "
;      .reg cond         ; conditions 
;      .reg cond1        ;
;      .reg allzero      ;
;      .reg loopcnt      ; loop downcounter
;      .reg one
;      .reg bcp          ; constants pointers
;      .reg ccp
;      .reg oldbest
;      .reg icnt,jcnt

           STW     .D2T2   B13,*SP--(48) ; |940| 
||         MVK     .S1     _c_asm_consts,A3 ; |996| 

           STW     .D2T1   A12,*+SP(16) ; |940| 
||         MVKH    .S1     _c_asm_consts,A3 ; |997|  load address of C constants

           LDW     .D1T1   *+A3(64),A5  ; |1010| 
||         STW     .D2T2   B3,*+SP(32)  ; |940| 

           STW     .D2T1   A14,*+SP(24) ; |940| 
||         MVK     .S2     _g_BConsts_pt,B5 ; |993| 

           STW     .D2T1   A11,*+SP(12) ; |940| 
||         MV      .L2X    A3,B4        ; |999| 
||         MV      .S2     B4,B3        ; |940| 

           LDW     .D2T1   *+B4(68),A2  ; |1009| 
||         MV      .L1     A6,A12       ; |940| 
||         MV      .S1X    B5,A0        ; |997| 

           LDW     .D1T1   *+A3(20),A6  ; |999| 
||         MVKH    .S1     _g_BConsts_pt,A0 ; |994|  load address of pointer

           LDW     .D1T1   *A0,A0       ; |995| 
||         STW     .D2T2   B10,*+SP(36) ; |940| 

           STW     .D2T1   A5,*+SP(4)   ; |1010| 
           STW     .D2T2   B11,*+SP(40) ; |940| 

           STW     .D2T2   B12,*+SP(44) ; |940| 
||         MV      .L2X    A3,B4        ; |1010| 

           LDW     .D1T1   *+A3(4),A5   ; |1001| 
||         LDW     .D2T2   *+B4(8),B7   ; |1003| 

           LDW     .D2T2   *+B4(72),B1  ; |1012| 
||         LDBU    .D1T1   *+A0[A6],A7  ; |1000| 

           LDW     .D2T2   *+B4(24),B5  ; |1006| 

           LDW     .D2T1   *+SP(4),A6   ; |1014| 
||         LDW     .D1T2   *+A3(76),B9  ; |1013| 
||         MV      .L1X    B10,A14      ; |940| 

           LDW     .D1T2   *+A3(60),B10 ; |1014| 
||         STW     .D2T1   A13,*+SP(20) ; |940| 

           MV      .L2X    A0,B4        ; |1016| 
||         LDBU    .D1T2   *+A0[A5],B0  ; |1002| 
||         STW     .D2T1   A15,*+SP(28) ; |940| 

           MV      .L1     A8,A0        ; |1002| 
||         ADDAW   .D1     A4,A7,A3     ; |1024| 
||         ADDAB   .D2     B4,B7,B7     ; |1004| 
||         MV      .L2     B8,B2        ; |940| 

           ADDAW   .D1     A3,A0,A3     ; |1025| 
||         ZERO    .L2     B5:B4        ; |1030| 
||         LDB     .D2T1   *+B4[B5],A11 ; |1007| 
||         MV      .L1X    B6,A13       ; |940| 
||         MV      .S2X    A8,B13       ; |1002| 

           MVC     .S2     CSR,B6       ; |1030| 
||         MV      .S1     A3,A4        ; |1030| 
||         MV      .L2X    A3,B8        ; |1030| 
||         ZERO    .L1     A1:A0        ; |1029|  zero the accumulators
||         LDHU    .D2T1   *B7,A15      ; |1005| 
||         ADDAH   .D1     A2,A6,A6     ; |1016| 

           MV      .L1X    B6,A7        ; |1030| 
           AND     .L1     -2,A7,A3     ; |1030| 

           MVC     .S2X    A3,CSR       ; |1030| 
||         ADD     .L2     2,B8,B8      ; |1030| 
||         SUB     .D2     B0,13,B0     ; |1030| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : accnrgloop
;*      Known Minimum Trip Count         : 16
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 1
;*      Partitioned Resource Bound(*)    : 1
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     1*       1*    
;*      .S units                     1*       1*    
;*      .D units                     1*       1*    
;*      .M units                     1*       1*    
;*      .X cross paths               0        0     
;*      .T address paths             1*       1*    
;*      Long read paths              1*       1*    
;*      Long write paths             1*       1*    
;*      Logical  ops (.LS)           0        0     (.L or .S unit)
;*      Addition ops (.LSD)          0        0     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1*       1*    
;*      Bound(.L .S .D .LS .LSD)     1*       1*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 1  Schedule found with 8 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : Unknown
;*
;*----------------------------------------------------------------------------*
L36:        ; PIPED LOOP PROLOG
; accnrgloop:     .trip 16                ; 7+7/7, kernel:1

           LDH     .D1T1   *A4++(4),A3  ; |1053| 
||         LDH     .D2T2   *B8++(4),B7  ; |1054| 

           LDH     .D1T1   *A4++(4),A3  ;@ |1053| 
||         LDH     .D2T2   *B8++(4),B7  ;@ |1054| 

   [ B0]   B       .S1     accnrgloop   ; |1062| 
||         LDH     .D1T1   *A4++(4),A3  ;@@ |1053| 
||         LDH     .D2T2   *B8++(4),B7  ;@@ |1054| 

   [ B0]   B       .S1     accnrgloop   ;@ |1062| 
||         LDH     .D1T1   *A4++(4),A3  ;@@@ |1053| 
||         LDH     .D2T2   *B8++(4),B7  ;@@@ |1054| 

   [ B0]   B       .S1     accnrgloop   ;@@ |1062| 
||         LDH     .D1T1   *A4++(4),A3  ;@@@@ |1053| 
||         LDH     .D2T2   *B8++(4),B7  ;@@@@ |1054| 

           SMPY    .M1     A3,A3,A5     ; |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ; |1058| 
|| [ B0]   B       .S1     accnrgloop   ;@@@ |1062| 
||         LDH     .D1T1   *A4++(4),A3  ;@@@@@ |1053| 
||         LDH     .D2T2   *B8++(4),B7  ;@@@@@ |1054| 

           SMPY    .M1     A3,A3,A5     ;@ |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ;@ |1058| 
|| [ B0]   B       .S1     accnrgloop   ;@@@@ |1062| 
||         LDH     .D1T1   *A4++(4),A3  ;@@@@@@ |1053| 
||         LDH     .D2T2   *B8++(4),B7  ;@@@@@@ |1054| 

;** --------------------------------------------------------------------------*
accnrgloop:        ; PIPED LOOP KERNEL

           SADD    .L1     A5,A1:A0,A1:A0 ; |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ; |1059| 
||         SMPY    .M1     A3,A3,A5     ;@@ |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ;@@ |1058| 
|| [ B0]   B       .S1     accnrgloop   ;@@@@@ |1062| 
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@@@@@@ |1061| 
||         LDH     .D1T1   *A4++(4),A3  ;@@@@@@@ |1053| 
||         LDH     .D2T2   *B8++(4),B7  ;@@@@@@@ |1054| 

;** --------------------------------------------------------------------------*
L38:        ; PIPED LOOP EPILOG

           SADD    .L1     A5,A1:A0,A1:A0 ;@ |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ;@ |1059| 
||         SMPY    .M1     A3,A3,A5     ;@@@ |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ;@@@ |1058| 

           SADD    .L1     A5,A1:A0,A1:A0 ;@@ |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ;@@ |1059| 
||         SMPY    .M1     A3,A3,A5     ;@@@@ |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ;@@@@ |1058| 

           SADD    .L1     A5,A1:A0,A1:A0 ;@@@ |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ;@@@ |1059| 
||         SMPY    .M1     A3,A3,A5     ;@@@@@ |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ;@@@@@ |1058| 

           SADD    .L1     A5,A1:A0,A1:A0 ;@@@@ |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ;@@@@ |1059| 
||         SMPY    .M1     A3,A3,A5     ;@@@@@@ |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ;@@@@@@ |1058| 

           SADD    .L1     A5,A1:A0,A1:A0 ;@@@@@ |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ;@@@@@ |1059| 
||         SMPY    .M1     A3,A3,A5     ;@@@@@@@ |1056|  sum squared magnitudes of
||         SMPY    .M2     B7,B7,B6     ;@@@@@@@ |1058| 

           SADD    .L1     A5,A1:A0,A1:A0 ;@@@@@@ |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ;@@@@@@ |1059| 

           SADD    .L1     A5,A1:A0,A1:A0 ;@@@@@@@ |1057|  samples over Tseq window
||         SADD    .L2     B6,B5:B4,B5:B4 ;@@@@@@@ |1059| 

;** --------------------------------------------------------------------------*

           MV      .L1X    B4,A3
||         MVC     .S2X    A7,CSR       ; |1030| 

           SHRU    .S1     A3,0x1,A3    ; |1068|  unsigned half of res0
           SADD    .L1     A3,A1:A0,A1:A0 ; |1069|  add lo part of one into other
           SADD    .L1     A3,A1:A0,A1:A0 ; |1070|  add lo part of one into other

           MV      .L1     A1,A5        ; |1070| 
||         SHL     .S2     B5,0x18,B5   ; |1072|  so that we can 

           MV      .L1X    B5,A3        ; |1071| 
||         SHL     .S1     A5,0x18,A5   ; |1071|  make the high parts 8 MSBs

           SADD    .L1     A5,A3,A5     ; |1073|  add them with saturation
           SHRU    .S1     A5,0x18,A5   ; |1074|  shift back to 8 LSBs of acc1
           MV      .L1     A5,A1        ; |1074| 
           NORM    .L1     A1:A0,A3     ; |1097| 

           MV      .L1     A3,A4        ; |1097| 
||         MV      .L2X    A3,B8        ; |1097| 

           LDW     .D2T1   *+SP(4),A3   ; |1097| 
           NOP             4

           ADD     .L1     A2,A3,A4     ; |1109| 
||         SHL     .S1     A1:A0,A4,A1:A0 ; |1108| 

           SHRU    .S1     A1:A0,A4,A1:A0 ; |1110|  data in 16 LSBs of acc0 now
           MPY     .M1     A0,A15,A0    ; |1118|  32bit normed value in acc0
           NOP             1

           SHL     .S1     A1:A0,A2,A1:A0 ; |1129|  restore binary point
||         MV      .L1X    B8,A3        ; |1129| 

           SHRU    .S1     A1:A0,A3,A1:A0 ; |1130| 
           NORM    .L1     A1:A0,A3     ; |1136| 

           SHL     .S1     A1:A0,A3,A9:A8 ; |1147| 
||         SUB     .L1     A6,0x4,A4    ; |1146| 

           SHR     .S1     A9:A8,A4,A9:A8 ; |1148| 
||         MV      .L2X    A3,B8        ; |1136| 
||         MVK     .S2     _logTable,B4 ; |1174|  sptr => logTable[0]

           MV      .L2X    A8,B4        ; |1148| 
||         MV      .L1X    B4,A3        ; |1148| 

           SUB     .L1     A8,0x4,A4    ; |1148| 
||         MVKH    .S1     _logTable,A3 ; |1175| 
||         MV      .S2X    A8,B7        ; |1148| 
||         CMPLT   .L2     0x3,B4,B0    ; |1149| 

           MV      .L1X    B8,A3        ; |1179| 
|| [ B0]   LDH     .D1T2   *+A3[A4],B7  ; |1179|   look up frac part
||         SUB     .L2X    A6,0x2,B6    ; |1179| 

           MV      .L2X    A0,B5        ; |1163| 
||         MV      .S2     B1,B4        ; |1163| 
||         SUB     .L1X    B6,A3,A3     ; |1159| 

           MPYLHU  .M2     B4,B5,B4     ; |1193|  energy is positive so
||         MV      .L1X    B1,A6        ; |1193| 

           MPYU    .M1     A6,A0,A0     ; |1192|  short by long (40bit) mult
||         SUBAB   .D1     A3,31,A4     ; |1163|  result = power

           MPY     .M1     0xc,A4,A4    ; |1164| 
||         MV      .L2     B4,B6        ; |1193| 

           ZERO    .L1     A9:A8        ; |1190| 
||         MV      .L2X    A0,B4        ; |1192| 

           SHL     .S2     B6,0x10,B7   ; |1194|  use unsigned operations
|| [ B0]   ADD     .L1X    A4,B7,A4     ; |1180|   add to result
||         MV      .L2X    A9,B5        ; |1193| 

           SHRU    .S2     B6,0x10,B7   ; |1196|  nrgpct/(1<<thrfmt) * sum
||         ADDU    .L2     B7,B5:B4,B5:B4 ; |1195| 

           MV      .L2X    A1,B6        ; |1196| 
||         ADD     .S2     B5,B7,B5     ; |1197| 

           MPYU    .M2     B1,B6,B7     ; |1198| 
           NOP             1
           ADD     .L2     B7,B5,B5     ; |1199| 

           SHRU    .S2     B5:B4,B9,B5:B4 ; |1200| 
||         MV      .L1X    B13,A0       ; |1199| 

           MVK     .S2     0x1,B1       ; |1228| 
||         ZERO    .L1     A1:A0        ; |1232| 
||         ADDAW   .D1     A13,A0,A3    ; |1221| 
||         MV      .S1X    B13,A7       ; |1199| 

           MV      .L1     A1,A5        ; |1232| 
||         MV      .S1X    B3,A6        ; |1234| 
||         MV      .L2     B1,B9        ; |1234| 
||         STH     .D1T1   A4,*A10      ; |1182|  store it at pwrptr
||         MV      .S2     B13,B11      ; |1208|  save in case cir all < thresh

           MVC     .S2     CSR,B12      ; |1234| 
||         MV      .L2X    A3,B6        ; |1234| 
||         MV      .L1     A12,A0       ; |1234| 
||         MV      .D1     A0,A4        ; |1234| 
||         MV      .S1X    B10,A1       ; |1232| 
||         MV      .D2     B13,B7       ; |1199| 

           AND     .L2     -2,B12,B8    ; |1234| 

           MVC     .S2     B8,CSR       ; |1234| 
||         SUB     .L1     A1,3,A1      ; |1234| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : threshcirloop
;*      Known Minimum Trip Count         : 5
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 1
;*      Unpartitioned Resource Bound     : 3
;*      Partitioned Resource Bound(*)    : 3
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     1        1     
;*      .S units                     0        1     
;*      .D units                     3*       2     
;*      .M units                     0        0     
;*      .X cross paths               1        0     
;*      .T address paths             3*       2     
;*      Long read paths              3*       2     
;*      Long write paths             1        0     
;*      Logical  ops (.LS)           0        1     (.L or .S unit)
;*      Addition ops (.LSD)          4        3     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1        2     
;*      Bound(.L .S .D .LS .LSD)     3*       3*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 3  Register is live too long
;*                   |1269| -> |1270|
;*         ii = 4  Schedule found with 3 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : Unknown
;*
;*----------------------------------------------------------------------------*
L40:        ; PIPED LOOP PROLOG
; threshcirloop:  .trip 5                         ; 5+8/2 kernel:4
           LDW     .D2T2   *B6++,B8     ; |1260|  get an energy element

           MV      .L1     A7,A8        ; ^ |1269|  (this helps scheduling)
||         LDW     .D1T1   *+A6[A7],A3  ; ^ |1241|  save this cir entry

           NOP             1
   [ A1]   B       .S2     threshcirloop ; |1274| 

           ADD     .L1     0x1,A7,A7    ; ^ |1272| 
||         LDW     .D2T2   *B6++,B8     ;@ |1260|  get an energy element

;** --------------------------------------------------------------------------*
threshcirloop:        ; PIPED LOOP KERNEL

           MV      .S1     A8,A9        ; |1302| 
||         ZERO    .S2     B0           ; |1259|  cond1 false by default
||         CMPGT   .L2     B8,B5:B4,B1  ; |1261|  compare to threshold
||         MV      .L1     A7,A8        ;@ ^ |1269|  (this helps scheduling)
||         LDW     .D1T1   *+A6[A7],A3  ;@ ^ |1241|  save this cir entry

   [ B1]   ZERO    .S2     B9           ; ^ |1266|  allzero is no longer true
|| [ B1]   ADD     .L1X    B8,A5:A4,A5:A4 ; |1263|  if greater, accumulate it
|| [ B1]   AND     .L2     B1,B9,B0     ; ^ |1264|  allzero if none greater yet
|| [ A1]   SUB     .S1     A1,0x1,A1    ;@ |1273| 

   [!B1]   ZERO    .L1     A3           ; |1268|  if not greater
||         STW     .D1T1   A3,*A0++     ; |1242| 
||         ADD     .L2     1,B7,B7      ; ^ |1302| 
|| [ B0]   STH     .D2T2   B7,*B2       ; ^ |1265|  if greater and allzero update
|| [ A1]   B       .S2     threshcirloop ;@ |1274| 

   [!B1]   STW     .D1T1   A3,*+A6[A9]  ; |1270|  zero the cir entry
||         ADD     .L1     0x1,A7,A7    ;@ ^ |1272| 
||         LDW     .D2T2   *B6++,B8     ;@@ |1260|  get an energy element

;** --------------------------------------------------------------------------*
L42:        ; PIPED LOOP EPILOG

           MV      .S1     A8,A9        ;@ |1302| 
||         ZERO    .S2     B0           ;@ |1259|  cond1 false by default
||         CMPGT   .L2     B8,B5:B4,B1  ;@ |1261|  compare to threshold
||         MV      .L1     A7,A8        ;@@ ^ |1269|  (this helps scheduling)
||         LDW     .D1T1   *+A6[A7],A3  ;@@ ^ |1241|  save this cir entry

   [ B1]   ZERO    .S2     B9           ;@ ^ |1266|  allzero is no longer true
|| [ B1]   ADD     .L1X    B8,A5:A4,A5:A4 ;@ |1263|  if greater, accumulate it
|| [ B1]   AND     .L2     B1,B9,B0     ;@ ^ |1264|  allzero if none greater yet

   [!B1]   ZERO    .L1     A3           ;@ |1268|  if not greater
||         STW     .D1T1   A3,*A0++     ;@ |1242| 
||         ADD     .L2     1,B7,B7      ;@ ^ |1302| 
|| [ B0]   STH     .D2T2   B7,*B2       ;@ ^ |1265|  if greater and allzero update

   [!B1]   STW     .D1T1   A3,*+A6[A9]  ;@ |1270|  zero the cir entry
||         ADD     .L1     0x1,A7,A7    ;@@ ^ |1272| 

           MV      .S1     A8,A9        ;@@ |1302| 
||         ZERO    .S2     B0           ;@@ |1259|  cond1 false by default
||         CMPGT   .L2     B8,B5:B4,B1  ;@@ |1261|  compare to threshold

   [ B1]   ZERO    .S2     B9           ;@@ ^ |1266|  allzero is no longer true
|| [ B1]   ADD     .L1X    B8,A5:A4,A5:A4 ;@@ |1263|  if greater, accumulate it
|| [ B1]   AND     .L2     B1,B9,B0     ;@@ ^ |1264|  allzero if none greater yet

   [!B1]   ZERO    .L1     A3           ;@@ |1268|  if not greater
||         STW     .D1T1   A3,*A0++     ;@@ |1242| 
||         ADD     .L2     1,B7,B7      ;@@ ^ |1302| 
|| [ B0]   STH     .D2T2   B7,*B2       ;@@ ^ |1265|  if greater and allzero update

   [!B1]   STW     .D1T1   A3,*+A6[A9]  ;@@ |1270|  zero the cir entry
;** --------------------------------------------------------------------------*

           MV      .L2     B9,B1        ; |1234| 
||         MVC     .S2     B12,CSR      ; |1234| 

   [!B1]   B       .S1     L50          ; |1295| 

           MV      .L1     A4,A0        ; |1234| 
|| [!B1]   MV      .S1     A5,A1

   [!B1]   NORM    .L1     A1:A0,A4     ; |1340| 
|| [ B1]   ADDAH   .D2     B3,B13,B3    ; |1296| 
|| [ B1]   MV      .L2     B10,B0       ; |1302| 
|| [ B1]   MV      .S1X    B13,A3       ; |1297| 

   [!B1]   SHL     .S1     A1:A0,A4,A1:A0 ; |1344| 
|| [!B1]   MV      .L2X    A4,B8        ; |1340| 

   [!B1]   LDW     .D2T1   *+SP(4),A4   ; |1345| 
   [!B1]   LDH     .D2T1   *B2,A3       ; |1322| 
           ; BRANCH OCCURS ; |1295| 
;** --------------------------------------------------------------------------*

           MVC     .S2     CSR,B7       ; |1302| 
||         MV      .D2     B3,B4        ; |1302| 
||         ADDAW   .D1     A13,A3,A3    ; |1299| 
||         MV      .L2X    A12,B5       ; |1302| 
||         MV      .L1     A5,A1        ; |1302| 

           AND     .L2     -2,B7,B6     ; |1302| 

           MVC     .S2     B6,CSR       ; |1302| 
||         SUB     .L2     B0,4,B0      ; |1302| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : zerocirloop
;*      Known Minimum Trip Count         : 5
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 2
;*      Partitioned Resource Bound(*)    : 2
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     1        0     
;*      .S units                     1        0     
;*      .D units                     1        2*    
;*      .M units                     0        0     
;*      .X cross paths               0        0     
;*      .T address paths             1        2*    
;*      Long read paths              1        1     
;*      Long write paths             1        0     
;*      Logical  ops (.LS)           0        0     (.L or .S unit)
;*      Addition ops (.LSD)          0        1     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1        0     
;*      Bound(.L .S .D .LS .LSD)     1        1     
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 2  Schedule found with 4 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 32
;*
;*----------------------------------------------------------------------------*
L45:        ; PIPED LOOP PROLOG
; zerocirloop:    .trip 5                    ; 4+6/2 kernel:2

           LDW     .D2T2   *B5++,B6     ; |1305|  time: 4 + cbstlen*2
||         LDW     .D1T1   *A3++,A4     ; |1308|  accumulate old cir energy too
|| [ B0]   B       .S1     zerocirloop  ; |1312| 

   [ B0]   SUB     .L2     B0,0x1,B0    ;@ |1311| 

           LDW     .D2T2   *B5++,B6     ;@ |1305|  time: 4 + cbstlen*2
||         LDW     .D1T1   *A3++,A4     ;@ |1308|  accumulate old cir energy too
|| [ B0]   B       .S1     zerocirloop  ;@ |1312| 

   [ B0]   SUB     .L2     B0,0x1,B0    ;@@ |1311| 
;** --------------------------------------------------------------------------*
zerocirloop:        ; PIPED LOOP KERNEL

           LDW     .D2T2   *B5++,B6     ;@@ |1305|  time: 4 + cbstlen*2
||         LDW     .D1T1   *A3++,A4     ;@@ |1308|  accumulate old cir energy too
|| [ B0]   B       .S1     zerocirloop  ;@@ |1312| 

           STW     .D2T2   B6,*B4++     ; |1306|  restore old cir    
||         ADD     .L1     A4,A1:A0,A1:A0 ; |1309| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@@@ |1311| 

;** --------------------------------------------------------------------------*
L47:        ; PIPED LOOP EPILOG

           LDW     .D2T2   *B5++,B6     ;@@@ |1305|  time: 4 + cbstlen*2
||         LDW     .D1T1   *A3++,A4     ;@@@ |1308|  accumulate old cir energy too

           STW     .D2T2   B6,*B4++     ;@ |1306|  restore old cir    
||         ADD     .L1     A4,A1:A0,A1:A0 ;@ |1309| 

           NOP             1

           STW     .D2T2   B6,*B4++     ;@@ |1306|  restore old cir    
||         ADD     .L1     A4,A1:A0,A1:A0 ;@@ |1309| 

           NOP             1

           STW     .D2T2   B6,*B4++     ;@@@ |1306|  restore old cir    
||         ADD     .L1     A4,A1:A0,A1:A0 ;@@@ |1309| 

;** --------------------------------------------------------------------------*

           STH     .D2T2   B11,*B2      ; |1314| 
||         MV      .L1     A1,A5        ; |1302| 
||         MVC     .S2     B7,CSR       ; |1302| 

;** --------------------------------------------------------------------------*
nozerocir:        
           MV      .L1     A5,A1
           NORM    .L1     A1:A0,A4     ; |1340| 

           SHL     .S1     A1:A0,A4,A1:A0 ; |1344| 
||         MV      .L2X    A4,B8        ; |1340| 

           LDW     .D2T1   *+SP(4),A4   ; |1345| 
           LDH     .D2T1   *B2,A3       ; |1322| 
;** --------------------------------------------------------------------------*
L50:        
           LDW     .D2T2   *+SP(4),B4   ; |1346| 
           NOP             1
           SHR     .S1     A1:A0,A2,A1:A0 ; |1345| 
           SHR     .S1     A1:A0,A4,A1:A0 ; |1346|  acc0 is mantissa

           NORM    .L1     A0,A5        ; |1356|  align msb of acc0 with 1 (F32.29)
||         ADD     .S1     A3,A11,A3    ; |1323| 

           SUB     .L2X    A5,6,B0      ; |1356| 
||         ADD     .S2     B4,B4,B4     ; |1351|  one = 1 in F32.29
||         SUB     .L1X    A2,B8,A3     ; |1324| 
||         STH     .D2T1   A3,*B2       ; |1324| 

           MV      .L2X    A5,B8        ; |1356| 
||         SHL     .S1     A0,A5,A0     ; |1357|  shift left number of redundant
||         SUB     .L1X    B4,0x3,A4    ; |1356| 

           SHR     .S1     A0,0x1,A0    ; |1358|  sign bits, minus one
||         MV      .L2X    A4,B4        ; |1357| 
||         MVK     .S2     0x1,B5       ; |1353| 

           MV      .L2X    A0,B4        ; |1360| 
||         SHL     .S2     B5,B4,B5     ; |1354|  one = 2^29 = 0x20000000  
||         STH     .D1T1   A3,*A14      ; |1342| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : mantinvloop
;*      Known Minimum Trip Count         : 12
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 1
;*      Partitioned Resource Bound(*)    : 1
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     0        1*    
;*      .S units                     1*       0     
;*      .D units                     0        0     
;*      .M units                     0        0     
;*      .X cross paths               0        0     
;*      .T address paths             0        0     
;*      Long read paths              0        0     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           0        0     (.L or .S unit)
;*      Addition ops (.LSD)          0        1     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1*       1*    
;*      Bound(.L .S .D .LS .LSD)     1*       1*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 1  Schedule found with 7 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : 32
;*
;*----------------------------------------------------------------------------*
L51:        ; PIPED LOOP PROLOG
; mantinvloop:    .trip 12                  ; 5+0/0 k:1
   [ B0]   B       .S1     mantinvloop  ; |1367| 
   [ B0]   B       .S1     mantinvloop  ;@ |1367| 
   [ B0]   B       .S1     mantinvloop  ;@@ |1367| 
   [ B0]   B       .S1     mantinvloop  ;@@@ |1367| 
   [ B0]   B       .S1     mantinvloop  ;@@@@ |1367| 
;** --------------------------------------------------------------------------*
mantinvloop:        ; PIPED LOOP KERNEL

           SUBC    .L2     B5,B4,B5     ; |1365|  divide 1(F32.29) by mantissa (acc0)
|| [ B0]   B       .S1     mantinvloop  ;@@@@@ |1367| 
|| [ B0]   SUB     .S2     B0,0x1,B0    ;@@@@@@ |1366|  

;** --------------------------------------------------------------------------*
L53:        ; PIPED LOOP EPILOG
;** --------------------------------------------------------------------------*
           LDW     .D2T2   *+SP(32),B3
           LDW     .D2T2   *+SP(40),B11
           LDW     .D2T2   *+SP(36),B10
           LDW     .D2T1   *+SP(28),A15
           LDW     .D2T1   *+SP(24),A14
           LDW     .D2T1   *+SP(20),A13
           LDW     .D2T1   *+SP(16),A12
           LDW     .D2T1   *+SP(12),A11

           B       .S2     B3
||         LDW     .D2T2   *+SP(44),B12
||         MVK     .S1     0x20,A0      ; |1369| 

           LDW     .D2T2   *++SP(48),B13
||         SUB     .L2X    A0,B8,B4     ; |1370| 

           SHL     .S2     B4,0x5,B7    ; |1371|  extracts the nsignb long field

           ADD     .L2     B4,B7,B4     ; |1372|  from bit 0 up to bit nsignb-1.
||         MV      .L1X    B5,A3        ; |1372| 

           MV      .L1X    B4,A0        ; |1372| 
           EXTU    .S1     A3,A0,A4     ; |1373|  because 32-nsignb = 31-(nsignb-1)
           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _normCir                                                    *
;*                                                                            *
;*   Regs Modified     : A0,A3,A4,A5,A6,A7,A8,B0,B4,B5,B6,B7,B8,SP            *
;*   Regs Used         : A0,A3,A4,A5,A6,A7,A8,B0,B3,B4,B5,B6,B7,B8,SP         *
;******************************************************************************
_normCir:
;** --------------------------------------------------------------------------*
;
;
;/*************************************************************************
;* normCir :
;* --------
;*   We normalize the value of Cir ( Cir / Mantissa ).
;*
;*
;* Input data      . Channel impulse response : cir[CIR_LENGTH].{I,Q}
;*                 . 1/Mantissa : mantInv (1 <mantInt <= 2)
;*                 . Position of the best CIR window : *indexBest
;*
;* Output data     . Normalized channel impulse response : nCir[CIR_SIZE].{I,Q}
;*
;***************************************************************************/
;
;void normCir
;(
;   t_ULCirSamp cir[MAX_CIR_LENGTH], 
;   UWord2      mantInv, 
;   Char        indexBest_fix, 
;   t_ULCirSamp cirNorm[MAX_CIR_LENGTH] 
;);
;
; _normCir        .cproc  cirptr,mantinv,best,ncirptr
;     .reg cbstlen                ; const: CIR_BEST_LENGTH (5)
;     .reg wsize                  ; const: WORD_SIZE (16)
;     .reg cptr0                  ; pointer to cir
;     .reg ncptr0                 ; pointer to normed cir
;     .reg temp,qtemp,itemp       ; temp storage
;     .reg loopcnt                ; loop counter
;     .reg ccp                    ; constants pointer
           MVK     .S1     _c_asm_consts,A5 ; |1422| 
           MVKH    .S1     _c_asm_consts,A5 ; |1423|  load address of C constants
           LDW     .D1T2   *+A5(60),B0  ; |1425| 
           MV      .L2X    A4,B5        ; |1404| 

           MV      .L2X    A6,B7        ; |1404| 
||         MV      .L1X    B4,A8        ; |1404| 

           ADDAW   .D2     B5,B7,B4     ; |1432|  cptr0 => cir[best]
||         SUB     .L2     SP,8,SP      ; |1404| 
||         MV      .L1X    B6,A0        ; |1432| 

           MVC     .S2     CSR,B8       ; |1432| 
||         MV      .L2     B4,B6        ; |1432| 
||         MV      .L1X    B4,A4        ; |1432| 
||         MV      .D2     B6,B5        ; |1432| 
||         LDW     .D1T1   *+A5(64),A3  ; |1426| 

           AND     .L2     -2,B8,B4     ; |1432| 
||         SUB     .S2     B0,4,B0      ; |1432| 

           MVC     .S2     B4,CSR       ; |1432| 
||         ADD     .L2     2,B6,B6      ; |1432| 
||         ADD     .D2     2,B5,B5      ; |1432| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : normcirloop
;*      Known Minimum Trip Count         : 5
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 0
;*      Unpartitioned Resource Bound     : 2
;*      Partitioned Resource Bound(*)    : 2
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     0        0     
;*      .S units                     2*       1     
;*      .D units                     2*       2*    
;*      .M units                     2*       0     
;*      .X cross paths               1        1     
;*      .T address paths             2*       2*    
;*      Long read paths              1        1     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           0        1     (.L or .S unit)
;*      Addition ops (.LSD)          0        1     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1        1     
;*      Bound(.L .S .D .LS .LSD)     2*       2*    
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 2  Schedule found with 5 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : Unknown
;*
;*----------------------------------------------------------------------------*
L57:        ; PIPED LOOP PROLOG
; normcirloop:    .trip 5                     ; 4+6/4 kernel:2

           LDH     .D1T1   *A4++(4),A7  ; |1454|  cir[i].I * mantinv >> 16
||         LDH     .D2T2   *B6++(4),B4  ; |1459|  cir[i].I * mantinv >> 16

           NOP             1

           LDH     .D1T1   *A4++(4),A7  ;@ |1454|  cir[i].I * mantinv >> 16
||         LDH     .D2T2   *B6++(4),B4  ;@ |1459|  cir[i].I * mantinv >> 16

   [ B0]   SUB     .L2     B0,0x1,B0    ; |1464| 

   [ B0]   B       .S2     normcirloop  ; |1465| 
||         LDH     .D1T1   *A4++(4),A7  ;@@ |1454|  cir[i].I * mantinv >> 16
||         LDH     .D2T2   *B6++(4),B4  ;@@ |1459|  cir[i].I * mantinv >> 16

           SMPY    .M1X    B4,A8,A6     ; |1460| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@ |1464| 

           SMPY    .M1     A7,A8,A6     ; |1455| 
|| [ B0]   B       .S2     normcirloop  ;@ |1465| 
||         LDH     .D1T1   *A4++(4),A7  ;@@@ |1454|  cir[i].I * mantinv >> 16
||         LDH     .D2T2   *B6++(4),B4  ;@@@ |1459|  cir[i].I * mantinv >> 16

           SHR     .S1     A6,A3,A5     ; |1461| 
||         SMPY    .M1X    B4,A8,A6     ;@ |1460| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@@ |1464| 

;** --------------------------------------------------------------------------*
normcirloop:        ; PIPED LOOP KERNEL

           SHR     .S1     A6,A3,A5     ; |1456| 
||         MV      .L2X    A5,B7        ; |1404| 
||         SMPY    .M1     A7,A8,A6     ;@ |1455| 
|| [ B0]   B       .S2     normcirloop  ;@@ |1465| 
||         LDH     .D1T1   *A4++(4),A7  ;@@@@ |1454|  cir[i].I * mantinv >> 16
||         LDH     .D2T2   *B6++(4),B4  ;@@@@ |1459|  cir[i].I * mantinv >> 16

           STH     .D1T1   A5,*A0++(4)  ; |1457|  is normcir[i].I
||         STH     .D2T2   B7,*B5++(4)  ; |1462|  is normcir[i].I
||         SHR     .S1     A6,A3,A5     ;@ |1461| 
||         SMPY    .M1X    B4,A8,A6     ;@@ |1460| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@@@ |1464| 

;** --------------------------------------------------------------------------*
L59:        ; PIPED LOOP EPILOG

           SHR     .S1     A6,A3,A5     ;@ |1456| 
||         MV      .L2X    A5,B7        ;@ |1404| 
||         SMPY    .M1     A7,A8,A6     ;@@ |1455| 

           STH     .D1T1   A5,*A0++(4)  ;@ |1457|  is normcir[i].I
||         STH     .D2T2   B7,*B5++(4)  ;@ |1462|  is normcir[i].I
||         SHR     .S1     A6,A3,A5     ;@@ |1461| 
||         SMPY    .M1X    B4,A8,A6     ;@@@ |1460| 

           SHR     .S1     A6,A3,A5     ;@@ |1456| 
||         MV      .L2X    A5,B7        ;@@ |1404| 
||         SMPY    .M1     A7,A8,A6     ;@@@ |1455| 

           STH     .D1T1   A5,*A0++(4)  ;@@ |1457|  is normcir[i].I
||         STH     .D2T2   B7,*B5++(4)  ;@@ |1462|  is normcir[i].I
||         SHR     .S1     A6,A3,A5     ;@@@ |1461| 
||         SMPY    .M1X    B4,A8,A6     ;@@@@ |1460| 

           SHR     .S1     A6,A3,A5     ;@@@ |1456| 
||         MV      .L2X    A5,B7        ;@@@ |1404| 
||         SMPY    .M1     A7,A8,A6     ;@@@@ |1455| 

           STH     .D1T1   A5,*A0++(4)  ;@@@ |1457|  is normcir[i].I
||         STH     .D2T2   B7,*B5++(4)  ;@@@ |1462|  is normcir[i].I
||         SHR     .S1     A6,A3,A5     ;@@@@ |1461| 

           SHR     .S1     A6,A3,A5     ;@@@@ |1456| 
||         MV      .L2X    A5,B7        ;@@@@ |1404| 

           STH     .D1T1   A5,*A0++(4)  ;@@@@ |1457|  is normcir[i].I
||         STH     .D2T2   B7,*B5++(4)  ;@@@@ |1462|  is normcir[i].I

;** --------------------------------------------------------------------------*
           MVC     .S2     B8,CSR       ; |1432| 
           B       .S2     B3
           NOP             4
           ADD     .L2     8,SP,SP
           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _varCalcRho                                                 *
;*                                                                            *
;*   Regs Modified     : A0,A1,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B4,B5,B6,B7,B8,  *
;*                           B9,SP                                            *
;*   Regs Used         : A0,A1,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,  *
;*                           B8,B9,SP                                         *
;******************************************************************************
_varCalcRho:
;** --------------------------------------------------------------------------*
;
;/*************************************************************************
;**
;** varCalcRho :        calcRho when CIR_SIZE!=5
;** -----------
;**   Compute the autocorrelation of the Cir with cirNorm.
;**   calcRho will be used for the VITERBI algorithm as the
;**   real channel response.
;**
;** Input data  - Channel impulse response : cir[CIR_LENGTH].{I,Q}
;**             - Normalized channel impulse response : cirNorm[CIR_SIZE].{I,Q}
;**             - Position of the best CIR window : indexBest
;**             - Exponent of the normalized energy : exponent
;**
;** Ouput data  - Autocorrelation of the Cir : rho[DELAY_MAX]
;**
;** This illistrates the NB case where index = 0 and CIR_SIZE = 5:
;**   When i = 1, j = 1:4, j-i = 0:3
;**        i = 2, j = 2:4, j-i = 0:2
;**        i = 3, j = 3:4, j-i = 0:1
;**        i = 4, j = 4,   j-i = 0
;**
;** In this manner the autocorrelation is commputed on the samples except
;** the i = 0 delay which is normalized by the mantissa and exponent.
;**
;** NOTE: as of 9/98 rho is not in A|RT.  It will be ported when viterbi
;** is ported to A|RT.
;**
;***************************************************************************/
;void calcRho
;(
;   t_ULCirSamp cir[MAX_CIR_LENGTH], 
;   t_ULCirSamp cirNorm[MAX_CIR_LENGTH], 
;   Char  indexBest, 
;   Short exponent, 
;   Word3 rho[DELAY_MAX] 
;);
;
;   This routine works with a variable CIR_SIZE, BEST_CIR_LENGTH. This
;   is a fallback, in case the variability is needed. A much faster
;   (4.5 times) version is defined below, that unrolls the entire
;   loop for speed, in the case that CIR_SIZE=BEST_CIR_LENGTH=5. In
;   the case CIR_SIZE=4, its better to modify that procedure below. If
;   CIR_SIZE changes to 6, that's long enough to pipeline the loops
;   in this routine, and it will run far more efficiently.
;
; _varCalcRho        .cproc cirptr,ncirptr,best,expon,rhoptr
;     .reg cptrstart
;     .reg cptr0,cptr1
;     .reg cnptr0,cnptr1
;     .reg icnt,jcnt
;     .reg qtemp,itemp
;     .reg temp,tempn
;     .reg rptr
;     .reg iloop
;     .reg maxcir
;     .reg cirsiz
;     .reg wsize
;     .reg result
;     .reg cond
;     .reg ccp
           MVK     .S1     _c_asm_consts,A3 ; |1537| 
           MVKH    .S1     _c_asm_consts,A3 ; |1538|  load address of C constants
           LDW     .D1T1   *+A3(64),A0  ; |1541| 
           NOP             1
           LDW     .D1T2   *+A3(56),B8  ; |1540| 
           NOP             1
           MV      .L2X    A6,B5        ; |1516| 

           ADD     .L1X    B6,A0,A9     ; |1541| 
||         MV      .L2X    A4,B7        ; |1516| 

           CMPGT   .L1     0x0,A9,A1    ; |1568|  make expon positive
||         ADDAW   .D2     B7,B5,B7     ; |1564| 
||         ZERO    .L2     B9           ; |1562|  outer loop index zeroed

   [ A1]   NEG     .L1     A9,A9        ; |1569|  cond remembers if this done
||         ADDAW   .D2     B7,1,B7      ; |1565|  cirptr => cir[1+best]
||         SUB     .L2     B8,0x1,B2    ; |1563|  outer loop for CIR_SIZE-1
||         SUB     .S2     SP,8,SP      ; |1516| 
||         MV      .S1X    B4,A7        ; |1516| 

;** --------------------------------------------------------------------------*
rhoouterloop:        

           ADD     .L2     0x1,B9,B9    ; |1583|  iloop index advances
||         ADDAW   .D2     B7,B9,B4     ; |1581|  cptr0 => cir[1+best+iloop]

           SUB     .L2     B8,B9,B0     ; |1584|  jcnt = cirsiz-1-iloop    
           CMPGTU  .L2     B0,4,B1      ; |1584| 
   [ B1]   B       .S1     L66          ; |1584| 
           NOP             4

           MV      .L1     A7,A5        ; |1580|  cnptr0 => normed cir[0]
||         ZERO    .S1     A6           ; |1576| 

           ; BRANCH OCCURS ; |1584| 
;** --------------------------------------------------------------------------*
L64:        

           LDW     .D2T2   *B4++,B6     ; |1593| 
||         LDW     .D1T1   *A5++,A0     ; |1592| 

           NOP             1
           SUB     .L2     B0,0x1,B0    ; |1599|  cirsiz-1-i times
   [ B0]   B       .S1     L64          ; |1600| 
           NOP             1

           MPY     .M1X    B6,A0,A3     ; |1594| 
||         MV      .L2X    A0,B5        ; |1594| 

           MPYH    .M2     B6,B5,B5     ; |1596| 
           ADD     .L1     A6,A3,A6     ; |1595| 
           ADD     .L1X    A6,B5,A6     ; |1595| 
           ; BRANCH OCCURS ; |1600| 
;** --------------------------------------------------------------------------*
           B       .S1     L72          ; |1600| 
           NOP             4
           SUB     .L2     B2,0x1,B2    ; |1612| 
           ; BRANCH OCCURS ; |1600| 
;** --------------------------------------------------------------------------*
L66:        
           MVC     .S2     CSR,B1       ; |1600| 
           AND     .L2     -2,B1,B5     ; |1600| 

           MVC     .S2     B5,CSR       ; |1600| 
||         SUB     .L2     B0,4,B0      ; |1600| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : rhoinnerloop
;*      Loop Carried Dependency Bound(^) : 2
;*      Unpartitioned Resource Bound     : 1
;*      Partitioned Resource Bound(*)    : 2
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     0        0     
;*      .S units                     0        1     
;*      .D units                     1        1     
;*      .M units                     1        1     
;*      .X cross paths               2*       1     
;*      .T address paths             1        1     
;*      Long read paths              0        0     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           1        0     (.L or .S unit)
;*      Addition ops (.LSD)          2        1     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             1        1     
;*      Bound(.L .S .D .LS .LSD)     2*       1     
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 2  Schedule found with 5 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : Unknown
;*
;*----------------------------------------------------------------------------*
L67:        ; PIPED LOOP PROLOG

           LDW     .D1T1   *A5++,A0     ; |1592| 
||         LDW     .D2T2   *B4++,B5     ; |1593| 

           NOP             1

           LDW     .D1T1   *A5++,A0     ;@ |1592| 
||         LDW     .D2T2   *B4++,B5     ;@ |1593| 

   [ B0]   SUB     .L2     B0,0x1,B0    ; |1599|  cirsiz-1-i times

   [ B0]   B       .S2     rhoinnerloop ; |1600| 
||         LDW     .D1T1   *A5++,A0     ;@@ |1592| 
||         LDW     .D2T2   *B4++,B5     ;@@ |1593| 

           MPYH    .M2X    B5,A0,B6     ; |1596| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@ |1599|  cirsiz-1-i times

           MPY     .M1X    B5,A0,A3     ; |1594| 
|| [ B0]   B       .S2     rhoinnerloop ;@ |1600| 
||         LDW     .D1T1   *A5++,A0     ;@@@ |1592| 
||         LDW     .D2T2   *B4++,B5     ;@@@ |1593| 

           MV      .L1X    B6,A4        ; |1516| 
||         MPYH    .M2X    B5,A0,B6     ;@ |1596| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@@ |1599|  cirsiz-1-i times

;** --------------------------------------------------------------------------*
rhoinnerloop:        ; PIPED LOOP KERNEL

           ADD     .L1     A6,A3,A3     ; ^ |1595| 
||         MPY     .M1X    B5,A0,A3     ;@ |1594| 
|| [ B0]   B       .S2     rhoinnerloop ;@@ |1600| 
||         LDW     .D1T1   *A5++,A0     ;@@@@ |1592| 
||         LDW     .D2T2   *B4++,B5     ;@@@@ |1593| 

           ADD     .S1     A3,A4,A6     ; ^ |1597| 
||         MV      .L1X    B6,A4        ;@ |1516| 
||         MPYH    .M2X    B5,A0,B6     ;@@ |1596| 
|| [ B0]   SUB     .L2     B0,0x1,B0    ;@@@ |1599|  cirsiz-1-i times

;** --------------------------------------------------------------------------*
L69:        ; PIPED LOOP EPILOG

           ADD     .L1     A6,A3,A3     ;@ ^ |1595| 
||         MPY     .M1X    B5,A0,A3     ;@@ |1594| 

           ADD     .S1     A3,A4,A6     ;@ ^ |1597| 
||         MV      .L1X    B6,A4        ;@@ |1516| 
||         MPYH    .M2X    B5,A0,B6     ;@@@ |1596| 

           ADD     .L1     A6,A3,A3     ;@@ ^ |1595| 
||         MPY     .M1X    B5,A0,A3     ;@@@ |1594| 

           ADD     .S1     A3,A4,A6     ;@@ ^ |1597| 
||         MV      .L1X    B6,A4        ;@@@ |1516| 
||         MPYH    .M2X    B5,A0,B6     ;@@@@ |1596| 

           ADD     .L1     A6,A3,A3     ;@@@ ^ |1595| 
||         MPY     .M1X    B5,A0,A3     ;@@@@ |1594| 

           ADD     .S1     A3,A4,A6     ;@@@ ^ |1597| 
||         MV      .L1X    B6,A4        ;@@@@ |1516| 

           ADD     .L1     A6,A3,A3     ;@@@@ ^ |1595| 
           ADD     .S1     A3,A4,A6     ;@@@@ ^ |1597| 
;** --------------------------------------------------------------------------*
           MVC     .S2     B1,CSR       ; |1600| 
;** --------------------------------------------------------------------------*
L71:        
           SUB     .L2     B2,0x1,B2    ; |1612| 
;** --------------------------------------------------------------------------*
L72:        
   [ B2]   B       .S1     rhoouterloop ; |1613| 
           NOP             2
   [!A1]   SHR     .S1     A6,A9,A6     ; |1609| 
   [ A1]   SHL     .S1     A6,A9,A6     ; |1608| 
           STH     .D1T1   A6,*A8++     ; |1610| 
           ; BRANCH OCCURS ; |1613| 
;** --------------------------------------------------------------------------*
           B       .S2     B3
           NOP             4
           ADD     .L2     8,SP,SP
           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _calcRho                                                    *
;*                                                                            *
;*   Regs Modified     : A0,A1,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B4,B5,B6,B7,B8,  *
;*                           B9,SP                                            *
;*   Regs Used         : A0,A1,A3,A4,A5,A6,A7,A8,A9,B0,B1,B2,B3,B4,B5,B6,B7,  *
;*                           B8,B9,SP                                         *
;******************************************************************************
_calcRho:
;** --------------------------------------------------------------------------*
;
;
;/*************************************************************************
;**
;** calcRho :       calcRho when CIR_SIZE==5
;** -----------
;**   Compute the autocorrelation of the Cir with cirNorm.
;**   calcRho will be used for the VITERBI algorithm as the
;**   real channel response.
;**
;** This illustrates the NB case where index = 0 and CIR_SIZE = 5:
;**   When i = 1, j = 1:4, j-i = 0:3
;**        i = 2, j = 2:4, j-i = 0:2
;**        i = 3, j = 3:4, j-i = 0:1
;**        i = 4, j = 4,   j-i = 0
;** In this manner the autocorrelation is commputed on the samples except
;** the i = 0 delay which is normalized by the mantissa and exponent.
;**
;** Input data  - Channel impulse response : cir[CIR_LENGTH].{I,Q}
;**             - Normalized channel impulse response : cirNorm[CIR_SIZE].{I,Q}
;**             - Position of the best CIR window : indexBest
;**             - Exponent of the normalized energy : exponent
;**
;** Ouput data  - Autocorrelation of the Cir : rho[DELAY_MAX]
;**
;** Limitations - CIR_SIZE is assumed a constant 5 for unrolling. If it
;**               changes, call varCalcRho above.
;**
;** NOTE: as of 9/98 rho is not in A|RT.  It will be ported when viterbi
;** is ported to A|RT.
;**
;*************************************************************************/
;
; _calcRho     .cproc cirptr,ncirptr,best,expon,rhoptr
;     .reg cptr0
;     .reg cnptr0
;     .reg temp,tempn
;     .reg temp1,tempn1
;     .reg temp2,tempn2
;     .reg temp3,tempn3
;     .reg qtemp,itemp
;     .reg qtemp1,itemp1
;     .reg qtemp2,itemp2
;     .reg qtemp3,itemp3
;     .reg maxcir
;     .reg wsize
;     .reg result,iresult
;     .reg cond
;     .reg ccp
           SUB     .L2     SP,8,SP      ; |1652| 
                ; completely unrolled for CIR_SIZE=5, 95 clocks
           ADDAW   .D1     A4,A6,A0     ; |1700| 
           ADDAW   .D1     A0,1,A0      ; |1701|  cptr0 => cir[0][1+best]
           LDW     .D1T1   *A0,A6       ; |1718| 
           LDW     .D2T2   *B4,B0       ; |1717| 
           NOP             1
           LDW     .D2T1   *+B4(4),A3   ; |1732| 
           LDW     .D1T1   *+A0(4),A5   ; |1732| 
           MV      .L2X    A6,B5        ; |1746| 
           MPY     .M2     B5,B0,B8     ; |1719| 

           MPYH    .M2     B5,B0,B0     ; |1721| 
||         LDW     .D1T1   *+A0(8),A9   ; |1746| 
||         LDW     .D2T2   *+B4(8),B2   ; |1745| 
||         MVK     .S2     _c_asm_consts,B9 ; |1674| 

           ZERO    .L2     B5           ; |1721| 
||         LDW     .D1T1   *+A0(12),A7  ; |1760| 
||         MVKH    .S2     _c_asm_consts,B9 ; |1675|  load address of C constants

           ADD     .L2     B5,B8,B5     ; |1720| 
||         MV      .L1X    B0,A0        ; |1760| 
||         MPY     .M1     A5,A3,A4     ; |1733| 
||         LDW     .D2T2   *+B9(64),B9  ; |1678| 
||         MV      .S2X    A0,B7        ; |1718| 

           ADD     .L1X    B5,A0,A0     ; |1722| 
||         LDW     .D2T2   *+B4(12),B1  ; |1759| 

           MV      .L1X    B7,A4        ; |1734| 
||         ADD     .S1     A0,A4,A0     ; |1734| 
||         MPYH    .M1     A5,A3,A3     ; |1735| 

           LDW     .D1T1   *+A4(4),A6   ; |1793| 
||         MPY     .M2X    A9,B2,B8     ; |1747| 

           MV      .L2X    A7,B2        ; |1736| 
||         ADD     .L1     A0,A3,A0     ; |1736| 
||         MPYH    .M1X    A9,B2,A4     ; |1749| 

           ADD     .L2X    A0,B8,B5     ; |1736| 
||         ADD     .S2     B6,B9,B6     ; |1696| 

           MPY     .M2     B2,B1,B8     ; |1761| 
||         CMPGT   .L1X    0x0,B6,A1    ; |1750| 

           MV      .L2X    A7,B5        ; |1748| 
||         ADD     .L1X    B5,A4,A0     ; |1748| 

           MV      .L1X    B6,A0        ; |1750| 
||         ADD     .L2X    A0,B8,B5     ; |1750| 
||         MPYH    .M2     B5,B1,B1     ; |1763| 
||         LDW     .D2T2   *+B4(4),B9   ; |1806| 

   [ A1]   NEG     .L1     A0,A0        ; |1698| 
||         MV      .S1X    B7,A3        ; |1748| 

   [ A1]   MV      .L2X    A0,B6        ; |1764| 
||         MV      .L1X    B7,A4        ; |1762| 
||         LDW     .D2T2   *B4,B0       ; |1792| 

           MV      .L1X    B6,A0        ; |1764| 
||         ADD     .L2     B5,B1,B5     ; |1764| 

           MV      .L1X    B5,A4        ; |1764| 
||         LDW     .D1T1   *+A4(12),A9  ; |1821| 

   [!A1]   SHR     .S2     B5,B6,B9     ; |1773| 
|| [ A1]   SHL     .S1     A4,A0,A0     ; |1772| 
||         MV      .L2X    A6,B8        ; |1764| 
||         MV      .L1X    B9,A3        ; |1750| 
||         LDW     .D1T1   *+A3(8),A5   ; |1807| 

   [!A1]   MV      .L2     B9,B5        ; |1772| 
|| [ A1]   MV      .S2X    A0,B5        ; |1772| 

           MPY     .M2     B8,B0,B8     ; |1794| 
||         MV      .L2X    A6,B5        ; |1772| 
||         MV      .L1X    B5,A0        ; |1772| 

           ZERO    .L2     B5           ; |1780| 
||         MPYH    .M2     B5,B0,B9     ; |1796| 
||         LDW     .D2T2   *+B4(8),B2   ; |1820| 

           ADD     .L2     B5,B8,B5     ; |1795| 

           MPY     .M1     A5,A3,A4     ; |1808| 
||         MV      .L1X    B5,A6        ; |1795| 

           ADD     .L1X    A6,B9,A0     ; |1795| 
||         STH     .D1T1   A0,*A8++     ; |1774| 

           MPYH    .M1     A5,A3,A3     ; |1810| 
||         MV      .L1X    B7,A4        ; |1809| 
||         ADD     .S1     A0,A4,A0     ; |1809| 

           MPY     .M2X    A9,B2,B8     ; |1822| 
||         LDW     .D1T1   *+A4(12),A6  ; |1867| 
||         MV      .L1X    B7,A7        ; |1795| 

           MPYH    .M1X    A9,B2,A4     ; |1824| 
||         ADD     .L1     A0,A3,A0     ; |1811| 
||         LDW     .D1T1   *+A7(8),A7   ; |1853| 

           LDW     .D2T2   *B4,B1       ; |1852| 
||         ADD     .L2X    A0,B8,B5     ; |1823| 

           ADD     .L1X    B5,A4,A0     ; |1825| 

           MV      .L1     A0,A4        ; |1825| 
||         MV      .L2X    A0,B5        ; |1825| 

           LDW     .D2T2   *+B4(4),B0   ; |1866| 
||         MV      .L1X    B4,A0        ; |1825| 

           MV      .L2X    A7,B8        ; |1825| 
||         MV      .L1X    B6,A3        ; |1825| 

           MPY     .M2     B8,B1,B8     ; |1854| 
|| [ A1]   SHL     .S1     A4,A3,A0     ; |1833| 
||         LDW     .D1T1   *A0,A3       ; |1894| 
||         MV      .L2X    A7,B4        ; |1833| 

           ZERO    .L1     A0           ; |1841| 
||         MPYH    .M2     B4,B1,B1     ; |1856| 
|| [ A1]   MV      .L2X    A0,B5        ; |1833| 

           ADD     .L1X    A0,B8,A0     ; |1841| 
||         MV      .L2X    A6,B4        ; |1841| 

           MPY     .M2     B4,B0,B8     ; |1868| 
|| [!A1]   SHR     .S2     B5,B6,B9     ; |1834| 

           MPYH    .M2     B4,B0,B4     ; |1870| 
|| [!A1]   MV      .L2     B9,B5        ; |1833| 

           ADD     .L2X    A0,B1,B5     ; |1868| 
||         STH     .D1T2   B5,*A8++     ; |1835| 

           LDW     .D2T1   *+B7(12),A5  ; |1895| 
||         ADD     .L2     B5,B8,B5     ; |1869| 
||         MV      .L1X    B4,A0        ; |1870| 

           ADD     .L1X    B5,A0,A0     ; |1871| 

           MV      .L2X    A0,B5        ; |1871| 
||         MV      .L1X    B6,A4        ; |1871| 

   [ A1]   SHL     .S1     A0,A4,A0     ; |1879| 
|| [!A1]   SHR     .S2     B5,B6,B4     ; |1880| 

   [!A1]   MV      .L2     B4,B5        ; |1896| 
|| [ A1]   MV      .S2X    A0,B5        ; |1896| 

           MV      .L1X    B5,A0        ; |1896| 
||         MPY     .M1     A5,A3,A4     ; |1896| 

           MPYH    .M1     A5,A3,A3     ; |1898| 
||         ZERO    .L1     A0           ; |1887| 
||         STH     .D1T1   A0,*A8++     ; |1881| 

           ADD     .L1     A0,A4,A0     ; |1897| 

           B       .S2     B3
||         ADD     .L1     A0,A3,A0     ; |1899| 

           MV      .L1X    B6,A0        ; |1899| 
||         MV      .S1     A0,A3        ; |1899| 
||         MV      .L2X    A0,B5        ; |1899| 

   [ A1]   SHL     .S1     A3,A0,A0     ; |1907| 
||         MV      .L2     B6,B4        ; |1899| 

   [!A1]   SHR     .S2     B5,B4,B5     ; |1907| 
|| [ A1]   MV      .L2X    A0,B5        ; |1907| 

           MV      .L1X    B5,A0        ; |1907| 

           STH     .D1T1   A0,*A8++     ; |1909| 
||         ADD     .L2     8,SP,SP

           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _matchFilter                                                *
;*                                                                            *
;*   Regs Modified     : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,B0,*
;*                           B1,B2,B3,B4,B5,B6,B7,B8,B9,B10,B11,B12,B13,SP    *
;*   Regs Used         : A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,B0,*
;*                           B1,B2,B3,B4,B5,B6,B7,B8,B9,B10,B11,B12,B13,SP    *
;******************************************************************************
_matchFilter:
;** --------------------------------------------------------------------------*
;
;
;/*************************************************************************
;**
;** matchFilter :
;** -----------
;**   Matched filter the input samples with the normalized CIR 
;**
;** Correlation of Cir and cirNorm
;**
;**  Note that the maximum of the CIR's aoutcorrelation is:
;**   max( autocorr(cir) ) = energySum = mantissa * 2^exponent 
;**
;**  and that the maximum of the CIR and cirNorms autcorrelation is:
;**   max( corr(cir,cirNorm) ) = energySum/mantissa = 2^exponent 
;**
;**  Because cirNorm = Cir/mantissa
;**
;**  Typically the exponent will be negative, so when Rho is shifted
;**  by the exponent, it is actually reducing the number of sign bits and
;**  maxmizing the fixed point precision.
;**
;** Input data     - Input samples : data[2][]
;**                - channel impulse response : cirNorm[CIR_SIZE].{I,Q}
;**                - Position of the best CIR window : indexBest
;**                - Exponent of the normalized energy : exponent
;**
;** Output data    - Matched filtered samples : Output[nb_samp]
;**
;** Limitations    - CIR_SIZE is assumed a constant 5 for unrolling. If it
;**                  changes, change the number of blocks in the loop below.
;** 
;***************************************************************************/
;
;void  matchFilter
;(
;   t_ULComplexSamp   samples[BURST_SIZE_UL_MAX], 
;   t_ULCirSamp       cirNorm[MAX_CIR_LENGTH],
;   Char              indexBest_fxp,
;   Short             exponent,
;   Word3             output[]
;);
;
; _matchFilter    .cproc samptr,ncirptr,best,expon,outptr
;     .reg numbits                        ; const: g_BConsts_pt->numBits
;     .reg sptr0,sptr1                    ; pointers
;     .reg acc1                           ; loop accumulator
;     .reg temp,qtemp,itemp               ; temp storage
;     .reg nciq0,nciq1,nciq2,nciq3,nciq4  ; normedcir samples Q:I in word
;     .reg expsgn                         ; sign of exponent
;     .reg jcnt                           ; loop counter
;     .reg bcp                            ; constants pointers
;     .reg ccp
           MVK     .S1     _g_BConsts_pt,A0 ; |1980| 

           MVKH    .S1     _g_BConsts_pt,A0 ; |1981|  load address of pointer
||         MVK     .S2     _c_asm_consts,B5 ; |1983| 

           LDW     .D1T2   *A0,B7       ; |1982|  load pointer to burst constants
||         MVKH    .S2     _c_asm_consts,B5 ; |1984|  load address of C constants

           LDW     .D2T1   *+B5(24),A3  ; |1990| 
           NOP             3
           MV      .L1X    B7,A0        ; |1982| 

           LDB     .D1T1   *+A0[A3],A3  ; |1991| 
||         MV      .L1X    B5,A5        ; |1991| 

           LDW     .D1T1   *+A5(36),A5  ; |1993| 
           NOP             3
           ADD     .L1     A3,A6,A6     ; |1991| 
           LDBU    .D1T1   *+A0[A5],A3  ; |1994| 
           STW     .D2T2   B13,*SP--(48) ; |1957| 
           STW     .D2T1   A10,*+SP(12) ; |1957| 
           STW     .D2T1   A12,*+SP(20) ; |1957| 
           STW     .D2T2   B11,*+SP(40) ; |1957| 

           MV      .L1X    B5,A3        ; |1995| 
||         ADD     .S1     A3,A6,A6     ; |1995| 
||         STW     .D2T1   A11,*+SP(16) ; |1957| 

           LDW     .D1T1   *+A3(64),A3  ; |1999| 
||         STW     .D2T2   B12,*+SP(44) ; |1957| 

           STW     .D2T1   A14,*+SP(28) ; |1957| 

           MV      .L1X    B5,A5        ; |1957| 
||         LDW     .D2T1   *B4,A10      ; |2011|  read all 5 normed cir values

           LDW     .D1T2   *+A5(28),B8  ; |1996| 
||         LDW     .D2T1   *+B4(8),A12  ; |2013| 

           STW     .D2T2   B3,*+SP(32)  ; |1957| 

           ADD     .L2X    A3,B6,B6     ; |1999| 
||         LDW     .D2T2   *+B4(16),B7  ; |2015| 

           LDW     .D2T1   *+B4(4),A3   ; |2012| 

           MV      .L2X    A0,B4        ; |2002| 
||         LDW     .D2T2   *+B4(12),B5  ; |2014| 

           LDBU    .D2T2   *+B4[B8],B0  ; |1997| 

           CMPGT   .L1X    0x0,B6,A1    ; |2002|  and shift direction
||         STW     .D2T2   B10,*+SP(36) ; |1957| 

   [ A1]   NEG     .L2     B6,B6        ; |2003|  expsgn is sign, expon > 0
||         MV      .L1X    B7,A11       ; |2019| 
||         MV      .S2X    A8,B11       ; |1957| 
||         STW     .D2T1   A13,*+SP(24) ; |1957| 

           MVC     .S2     CSR,B13      ; |2019| 
||         MV      .L1X    B6,A14       ; |2019| 
||         MV      .L2X    A3,B4        ; |2019| 
||         ADDAW   .D1     A4,A6,A2     ; |2019|  sptr0 => samples[start]
||         MV      .D2     B7,B3        ; |2019| 

           AND     .L2     -2,B13,B6    ; |2019| 

           MVC     .S2     B6,CSR       ; |2019| 
||         SUB     .L2     B0,6,B0      ; |2019| 

;*----------------------------------------------------------------------------*
;*   SOFTWARE PIPELINE INFORMATION
;*
;*      Loop label : mfloop
;*      Known Minimum Trip Count         : 36
;*      Known Max Trip Count Factor      : 1
;*      Loop Carried Dependency Bound(^) : 1
;*      Unpartitioned Resource Bound     : 5
;*      Partitioned Resource Bound(*)    : 5
;*      Resource Partition:
;*                                A-side   B-side
;*      .L units                     3        4     
;*      .S units                     3        4     
;*      .D units                     5*       1     
;*      .M units                     5*       5*    
;*      .X cross paths               3        5*    
;*      .T address paths             4        2     
;*      Long read paths              1        0     
;*      Long write paths             0        0     
;*      Logical  ops (.LS)           0        0     (.L or .S unit)
;*      Addition ops (.LSD)          1        0     (.L or .S or .D unit)
;*      Bound(.L .S .LS)             3        4     
;*      Bound(.L .S .D .LS .LSD)     4        3     
;*
;*      Searching for software pipeline schedule at ...
;*         ii = 5  Schedule found with 6 iterations in parallel
;*      Done
;*
;*      Speculative Load Threshold : Unknown
;*
;*----------------------------------------------------------------------------*
L78:        ; PIPED LOOP PROLOG
; mfloop:         .trip 36                
           MV      .L1     A2,A0        ; ^ |2028| 
           LDW     .D1T1   *A0,A5       ; |2033|  10 multiplies, 5 clocks
           NOP             1
           MV      .L1     A0,A4        ; |1997| 

           ADD     .L1     0x4,A2,A2    ; ^ |2074|  advance one sample 
||         LDW     .D1T1   *+A4(4),A0   ; |2038|  multiply and accumulate

           MV      .L1     A2,A0        ;@ ^ |2028| 

           MPYH    .M1     A5,A10,A5    ; |2035| 
||         LDW     .D1T1   *A0,A5       ;@ |2033|  10 multiplies, 5 clocks

           MV      .S1     A4,A6        ; |1997| 
||         LDW     .D1T1   *+A4(8),A8   ; |2044| 
||         MPY     .M1     A5,A10,A3    ; |2034|  loop is optimal

           LDW     .D1T2   *+A6(12),B7  ; |2050| 
||         MV      .L1     A0,A4        ;@ |1997| 

           MPY     .M2X    A0,B4,B6     ; |2039| 
||         ADD     .L1     0x4,A2,A2    ;@ ^ |2074|  advance one sample 
||         LDW     .D1T1   *+A4(4),A0   ;@ |2038|  multiply and accumulate

           LDW     .D1T2   *+A6(16),B1  ; |2056| 
||         MPYH    .M2X    A0,B4,B9     ; |2040| 
||         ADD     .S1     A3,A5,A3     ; |2036| 
||         MV      .L1     A2,A0        ;@@ ^ |2028| 

           MPYH    .M1     A5,A10,A5    ;@ |2035| 
||         LDW     .D1T1   *A0,A5       ;@@ |2033|  10 multiplies, 5 clocks

           ADD     .S2X    B6,A3,B6     ; |2041| 
||         MV      .S1     A4,A6        ;@ |1997| 
||         LDW     .D1T1   *+A4(8),A8   ;@ |2044| 
||         MPY     .M1     A5,A10,A3    ;@ |2034|  loop is optimal

           MPY     .M2     B7,B5,B2     ; |2051| 
||         MPYH    .M1     A8,A12,A9    ; |2046| 
||         LDW     .D1T2   *+A6(12),B7  ;@ |2050| 
||         MV      .L1     A0,A4        ;@@ |1997| 

           MPY     .M1     A8,A12,A6    ; |2045| 
||         MPY     .M2X    A0,B4,B6     ;@ |2039| 
||         ADD     .L1     0x4,A2,A2    ;@@ ^ |2074|  advance one sample 
||         LDW     .D1T1   *+A4(4),A0   ;@@ |2038|  multiply and accumulate

           MPY     .M1X    B1,A11,A7    ; |2057| 
||         ADD     .S2     B9,B6,B8     ; |2042| 
||         LDW     .D1T2   *+A6(16),B1  ;@ |2056| 
||         MPYH    .M2X    A0,B4,B9     ;@ |2040| 
||         ADD     .S1     A3,A5,A3     ;@ |2036| 
||         MV      .L1     A2,A0        ;@@@ ^ |2028| 

           MPYH    .M2     B7,B5,B8     ; |2052| 
||         ADD     .S2X    A6,B8,B10    ; |2047| 
||         MPYH    .M1     A5,A10,A5    ;@@ |2035| 
||         LDW     .D1T1   *A0,A5       ;@@@ |2033|  10 multiplies, 5 clocks

           MPYH    .M2     B1,B3,B12    ; |2058| 
||         ADD     .S2X    B6,A3,B6     ;@ |2041| 
||         MV      .S1     A4,A6        ;@@ |1997| 
||         LDW     .D1T1   *+A4(8),A8   ;@@ |2044| 
||         MPY     .M1     A5,A10,A3    ;@@ |2034|  loop is optimal

           ADD     .L2X    A9,B10,B1    ; |2048| 
||         MPY     .M2     B7,B5,B2     ;@ |2051| 
||         MPYH    .M1     A8,A12,A9    ;@ |2046| 
||         LDW     .D1T2   *+A6(12),B7  ;@@ |2050| 
||         MV      .L1     A0,A4        ;@@@ |1997| 

           ADD     .L2     B2,B1,B10    ; |2053| 
||         MPY     .M1     A8,A12,A6    ;@ |2045| 
||         MPY     .M2X    A0,B4,B6     ;@@ |2039| 
||         ADD     .L1     0x4,A2,A2    ;@@@ ^ |2074|  advance one sample 
||         LDW     .D1T1   *+A4(4),A0   ;@@@ |2038|  multiply and accumulate

           ADD     .L2     B8,B10,B9    ; |2054| 
||         MPY     .M1X    B1,A11,A7    ;@ |2057| 
||         ADD     .S2     B9,B6,B8     ;@ |2042| 
||         LDW     .D1T2   *+A6(16),B1  ;@@ |2056| 
||         MPYH    .M2X    A0,B4,B9     ;@@ |2040| 
||         ADD     .S1     A3,A5,A3     ;@@ |2036| 
||         MV      .L1     A2,A0        ;@@@@ ^ |2028| 

           ADD     .L1X    A7,B9,A6     ; |2059| 
||         MPYH    .M2     B7,B5,B8     ;@ |2052| 
||         ADD     .S2X    A6,B8,B10    ;@ |2047| 
||         MPYH    .M1     A5,A10,A5    ;@@@ |2035| 
||         LDW     .D1T1   *A0,A5       ;@@@@ |2033|  10 multiplies, 5 clocks

           ADD     .L1X    B12,A6,A13   ; |2060| 
||         MPYH    .M2     B1,B3,B12    ;@ |2058| 
||         ADD     .S2X    B6,A3,B6     ;@@ |2041| 
||         MV      .S1     A4,A6        ;@@@ |1997| 
||         LDW     .D1T1   *+A4(8),A8   ;@@@ |2044| 
||         MPY     .M1     A5,A10,A3    ;@@@ |2034|  loop is optimal

   [ B0]   B       .S2     mfloop       ; |2077|  numBits loops
|| [!A1]   SHR     .S1     A13,A14,A13  ; |2071| 
||         ADD     .L2X    A9,B10,B1    ;@ |2048| 
||         MPY     .M2     B7,B5,B2     ;@@ |2051| 
||         MPYH    .M1     A8,A12,A9    ;@@ |2046| 
||         LDW     .D1T2   *+A6(12),B7  ;@@@ |2050| 
||         MV      .L1     A0,A4        ;@@@@ |1997| 

;** --------------------------------------------------------------------------*
mfloop:        ; PIPED LOOP KERNEL

   [ A1]   SSHL    .S1     A13,A14,A13  ; |2070| 
||         ADD     .L2     B2,B1,B10    ;@ |2053| 
||         MPY     .M1     A8,A12,A6    ;@@ |2045| 
||         MPY     .M2X    A0,B4,B6     ;@@@ |2039| 
||         ADD     .L1     0x4,A2,A2    ;@@@@ ^ |2074|  advance one sample 
||         LDW     .D1T1   *+A4(4),A0   ;@@@@ |2038|  multiply and accumulate

           STH     .D2T1   A13,*B11++   ; |2073|  store output sample
||         ADD     .L2     B8,B10,B9    ;@ |2054| 
||         MPY     .M1X    B1,A11,A7    ;@@ |2057| 
||         ADD     .S2     B9,B6,B8     ;@@ |2042| 
||         LDW     .D1T2   *+A6(16),B1  ;@@@ |2056| 
||         MPYH    .M2X    A0,B4,B9     ;@@@ |2040| 
||         ADD     .S1     A3,A5,A3     ;@@@ |2036| 
||         MV      .L1     A2,A0        ;@@@@@ ^ |2028| 

           ADD     .L1X    A7,B9,A6     ;@ |2059| 
||         MPYH    .M2     B7,B5,B8     ;@@ |2052| 
||         ADD     .S2X    A6,B8,B10    ;@@ |2047| 
||         MPYH    .M1     A5,A10,A5    ;@@@@ |2035| 
||         LDW     .D1T1   *A0,A5       ;@@@@@ |2033|  10 multiplies, 5 clocks

   [ B0]   SUB     .L2     B0,0x1,B0    ;@ |2076| 
||         ADD     .L1X    B12,A6,A13   ;@ |2060| 
||         MPYH    .M2     B1,B3,B12    ;@@ |2058| 
||         ADD     .S2X    B6,A3,B6     ;@@@ |2041| 
||         MV      .S1     A4,A6        ;@@@@ |1997| 
||         LDW     .D1T1   *+A4(8),A8   ;@@@@ |2044| 
||         MPY     .M1     A5,A10,A3    ;@@@@ |2034|  loop is optimal

   [ B0]   B       .S2     mfloop       ;@ |2077|  numBits loops
|| [!A1]   SHR     .S1     A13,A14,A13  ;@ |2071| 
||         ADD     .L2X    A9,B10,B1    ;@@ |2048| 
||         MPY     .M2     B7,B5,B2     ;@@@ |2051| 
||         MPYH    .M1     A8,A12,A9    ;@@@ |2046| 
||         LDW     .D1T2   *+A6(12),B7  ;@@@@ |2050| 
||         MV      .L1     A0,A4        ;@@@@@ |1997| 

;** --------------------------------------------------------------------------*
L80:        ; PIPED LOOP EPILOG

   [ A1]   SSHL    .S1     A13,A14,A13  ;@ |2070| 
||         ADD     .L2     B2,B1,B10    ;@@ |2053| 
||         MPY     .M1     A8,A12,A6    ;@@@ |2045| 
||         MPY     .M2X    A0,B4,B6     ;@@@@ |2039| 
||         ADD     .L1     0x4,A2,A2    ;@@@@@ ^ |2074|  advance one sample 
||         LDW     .D1T1   *+A4(4),A0   ;@@@@@ |2038|  multiply and accumulate

           STH     .D2T1   A13,*B11++   ;@ |2073|  store output sample
||         ADD     .L2     B8,B10,B9    ;@@ |2054| 
||         MPY     .M1X    B1,A11,A7    ;@@@ |2057| 
||         ADD     .S2     B9,B6,B8     ;@@@ |2042| 
||         LDW     .D1T2   *+A6(16),B1  ;@@@@ |2056| 
||         MPYH    .M2X    A0,B4,B9     ;@@@@ |2040| 
||         ADD     .S1     A3,A5,A3     ;@@@@ |2036| 

           ADD     .L1X    A7,B9,A6     ;@@ |2059| 
||         MPYH    .M2     B7,B5,B8     ;@@@ |2052| 
||         ADD     .S2X    A6,B8,B10    ;@@@ |2047| 
||         MPYH    .M1     A5,A10,A5    ;@@@@@ |2035| 

           ADD     .L1X    B12,A6,A13   ;@@ |2060| 
||         MPYH    .M2     B1,B3,B12    ;@@@ |2058| 
||         ADD     .S2X    B6,A3,B6     ;@@@@ |2041| 
||         MV      .S1     A4,A6        ;@@@@@ |1997| 
||         LDW     .D1T1   *+A4(8),A8   ;@@@@@ |2044| 
||         MPY     .M1     A5,A10,A3    ;@@@@@ |2034|  loop is optimal

   [!A1]   SHR     .S1     A13,A14,A13  ;@@ |2071| 
||         ADD     .L2X    A9,B10,B1    ;@@@ |2048| 
||         MPY     .M2     B7,B5,B2     ;@@@@ |2051| 
||         MPYH    .M1     A8,A12,A9    ;@@@@ |2046| 
||         LDW     .D1T2   *+A6(12),B7  ;@@@@@ |2050| 

   [ A1]   SSHL    .S1     A13,A14,A13  ;@@ |2070| 
||         ADD     .L2     B2,B1,B10    ;@@@ |2053| 
||         MPY     .M1     A8,A12,A6    ;@@@@ |2045| 
||         MPY     .M2X    A0,B4,B6     ;@@@@@ |2039| 

           STH     .D2T1   A13,*B11++   ;@@ |2073|  store output sample
||         ADD     .L2     B8,B10,B9    ;@@@ |2054| 
||         MPY     .M1X    B1,A11,A7    ;@@@@ |2057| 
||         ADD     .S2     B9,B6,B8     ;@@@@ |2042| 
||         LDW     .D1T2   *+A6(16),B1  ;@@@@@ |2056| 
||         MPYH    .M2X    A0,B4,B9     ;@@@@@ |2040| 
||         ADD     .S1     A3,A5,A3     ;@@@@@ |2036| 

           ADD     .L1X    A7,B9,A6     ;@@@ |2059| 
||         MPYH    .M2     B7,B5,B8     ;@@@@ |2052| 
||         ADD     .S2X    A6,B8,B10    ;@@@@ |2047| 

           ADD     .L1X    B12,A6,A13   ;@@@ |2060| 
||         MPYH    .M2     B1,B3,B12    ;@@@@ |2058| 
||         ADD     .S2X    B6,A3,B6     ;@@@@@ |2041| 

   [!A1]   SHR     .S1     A13,A14,A13  ;@@@ |2071| 
||         ADD     .L2X    A9,B10,B1    ;@@@@ |2048| 
||         MPY     .M2     B7,B5,B2     ;@@@@@ |2051| 
||         MPYH    .M1     A8,A12,A9    ;@@@@@ |2046| 

   [ A1]   SSHL    .S1     A13,A14,A13  ;@@@ |2070| 
||         ADD     .L2     B2,B1,B10    ;@@@@ |2053| 
||         MPY     .M1     A8,A12,A6    ;@@@@@ |2045| 

           STH     .D2T1   A13,*B11++   ;@@@ |2073|  store output sample
||         ADD     .L2     B8,B10,B9    ;@@@@ |2054| 
||         MPY     .M1X    B1,A11,A7    ;@@@@@ |2057| 
||         ADD     .S2     B9,B6,B8     ;@@@@@ |2042| 

           ADD     .L1X    A7,B9,A6     ;@@@@ |2059| 
||         MPYH    .M2     B7,B5,B8     ;@@@@@ |2052| 
||         ADD     .S2X    A6,B8,B10    ;@@@@@ |2047| 

           ADD     .L1X    B12,A6,A13   ;@@@@ |2060| 
||         MPYH    .M2     B1,B3,B12    ;@@@@@ |2058| 

   [!A1]   SHR     .S1     A13,A14,A13  ;@@@@ |2071| 
||         ADD     .L2X    A9,B10,B1    ;@@@@@ |2048| 

   [ A1]   SSHL    .S1     A13,A14,A13  ;@@@@ |2070| 
||         ADD     .L2     B2,B1,B10    ;@@@@@ |2053| 

           STH     .D2T1   A13,*B11++   ;@@@@ |2073|  store output sample
||         ADD     .L2     B8,B10,B9    ;@@@@@ |2054| 

           ADD     .L1X    A7,B9,A6     ;@@@@@ |2059| 
           ADD     .L1X    B12,A6,A13   ;@@@@@ |2060| 
   [!A1]   SHR     .S1     A13,A14,A13  ;@@@@@ |2071| 
   [ A1]   SSHL    .S1     A13,A14,A13  ;@@@@@ |2070| 
           STH     .D2T1   A13,*B11++   ;@@@@@ |2073|  store output sample
           NOP             3
;** --------------------------------------------------------------------------*

           LDW     .D2T2   *+SP(32),B3
||         MVC     .S2     B13,CSR      ; |2019| 

           LDW     .D2T2   *+SP(40),B11
           LDW     .D2T2   *+SP(36),B10
           LDW     .D2T1   *+SP(28),A14
           LDW     .D2T1   *+SP(24),A13
           LDW     .D2T1   *+SP(20),A12
           LDW     .D2T1   *+SP(16),A11
           LDW     .D2T1   *+SP(12),A10

           B       .S2     B3
||         LDW     .D2T2   *+SP(44),B12

           LDW     .D2T2   *++SP(48),B13
           NOP             4
           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _expX                                                       *
;*                                                                            *
;*   Regs Modified     : A0,A4,A5,B5,SP                                       *
;*   Regs Used         : A0,A4,A5,B3,B4,B5,SP                                 *
;******************************************************************************
_expX:
;** --------------------------------------------------------------------------*
;
;
;/***************************************************************************
;**
;**  expX()
;**  ------
;**
;**   This will break a number into:
;**      x = mantissa * 2^exponent
;**
;**   This uses the C6x intrinsic NORM()
;**
;**
;**   Input data:    - x, input data to break apart
;**
;**   Output data     - exponent, exponent of the "x"
;**
;**   Return data     - mantissa, mantissa of the "x"
;**
;**   Limitations     - assumes ACCUM_GUARD_BITS=8, WORD_SIZE=16
;**                     necessary to get any improvement
;**
;***************************************************************************/
;Word expX
;(
;   UVLWord data, 
;   Word *exponent 
;);
;
; _expX           .cproc data1:data0, expptr          ; returns Word
;                 .reg sptr
;                 .reg nsignb
;                 .reg temp
           B       .S2     B3
           NOP             2

           MVK     .S2     0x10,B5      ; |2445| 
||         NORM    .L1     A5:A4,A0     ; |2446| 

           SUB     .L2X    B5,A0,B5     ; |2447| 
||         SHL     .S1     A5:A4,A0,A5:A4 ; |2450| 
||         SUB     .S2     SP,8,SP      ; |2425| 

           SHR     .S1     A5:A4,0x18,A5:A4 ; |2451| 
||         STH     .D2T2   B5,*B4       ; |2448| 
||         ADD     .L2     8,SP,SP

           ; BRANCH OCCURS


;                 .endproc
	.sect	".text"

;******************************************************************************
;* FUNCTION NAME: _mag2DB                                                     *
;*                                                                            *
;*   Regs Modified     : A0,A1,A3,A4,A5,B0,B4,B5,SP                           *
;*   Regs Used         : A0,A1,A3,A4,A5,B0,B3,B4,B5,SP                        *
;******************************************************************************
_mag2DB:
;** --------------------------------------------------------------------------*
;
;/*************************************************************************** 
;*     mag2DB()
;*     -------
;*
;*   Input Data:   mag, magnitude of a value F48 with 40 actual bits
;*
;*   Return Data:  power in decibels of format F16.2
;*
;*   Limitations:  assumes ACCUM_GUARD_BITS=8, WORD_SIZE=16
;*                 needed to fold constants to get any improvement
;*
;*  Convert an unsigned long word (40bits) in the format
;*    F32  to decibels.
;*  The magin F48 because is was calculated by 
;*    F16 * F16 = F32.
;*  Therefore "31" needs to subtracted from pow2(mag) 
;*   in order to align the decimal point.
;*
;*  In floating point this would be :   
;*  If wordsize is 16 then:
;*  *power = 10 * log10( (mag >> 16)/(double)((1<<2*16)-1) );
;*
;* Note that
;*   10*log10(x) = (10*0.301)*log2(x)
;*  And 3*log2() = 3*(integer + fractional) = 3*integer + 3*fractional
;*
;* The fractional part is derived from a lookup table with
;* the 2 next significant bits after the most significant bit.
;*
;* The entries are calculated by log2(1.0),log2(1.25),log2(1.5),log2(1.75)
;*
;* logTable[4] = { 3*-1, 3*-0.6781, 3*-0.415, 3*-0.1926 }; floating 
;* Multiply float table by 4 add 1 (to round) and round to nearest integer 
;*
;***************************************************************************/
;
;Word14 mag2DB
;(
;   UVLWord mag 
;);
;
; _mag2DB         .cproc mag1:mag0    ; returns Word14
;                 .reg sptr
;                 .reg lptr
;                 .reg nsignb
;                 .reg temp
;                 .reg temp1:temp0
;                 .reg power
;                 .reg intpart
;                 .reg fracpart
;                 .reg cond
;                 .reg num

           MVK     .S1     0x24,A0      ; |2528| 
||         NORM    .L1     A5:A4,A3     ; |2518| 

           SHL     .S1     A5:A4,A3,A5:A4 ; |2529| 
           SHR     .S1     A5:A4,A0,A1:A0 ; |2530| 

           CMPLT   .L2X    0x3,A0,B0    ; |2531| 
||         MVK     .S2     _logTable,B4 ; |2513| 

           SUB     .L2X    A0,0x4,B5    ; |2531| 
||         MVKH    .S2     _logTable,B4 ; |2514| 

           B       .S2     B3
|| [ B0]   LDH     .D2T1   *+B4[B5],A0  ; |2558| 
||         ADDK    .S1     0xffffffda,A3 ; |2540| 

           NEG     .L1     A3,A3        ; |2541| 
           SUBAB   .D1     A3,31,A4     ; |2545| 
           MPY     .M1     0xc,A4,A4    ; |2546| 
           SUB     .L2     SP,8,SP      ; |2500| 

   [ B0]   ADD     .L1     A4,A0,A4     ; |2559| 
||         ADD     .L2     8,SP,SP

           ; BRANCH OCCURS


;                 .endproc
