/***************************************************************************
* FILENAME: coffdefs.h
* VERSION:  2.2  3/1/96  17:06:02
* SCCS ID:  "@(#)coffdefs.h	2.2  3/1/96"
***************************************************************************/
/**************************************************************************/
/*  COFFDEFS.H                                                            */
/*     Definitions of COFF symbol type and storage class fields.          */
/**************************************************************************/

/*------------------------------------------------------------------------*/
/*   STORAGE CLASSES                                                      */
/*------------------------------------------------------------------------*/
#define  C_NULL          0
#define  C_AUTO          1     /* AUTOMATIC VARIABLE                      */
#define  C_EXT           2     /* EXTERNAL SYMBOL                         */
#define  C_STAT          3     /* STATIC                                  */
#define  C_REG           4     /* REGISTER VARIABLE                       */
#define  C_EXTREF        5     /* EXTERNAL DEFINITION                     */
#define  C_LABEL         6     /* LABEL                                   */
#define  C_ULABEL        7     /* UNDEFINED LABEL                         */
#define  C_MOS           8     /* MEMBER OF STRUCTURE                     */
#define  C_ARG           9     /* FUNCTION ARGUMENT                       */
#define  C_STRTAG        10    /* STRUCTURE TAG                           */
#define  C_MOU           11    /* MEMBER OF UNION                         */
#define  C_UNTAG         12    /* UNION TAG                               */
#define  C_TPDEF         13    /* TYPE DEFINITION                         */
#define  C_USTATIC       14    /* UNDEFINED STATIC                        */
#define  C_ENTAG         15    /* ENUMERATION TAG                         */
#define  C_MOE           16    /* MEMBER OF ENUMERATION                   */
#define  C_REGPARM       17    /* REGISTER PARAMETER                      */
#define  C_FIELD         18    /* BIT FIELD                               */
#define  C_UEXT          19    /* TENTATIVE EXTERNAL DEFINITION           */
#define  C_STATLAB       20    /* STATIC LOAD-TIME LABEL                  */
#define  C_EXTLAB        21    /* EXTERNAL LOAD-TIME LABEL                */
#define  C_VREG          22    /* VIRTUAL REGISTER VARIABLE               */
#define  C_SYSTEM        23    /* SYSTEM-WIDE SYMBOL                      */
#define  C_STATREG       24    /* STATIC REGISTER VARIABLE                */
#define  C_EXTREG        25    /* EXTERNAL REGISTER VARIABLE              */
#define  C_EXTREFREG     26    /* EXTERNAL REGISTER VARIABLE REFERENCE    */
#define  C_VARARG        27    /* LAST DECLARED PARAMETER OF VARARG FN    */
#define  C_EXTDEF        28    /* C_EXT DEFINED IN DIFFERENT FILE         */
#define  C_USTATREG      29    /* UNDEFINED STATIC REGISTER VARIABLE      */

#define  C_BLOCK         100   /* ".BB" OR ".EB"                          */
#define  C_FCN           101   /* ".BF" OR ".EF"                          */
#define  C_EOS           102   /* END OF STRUCTURE                        */
#define  C_FILE          103   /* FILE NAME                               */
#define  C_LINE          104   /* DUMMY SCLASS FOR LINE NUMBER ENTRY      */
#define  C_ALIAS         105   /* DUPLICATE TAG                           */
#define  C_PREF          106   /* DUMMY SCLASS FOR REF PRAGMA TABLE ENTRY */

#define  C_GRPTAG        107   /* TAG FOR GROUPED GLOBAL VARIABLES        */
#define  C_SMOG          108   /* STATIC MEMBER OF GROUP                  */
#define  C_EMOG          109   /* EXTERN MEMBER OF GROUP                  */
#define  C_EOG           110   /* END OF GROUP                            */

