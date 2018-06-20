/******************************************************************************/
/*  TIMER.H - TMS320C6x Peripheral Support Library Timers Support             */
/*                                                                            */
/*     This file provides the header for the DSP's timers support.            */
/*                                                                            */
/*                                                                            */
/*  MACRO FUNCTIONS:                                                          */
/*    TIMER_CTRL_ADDR(chan)                                                   */
/*    TIMER_PERIOD_ADDR(chan)                                                 */
/*    TIMER_COUNTER_ADDR(chan)                                                */
/*    TIMER_RESET(chan)                                                       */
/*    TIMER_INIT(chan,ctrl,per,cnt)                                           */
/*    TIMER_START(chan)                                                       */
/*    TIMER_STOP(chan)                                                        */
/*    TIMER_RESUME(chan)                                                      */
/*    TIMER_MODE_SELECT(chan,mode)                                            */
/*    TIMER_CLK_INTERNAL(chan)                                                */
/*    TIMER_CLK_EXTERNAL(chan)                                                */
/*    TOUT_ENABLE(chan)                                                       */
/*    TOUT_DISABLE(chan)                                                      */
/*    TOUT_VAL(chan,val)                                                      */
/*    TOUT_ASSERT(chan)                                                       */
/*    TOUT_NEGATE(chan)                                                       */
/*    TINP_GET(chan)                                                          */
/*    TIMER_READ(chan)                                                        */
/*    TIMER_GET_COUNT(chan)                                                   */
/*    TIMER_SET_COUNT(chan,val)                                               */
/*    TIMER_AVAILABLE(chan)                                                   */
/*    TIMER_SET_PERIOD(chan,val)                                              */
/*    TIMER_GET_PERIOD(chan)                                                  */
/*    TIMER_GET_TSTAT(chan)                                                   */
/*                                                                            */
/*                                                                            */
/*  FUNCTIONS:                                                                */
/*    timer_delay() - delay specified number of timer periods                 */
/*                                                                            */
/******************************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_

/*----------------------------------------------------------------------------*/
/* INCLUDES                                                                   */
/*----------------------------------------------------------------------------*/
#include "regs.h"

/*----------------------------------------------------------------------------*/
/* DEFINES AND MACROS                                                         */
/*----------------------------------------------------------------------------*/

/******************************************************************************/
/**************************** TIMER REGISTERS *********************************/

#define TIMER_BASE_ADDR 0x01940000

#define TIMER_CTRL_ADDR(chan) \
        (TIMER_BASE_ADDR + ((chan) * 0x40000))

#define TIMER_PERIOD_ADDR(chan) \
        (TIMER_BASE_ADDR + ((chan) * 0x40000) + 4)

#define TIMER_COUNTER_ADDR(chan) \
        (TIMER_BASE_ADDR + ((chan) * 0x40000) + 8)

#define TIMER0_CTRL_ADDR      TIMER_CTRL_ADDR(0)
#define TIMER0_PERIOD_ADDR    TIMER_PERIOD_ADDR(0)
#define TIMER0_COUNTER_ADDR   TIMER_COUNTER_ADDR(0)

#define TIMER1_CTRL_ADDR      TIMER_CTRL_ADDR(1)
#define TIMER1_PERIOD_ADDR    TIMER_PERIOD_ADDR(1)
#define TIMER1_COUNTER_ADDR   TIMER_COUNTER_ADDR(1)

#define TIMER0_CTRL     *(volatile unsigned int *)(TIMER0_CTRL_ADDR)
#define TIMER0_PERIOD   *(volatile unsigned int *)(TIMER0_PERIOD_ADDR)
#define TIMER0_COUNTER  *(volatile unsigned int *)(TIMER0_COUNTER_ADDR)

#define TIMER1_CTRL     *(volatile unsigned int *)(TIMER1_CTRL_ADDR)
#define TIMER1_PERIOD   *(volatile unsigned int *)(TIMER1_PERIOD_ADDR)
#define TIMER1_COUNTER  *(volatile unsigned int *)(TIMER1_COUNTER_ADDR)

