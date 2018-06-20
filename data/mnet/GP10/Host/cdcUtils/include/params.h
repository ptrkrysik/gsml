/****************************************************************************/
/*  PARAMS.H  - Target dependent parameters.                                */
/****************************************************************************/

#if TMS340
typedef unsigned long  T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned short T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned short T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned long  T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned short T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC         MAGIC_340
/* #define NEW_SYM_NDX   1 USE 32-bit SYMBOL INDEX, NO DISP */
#define BYTETOLOC(x)  ((T_ADDR)(x)<<3)  /* CONVERT ADDRESSES TO BYTES       */
#define LOCTOBYTE(x)  ((x)>>3)          /* CONVERT BYTES TO ADDRESSES       */
#define BIT_OFFSET(a) ((a)&7)           /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x1000            /* 16K BUFFER FOR LOADING DATA      */
#endif

#define LOADWORDSIZE  2                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    1                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    2                 /* SIZE IN BYTES OF INIT DATA ITEMS */
#endif

#if TMS32030 || TMS32040
typedef unsigned long  T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned long  T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned long  T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned long  T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned long  T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC MAGIC_C30                 /* Magic number for C30             */

#define LOCTOBYTE(x)   ((x)<<2)         /* C30 word addrs to byte addrs     */
#define BYTETOLOC(x)   ((x)>>2)         /* byte addrs to word addrs         */
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x0FF8            /* 4K-8 BUFFER FOR LOADING DATA     */
#endif

#define LOADWORDSIZE  2                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    1                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    4                 /* SIZE IN BYTES OF INIT DATA ITEMS */
#endif

#if TMS32025 || TMS32050
typedef unsigned short T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned short T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned short T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned short T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned short T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC MAGIC_DSP                 /* Magic number for C25            */

#define LOCTOBYTE(x)   ((x)<<1)         /* 16-bit word addrs to byte addrs */
#define BYTETOLOC(x)   ((x)>>1)         /* byte addrs to word addrs        */
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif /* ifdef OTIS */

#define LOADWORDSIZE  2                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    1                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    2                 /* SIZE IN BYTES OF INIT DATA ITEMS */

#endif /* TMS32025 || TMS32050 */

#if TMS380
typedef unsigned short T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned short T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned short T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned short T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned short T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC MAGIC_380                 /* Magic number for TMS380         */

#define LOCTOBYTE(x)   x                /* 16-bit byte addrs to byte addrs */
#define BYTETOLOC(x)   x                /* byte addrs to byte addrs        */
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif

#define LOADWORDSIZE  2                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    2                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    1                 /* SIZE IN BYTES OF INIT DATA ITEMS */
#endif

#if TMS370 || TMS370C8
typedef unsigned short T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned char  T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned short T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned short T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned char  T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC MAGIC_370                 /* 370 Magic Number                 */

#define LOCTOBYTE(x)   x                /* 370 addresses are same as bytes  */
#define BYTETOLOC(x)   x
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif

#define LOADWORDSIZE  2                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    1                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    1                 /* SIZE IN BYTES OF INIT DATA ITEMS */
#endif

#if MVP_PP
typedef unsigned long  T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned long  T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned long  T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned long  T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */

typedef struct {unsigned long ins_h; unsigned long ins_l; } T_INSTRUCT;      
					/* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC MAGIC_MVP                 /* PP  Magic Number                 */

#define LOCTOBYTE(x)   (x)              /* PP  addresses are same as bytes  */
#define BYTETOLOC(x)   (x)
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif

#define LOADWORDSIZE  8                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    8                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    4                 /* SIZE IN BYTES OF INIT DATA ITEMS */
#endif

#if MVP_MP
typedef unsigned long  T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned long  T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned long  T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned long  T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned long  T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define FT_IEEE_FLT		1
#define FT_IEEE_DBL		2
#define FLT_PRECISION		4

typedef struct trg_fval {
	unsigned long	fval1;
	unsigned long	fval2;
} TRG_FVAL;

#define MAGIC MAGIC_MVP                 /* MP  Magic Number                 */

#define LOCTOBYTE(x)   (x)              /* MP  addresses are same as bytes  */
#define BYTETOLOC(x)   (x)
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif

#define LOADWORDSIZE  8                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    8                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    4                 /* SIZE IN BYTES OF INIT DATA ITEMS */
#endif

#if LEAD
typedef unsigned short T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned short T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned short T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned short T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned short T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC MAGIC_LEAD                /* Magic number for C25            */

#define LOCTOBYTE(x)   ((x)<<1)         /* 16-bit word addrs to byte addrs */
#define BYTETOLOC(x)   ((x)>>1)         /* byte addrs to word addrs        */
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif /* ifdef OTIS */

#define LOADWORDSIZE  2                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    1                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    2                 /* SIZE IN BYTES OF INIT DATA ITEMS */

#endif /* LEAD */

#if ARM
typedef unsigned long T_ADDR;           /* TYPE FOR TARGET ADDRESS          */
typedef unsigned long T_DATA;           /* TYPE FOR TARGET DATA WORD        */
typedef unsigned long T_SIZE;           /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned long T_IADDR;          /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned long T_INSTRUCT;       /* TYPE FOR INSTRUCTION OPCODE      */

#define MAGIC MAGIC_ARM                 /* Magic number for ARM             */

#define LOCTOBYTE(x)   (x)              /* ARM addresses are same as bytes  */
#define BYTETOLOC(x)   (x)
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif

#define LOADWORDSIZE  4                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    4                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    4                 /* SIZE IN BYTES OF INIT DATA ITEMS */

#endif /* ARM */

#if TMS32060
typedef unsigned long  T_ADDR;          /* TYPE FOR TARGET ADDRESS          */
typedef unsigned long  T_DATA;          /* TYPE FOR TARGET DATA WORD        */
typedef unsigned long  T_SIZE;          /* TYPE FOR CINIT SIZE FIELD        */
typedef unsigned long  T_IADDR;         /* TYPE FOR CINIT ADDRESS FIELD     */
typedef unsigned long  T_INSTRUCT;      /* TYPE FOR INSTRUCTION OPCODE      */

#define FT_IEEE_FLT		1
#define FT_IEEE_DBL		2
#define FLT_PRECISION		4

typedef struct trg_fval {
	unsigned long	fval1;
	unsigned long	fval2;
} TRG_FVAL;

#define MAGIC MAGIC_C60                 /* C60 Magic Number                 */

#define LOCTOBYTE(x)   (x)              /* C60 addresses are same as bytes  */
#define BYTETOLOC(x)   (x)
#define BIT_OFFSET(a)  (0)              /* BIT OFFSET OF ADDR WITHIN BYTE   */

#ifdef OTIS
#define LOADBUFSIZE   (TRG_MAX_MBLK/8)  /* USE OTIS BUFFER SIZE             */
#else
#define LOADBUFSIZE   0x4000            /* 16K BUFFER FOR LOADING DATA      */
#endif

#define LOADWORDSIZE  8                 /* MINIMUM DIVISOR OF LOAD SIZE     */
#define CINIT         ".cinit"          /* NAME OF CINIT SECTION            */
#define INIT_ALIGN    8                 /* ALIGNMENT OF CINIT RECORDS       */
#define INIT_WSIZE    4                 /* SIZE IN BYTES OF INIT DATA ITEMS */
#endif