/*------------------------------------------------------------------------*/
/* STORAGE CLASS QUALIFIERS                                               */
/*------------------------------------------------------------------------*/
#define  CQ_NEAR         0x1
#define  CQ_FAR          0x2
#define  CQ_INLINE       0x4
#define  CQ_SUPPRESS     0x8
#define  CQ_CONTROL      0x10
#define  CQ_INTERRUPT    0x20
#define  CQ_TRAP         0x40
#define  CQ_GREGISTER    0x80
#define  CQ_PORT         0x100
#define  CQ_SYSTEM       0x200
#define  CQ_SYSTEMPP     0x400
#define  CQ_REENTRANT    0x800

/*------------------------------------------------------------------------*/
/* STORAGE CLASS MACROS                                                   */
/*------------------------------------------------------------------------*/
#define ISLOCAL(c) ((c) == C_AUTO   || (c) == C_REG     || (c) == C_VREG)
#define ISPARM(c)  ((c) == C_ARG    || (c) == C_REGPARM || (c) == C_VARARG)
#define ISAUTO(c)  ((c) == C_AUTO   || (c) == C_ARG     || (c) == C_VARARG)
#define ISREG(c)   ((c) == C_REG    || (c) == C_REGPARM || (c) == C_VREG || \
                    (c) == C_EXTREG || (c) == C_STATREG || (c) == C_EXTREFREG)
#define ISTAG(c)   ((c) == C_STRTAG || (c) == C_UNTAG   || (c) == C_ENTAG)
#define ISGROUP(c) ((c) == C_GRPTAG)

#define ISMOS(c)   ((c) == C_MOS    || (c) == C_MOU     || \
		    (c) == C_MOE    || (c) == C_FIELD)

#define ISXDEF(c)  ((c) == C_STAT   || (c) == C_STATREG || \
                    (c) == C_EXT    || (c) == C_EXTDEF  || (c) == C_EXTREG)

#define ISEXT(c)   ((c) == C_USTATIC || (c) == C_STAT   || (c) == C_STATREG  ||\
                    (c) == C_EXTREF  || (c) == C_UEXT   || (c) == C_EXTREFREG||\
                    (c) == C_EXT     || (c) == C_EXTDEF || (c) == C_EXTREG   ||\
		    (c) == C_EXTLAB  || (c) == C_SYSTEM)

#define ISGLOB(c)  ((c) == C_EXTREF  || (c) == C_UEXT   || (c) == C_EXTREFREG||\
                    (c) == C_EXT     || (c) == C_EXTDEF || (c) == C_EXTREG   ||\
		    (c) == C_EXTLAB  || (c) == C_SYSTEM)

#define ISNEAR(cq)        (((cq) & CQ_NEAR) != 0)
#define ISFAR(cq)         (((cq) & CQ_FAR) != 0)
#define ISCONTROL(cq)     (((cq) & CQ_CONTROL) != 0)
#define ISGREGISTER(cq)   (((cq) & CQ_GREGISTER) != 0)
#define ISPORT(cq)        (((cq) & CQ_PORT)    != 0)
#define ISINTERRUPT(cq)   (((cq) & CQ_INTERRUPT) != 0)
#define ISREENTRANT(cq)   (((cq) & CQ_REENTRANT) != 0)
#define ISTRAP(cq)        (((cq) & CQ_TRAP) != 0)
#define ISINT_OR_TRAP(cq) (((cq) & (CQ_TRAP | CQ_INTERRUPT)) != 0)