/* Timer Control Register Bitfield */
#define FUNC     0
#define INVOUT   1
#define DATOUT   2
#define DATIN    3
#define PWID     4
#define GO       6
#define HLD      7
#define C_P      8
#define CLKSRC   9
#define INVINP   10
#define TSTAT    11

#define TIMER_PULSE_MODE     0
#define TIMER_CLOCK_MODE     1

/*---------------------------------------------------------------------------*/
/* TIMER_RESET - reset timer to conditions defined by device reset           */
/*---------------------------------------------------------------------------*/
#define TIMER_RESET(chan) \
        { (*(unsigned int *)(TIMER_CTRL_ADDR(chan)) = 0);       \
          (*(unsigned int *)(TIMER_PERIOD_ADDR(chan)) = 0);     \
          (*(unsigned int *)(TIMER_COUNTER_ADDR(chan)) = 0);    \
        }

/*---------------------------------------------------------------------------*/
/* TIMER_INIT - initialize timer registers                                   */
/*---------------------------------------------------------------------------*/
#define TIMER_INIT(chan,ctrl,per,cnt) \
        { (*(unsigned int *)(TIMER_CTRL_ADDR(chan))    = ctrl); \
          (*(unsigned int *)(TIMER_PERIOD_ADDR(chan))  = per);  \
          (*(unsigned int *)(TIMER_COUNTER_ADDR(chan)) = cnt);  \
        }

/*---------------------------------------------------------------------------*/
/* TIMER_START - Sets both GO and HOLD bits in Timer Control Register which  */
/*               resets the Timer Counter Register and enables counting on   */
/*               on the next clock.  (GO bit autoclears)                     */
/*---------------------------------------------------------------------------*/
#define TIMER_START(chan) \
        REG_WRITE(TIMER_CTRL_ADDR(chan),(REG_READ(TIMER_CTRL_ADDR(chan)) | 0xc0))

/*---------------------------------------------------------------------------*/
/* TIMER_STOP - Asserts (clears) the HOLD bit in the Timer Control Register  */
/*---------------------------------------------------------------------------*/
#define TIMER_STOP(chan) \
        RESET_BIT(TIMER_CTRL_ADDR(chan),HLD)

/*---------------------------------------------------------------------------*/
/* TIMER_RESUME - Negates (sets) the HOLD bit to resume counting without     */
/*                resetting the counter register                             */
/*---------------------------------------------------------------------------*/
#define TIMER_RESUME(chan) \
        SET_BIT(TIMER_CTRL_ADDR(chan),HLD)

/*---------------------------------------------------------------------------*/
/* TIMER_MODE_SELECT - selects between PULSE and CLOCK modes                 */
/*---------------------------------------------------------------------------*/
#define TIMER_MODE_SELECT(chan,mode) \
        (mode == TIMER_CLOCK_MODE) ? SET_BIT(TIMER_CTRL_ADDR(chan),C_P) \
        : RESET_BIT(TIMER_CTRL_ADDR(chan),C_P)

/*---------------------------------------------------------------------------*/
/* TIMER_CLK_INTERNAL - sets CLKSRC to select CPU clock/4 as timer clock     */
/*---------------------------------------------------------------------------*/
#define TIMER_CLK_INTERNAL(chan) \
        SET_BIT(TIMER_CTRL_ADDR(chan),CLKSRC)

/*---------------------------------------------------------------------------*/
/* TIMER_CLK_EXTERNAL - clears CLKSRC to select TINP as timer clock          */
/*---------------------------------------------------------------------------*/
#define TIMER_CLK_EXTERNAL(chan) \
        RESET_BIT(TIMER_CTRL_ADDR(chan),CLKSRC)

