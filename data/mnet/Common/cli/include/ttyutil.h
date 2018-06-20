#ifndef __TTYUTIL_H__
#define __TTYUTIL_H__



/* Special Characters Mapping */
#define CH_NULL        0x00
#define CH_SOH         0x01
#define CH_STX         0x02
#define CH_ETX         0x03
#define CH_EOT         0x04
#define CH_ENQ         0x05
#define CH_ACK         0x06
#define CH_BEL         0x07        /* the bell character */
#define CH_BS          0x08
#define CH_HT          0x09
#define CH_NL          0x0A
#define CH_VT          0x0B       
#define CH_NP          0x0C
#define CH_CR          0x0D
#define CH_SO          0x0E
#define CH_SI          0x0F
#define CH_DLE         0x10
#define CH_DC1         0x11
#define CH_DC2         0x12
#define CH_DC3         0x13
#define CH_DC4         0x14
#define CH_NAK         0x15
#define CH_SYN         0x16
#define CH_ETB         0x17
#define CH_CAN         0x18
#define CH_EM          0x19
#define CH_SUB         0x1A
#define CH_ESC         0x1B
#define CH_FS          0x1C
#define CH_GS          0x1D
#define CH_RS          0x1E
#define CH_US          0x1F
#define CH_SPC         0x20
#define CH_DEL         0x7F

#define CH_LT_SQ_BRACE 0x5B

#define CH_EXT         0x80

#define CH_CTRL_B      0x02
#define CH_CTRL_C      0x03
#define CH_CTRL_F      0x06
#define CH_CTRL_N      0x0E
#define CH_CTRL_P      0x10
#define CH_CTRL_Z      0x1A

typedef enum { 
        PRIMARY_TASK = 1,
        SECONDARY_TASK
} tTTYATTR;


/*
 * Primary/Secondary task abstraction
 * for hierarchical distribution of signals.
 */
extern STATUS ttySignalSecondaryTasks(int fd, int signo);
extern STATUS taskSetTtyAttr(int fd, tTTYATTR type, BOOL value);
extern BOOL taskGetTtyAttr(int fd, tTTYATTR type);

/*
 * Manage list of control characters 
 */
extern STATUS addSignalChar (int ch, int signo);
extern STATUS removeSignalChar (int signo);

/*
 * Provides maintenance of per-task signal mask
 */
extern void   defaultSignalHandler(int signo);

extern STATUS setTaskSignalFlag (int tid, int signo, BOOL enable);
extern BOOL isTaskSignalSet (int tid, int signo);
extern uint32_t getTaskSignalMask (int tid);
extern STATUS setTaskSignalMask (int tid, uint32_t sigMask);

/*
 * Quick check to see if a signal has been delivered to current task
 */
extern BOOL tty_signal(void);

#endif