/*------------------------------------------------------------------------*/
/* BASIC TYPES - PACKED INTO THE LOWER 4 BITS OF THE TYPE FIELD           */
/*------------------------------------------------------------------------*/
#define  T_NULL     0x80       /* UNDEFINED OR ERROR TYPE (NO TYPE INFO)  */
#define  T_VOID     0          /* VOID TYPE                               */
#define  T_SCHAR    1          /* CHARACTER (EXPLICITLY "signed")         */
#define  T_CHAR     2          /* CHARACTER (IMPLICITLY SIGNED)           */
#define  T_SHORT    3          /* SHORT INTEGER                           */
#define  T_INT      4          /* INTEGER                                 */
#define  T_LONG     5          /* LONG INTEGER                            */
#define  T_FLOAT    6          /* SINGLE PRECISION FLOATING POINT         */
#define  T_DOUBLE   7          /* DOUBLE PRECISION FLOATING POINT         */
#define  T_STRUCT   8          /* STRUCTURE                               */
#define  T_UNION    9          /* UNION                                   */
#define  T_ENUM     10         /* ENUMERATION                             */
#define  T_LDOUBLE  11         /* LONG DOUBLE FLOATING POINT              */
#define  T_UCHAR    12         /* UNSIGNED CHARACTER                      */
#define  T_USHORT   13         /* UNSIGNED SHORT                          */
#define  T_UINT     14         /* UNSIGNED INTEGER                        */
#define  T_ULONG    15         /* UNSIGNED LONG                           */

/*------------------------------------------------------------------------*/
/* SIGNED AND UNSIGNED ARE NOT ACTUALLY STORED IN THE TYPE FIELD BUT      */
/* USED TO MODIFY THE BYTPE ACCORDINGLY.                                  */
/*------------------------------------------------------------------------*/
#define  T_SIGNED   16         /* BECOMES CHAR, SHORT, INT, OR LONG       */
#define  T_UNSIGNED 17         /* BECOMES UCHAR, USHORT, UINT, OR ULONG   */

/*------------------------------------------------------------------------*/
/* DERIVED TYPES: 2 BITS EACH                                             */
/*------------------------------------------------------------------------*/
#define  DT_NON     0x0        /* NO DERIVED TYPE                         */
#define  DT_PTR     0x1        /* POINTER                                 */
#define  DT_FCN     0x2        /* FUNCTION                                */
#define  DT_ARY     0x3        /* ARRAY                                   */

/*------------------------------------------------------------------------*/
/* TYPE QUALIFIERS - USES SAME ENCODING AS TYPE FIELDS                    */
/*------------------------------------------------------------------------*/
#define  Q_CONST    0x1
#define  Q_VOLATILE 0x2


/*------------------------------------------------------------------------*/
/* TYPE FIELD MASKS AND SIZES                                             */
/*------------------------------------------------------------------------*/
#define  N_BTMASK     0xF      /* MASK FOR BASIC TYPE                     */
#define  N_TMASK      0x30     /* MASK FOR FIRST DERIVED TYPE             */    
#define  N_DTMAX      12       /* MAXIMUM DERIVED TYPES                   */
#define  N_BTSHFT     4        /* SHIFT AMOUNT (WIDTH) FOR BASIC TYPE     */
#define  N_TSHIFT     2        /* SHIFT AMOUNT (WIDTH) FOR DERIVED TYPES  */

/*------------------------------------------------------------------------*/
/* TYPE MANIPULATION MACROS                                               */
/*                                                                        */
/*    BTYPE(t)    - Return basic type from t                              */
/*    DTYPE(t)    - Return all derived types from t                       */
/*    DTYPE1(t)   - Return 1st derived type from t                        */
/*    TQUAL(t,q)  - Return qualification of type                          */
/*    UNQUAL(t,q) - Return unqualified version of type                    */
/*                                                                        */
/*    MKTYPE()    - Build a type from basic and several derived types     */
/*    DERIVE(d,t) - Build a type from basic and one derived type          */
/*    INCREF(t)   - Convert 't' into pointer to 't'                       */
/*    DECREF(t)   - Remove first derviation from t                        */
/*                                                                        */
/*    ISINT(t)    - TRUE if t is an integral type                         */
/*    ISSGN(t)    - TRUE if t is a signed type                            */
/*    ISUNS(t)    - TRUE if t is an unsigned type                         */
/*    ISFLT(t)    - TRUE if t is a floating point type                    */
/*    ISDBL(t)    - TRUE if t is a double or long double type             */
/*    ISPTR(t)    - TRUE if t is a pointer                                */
/*    ISFCN(t)    - TRUE if t is a function                               */
/*    ISARY(t)    - TRUE if t is an array                                 */
/*    ISSTR(t)    - TRUE if t is a struct, union, or enum type            */
/*    ISAGG(t)    - TRUE if t is an array, struct, or union               */
/*                                                                        */
/*    ITOU(t)     - convert signed type to unsigned equivalent            */
/*    UTOI(t)     - convert unsigned type to signed equivalent            */
/*    NOSIGN(t)   - convert signed or unsigned type to "plain" equivalent */
/*------------------------------------------------------------------------*/
#define  BTYPE(t)    ((int)((t) & N_BTMASK))  
#define  DTYPE(t)    ((t) & ~N_BTMASK)
#define  DTYPE1(t)   (((t) & N_TMASK) >> N_BTSHFT)