/*---------------------------------------------------------------------------*/
/* TOUT_ENABLE - configures TOUT as general purpose output pin.              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#define TOUT_ENABLE(chan) \
        RESET_BIT(TIMER_CTRL_ADDR(chan),FUNC)

/*---------------------------------------------------------------------------*/
/* TOUT_DISABLE - configures TOUT as a timer pin; reflects value of TSTAT    */
/*                conditioned by INVOUT                                      */
/*---------------------------------------------------------------------------*/
#define TOUT_DISABLE(chan) \
        SET_BIT(TIMER_CTRL_ADDR(chan),FUNC)

/*---------------------------------------------------------------------------*/
/* TOUT_VAL - assigns val to TOUT pin when TOUT is enabled as general purpose*/
/*            output                                                         */
/*---------------------------------------------------------------------------*/
#define TOUT_VAL(chan,val) \
        ASSIGN_BIT_VAL(TIMER_CTRL_ADDR(chan),DATOUT,val)

/*---------------------------------------------------------------------------*/
/* TOUT_ASSERT - assigns 1 to TOUT pin when TOUT is enabled as general       */
/*               purpose output                                              */
/*---------------------------------------------------------------------------*/
#define TOUT_ASSERT(chan) \
        SET_BIT(TIMER_CTRL_ADDR(chan),DATOUT)

/*---------------------------------------------------------------------------*/
/* TOUT_NEGATE - assigns 0 to TOUT pin when TOUT is enabled as general       */
/*            purpose output                                                 */
/*---------------------------------------------------------------------------*/
#define TOUT_NEGATE(chan) \
        RESET_BIT(TIMER_CTRL_ADDR(chan),DATOUT)

/*---------------------------------------------------------------------------*/
/* TINP_GET - returns value on TINP input pin                                */
/*---------------------------------------------------------------------------*/
#define TINP_GET(chan) \
        GET_BIT(TIMER_CTRL_ADDR(chan),DATIN)

/*---------------------------------------------------------------------------*/
/* TIMER_READ - reads value of Timer Counter Register                      */
/*---------------------------------------------------------------------------*/
#define TIMER_READ(chan) \
        (REG_READ(TIMER_COUNTER_ADDR(chan)))

/*---------------------------------------------------------------------------*/
/* TIMER_GET_COUNT - reads value of Timer Counter Register                   */
/*---------------------------------------------------------------------------*/
#define TIMER_GET_COUNT(chan) \
        (REG_READ(TIMER_COUNTER_ADDR(chan)))

/*---------------------------------------------------------------------------*/
/* TIMER_SET_COUNT - reads value of Timer Counter Register                   */
/*---------------------------------------------------------------------------*/
#define TIMER_SET_COUNT(chan,val) \
        (REG_WRITE(TIMER_COUNTER_ADDR(chan),val))

/*---------------------------------------------------------------------------*/
/* TIMER_AVAILABLE - checks timer for availability; returns TRUE or FALSE    */
/*---------------------------------------------------------------------------*/
#define TIMER_AVAILABLE(chan) \
        (GET_BIT(TIMER_CTRL_ADDR(chan),HLD) ? 0 : 1)

/*---------------------------------------------------------------------------*/
/* TIMER_SET_PERIOD - sets value of Timer Period Register                    */
/*---------------------------------------------------------------------------*/
#define TIMER_SET_PERIOD(chan,val) \
        (REG_WRITE(TIMER_PERIOD_ADDR(chan),val))

/*---------------------------------------------------------------------------*/
/* TIMER_GET_PERIOD - returns value of Timer Period Register                 */
/*---------------------------------------------------------------------------*/
#define TIMER_GET_PERIOD(chan) \
        (REG_READ(TIMER_PERIOD_ADDR(chan)))

/*---------------------------------------------------------------------------*/
/* TIMER_GET_TSTAT - returns value of TSTAT bit in Timer Ctrl Register       */
/*---------------------------------------------------------------------------*/
#define TIMER_GET_TSTAT(chan) \
        (GET_BIT(TIMER_CTRL_ADDR(chan),TSTAT))

/*----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                  */
/*----------------------------------------------------------------------------*/

int timer_delay(short num_timer_periods);

#endif /* ifndef _TIMER_H_ */