#define  TQUAL(t,q)  ((int)(DTYPE(t) ? DTYPE1(q) : BTYPE(q)))
#define  UNQUAL(t,q) (ISPTR(t) ? ((q) & ~N_TMASK) : ((q) & ~N_BTMASK))

#define  MKTYPE(basic, d1,d2,d3,d4,d5,d6) \
                     ((basic) | ((d1) <<  4) | ((d2) <<  6) | ((d3) <<  8) |\
                                ((d4) << 10) | ((d5) << 12) | ((d6) << 14))
#define  DERIVE(d,t) ((DTYPE(t) << N_TSHIFT) | ((d)    << N_BTSHFT) | BTYPE(t))
#define  INCREF(t)   ((DTYPE(t) << N_TSHIFT) | (DT_PTR << N_BTSHFT) | BTYPE(t))
#define  DECREF(t)   (DTYPE((t) >> N_TSHIFT) | BTYPE(t))

#define  ISSGN(t)    (((t) >= T_SCHAR && (t) <= T_LONG)  || (t) == T_ENUM)
#define  ISUNS(t)    ((t) >= T_UCHAR  && (t) <= T_ULONG)
#define  ISINT(t)    (((t) >= T_SCHAR && (t) <= T_LONG)  || (t) == T_ENUM || \
                      ((t) >= T_UCHAR  && (t) <= T_ULONG))
#define  ISFLT(t)    ((t) == T_FLOAT || (t) == T_DOUBLE || (t) == T_LDOUBLE)
#define	 ISDBL(t)    ((t) == T_DOUBLE || (t) == T_LDOUBLE)
#define  ISPTR(t)    (((t) & N_TMASK) == (DT_PTR << N_BTSHFT)) 
#define  ISFCN(t)    (((t) & N_TMASK) == (DT_FCN << N_BTSHFT))
#define  ISARY(t)    (((t) & N_TMASK) == (DT_ARY << N_BTSHFT))
#define  ISSTR(t)    ((t) == T_STRUCT || (t) == T_UNION || (t) == T_ENUM)
#define  ISAGG(t)    (ISARY(t) || (t) == T_STRUCT || (t) == T_UNION)
#define  ISCHAR(t)   ((t) == T_CHAR || (t) == T_SCHAR || (t) == T_UCHAR)
#define  ISSHORT(t)  ((t) == T_SHORT || (t) == T_USHORT)
#define  ISLONG(t)   ((t) == T_LONG || (t) == T_ULONG)

#define  ITOU(t)     ((t) + (T_UCHAR - ((t) == T_SCHAR ? T_SCHAR : T_CHAR)))
#define  UTOI(t)     ((t) - (T_UCHAR - T_CHAR))
#define  NOSIGN(t)   (ISUNS(t) ? UTOI(t) : (t) == T_SCHAR ? T_CHAR : (t))

/*------------------------------------------------------------------------*/
/* ILLEGAL TYPES USED TO MARK SPECIAL OBJECTS.                            */
/*                                                                        */
/* T_VENEER  - DENOTES A VENEER OF A FUNCTION.                            */
/*------------------------------------------------------------------------*/
#define T_VENEER     (MKTYPE(T_VOID, DT_FCN, DT_FCN, 0, 0, 0, 0))
#define ISVENEER(t)  ((t) == T_VENEER)
