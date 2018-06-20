/******************************************************************************/
/*  INTR.H - TMS320C6x Peripheral Support Library Interrupt Support           */
/*                                                                            */
/*     This file provides the header for the DSP's interrupt support.         */
/*                                                                            */
/*  MACRO FUNCTIONS:                                                          */
/*     INTR_GLOBAL_ENABLE() - Enable global interrupts (GIE)                  */
/*     INTR_GLOBAL_DISABLE()- Disable global interrupts (GIE)                 */
/*     INTR_ENABLE()        - Enable interrupt (set bit in IER)               */
/*     INTR_DISABLE()       - Disable interrupt (clear bit in IER)            */
/*     INTR_CHECK_FLAG()    - Check interrupt bit in IFR                      */
/*     INTR_SET_FLAG()      - Set interrupt by writing to ISR bit             */
/*     INTR_CLR_FLAG()      - Clear interrupt by writing to ICR bit           */
/*     INTR_SET_MAP()       - Map CPU interrupt to interrupt selector         */
/*     INTR_GET_ISN()       - Get ISN of selected interrupt                   */
/*     INTR_MAP_RESET()     - Reset interrupt multiplexor map to defaults     */
/*     INTR_EXT_POLARITY()  - Assign external interrupt's polarity            */
/*                                                                            */
/*  FUNCTIONS:                                                                */
/*     intr_reset()         - Reset interrupt registers to default values     */
/*     intr_init()          - Interrupt initialization                        */
/*     intr_isn()           - Assign ISN to CPU interrupt                     */
/*     intr_get_cpu_intr()  - Return CPU interrupt assigned to ISN            */
/*     intr_map()           - Place ISN in interrupt multiplexor register     */
/*                                                                            */
/******************************************************************************/
#ifndef _INTR_H_
#define _INTR_H_

#ifdef _INLINE
#define __INLINE static inline
#else
#define __INLINE
#endif

/*----------------------------------------------------------------------------*/
/* INCLUDES                                                                   */
/*----------------------------------------------------------------------------*/
#include "regs.h"

/*----------------------------------------------------------------------------*/
/* DEFINES AND MACROS                                                         */
/*----------------------------------------------------------------------------*/

/******************************************************************************/
/********************** INTERRUPT SELECTOR REGISTERS **************************/
#define INTR_MULTIPLEX_HIGH_ADDR     0x019C0000
#define INTR_MULTIPLEX_LOW_ADDR      0x019C0004
#define EXTERNAL_INTR_POL_ADDR       0x019C0008

#define INTSEL4             0
#define INTSEL_SZ           4
#define INTSEL5             5
#define INTSEL6            10
#define INTSEL7            16
#define INTSEL8            21
#define INTSEL9            26
#define INTSEL10            0
#define INTSEL11            5
#define INTSEL12           10
#define INTSEL13           16
#define INTSEL14           21
#define INTSEL15           26

/* External Interrupt Polarity Register                                       */

#define XIP4                0
#define XIP5                1
#define XIP6                2
#define XIP7                3

/* CPU Interrupt Numbers                                                      */

#define CPU_INT_RST          0x00
#define CPU_INT_NMI          0x01
#define CPU_INT_RSV1         0x02
#define CPU_INT_RSV2         0x03
#define CPU_INT4             0x04
#define CPU_INT5             0x05
#define CPU_INT6             0x06
#define CPU_INT7             0x07
#define CPU_INT8             0x08
#define CPU_INT9             0x09
#define CPU_INT10            0x0A
#define CPU_INT11            0x0B
#define CPU_INT12            0x0C
#define CPU_INT13            0x0D
#define CPU_INT14            0x0E
#define CPU_INT15            0x0F

/* Interrupt Selection Numbers                                                */

#define ISN_DSPINT           0x00
#define ISN_TINT0            0x01
#define ISN_TINT1            0x02
#define ISN_SD_INT           0x03      
#define ISN_EXT_INT4         0x04
#define ISN_EXT_INT5         0x05
#define ISN_EXT_INT6         0x06
#define ISN_EXT_INT7         0x07
#define ISN_DMA_INT0         0x08
#define ISN_DMA_INT1         0x09
#define ISN_DMA_INT2         0x0A
#define ISN_DMA_INT3         0x0B
#define ISN_XINT0            0x0C
#define ISN_RINT0            0x0D
#define ISN_XINT1            0x0E
#define ISN_RINT1            0x0F

#define IML_SEL              0x00       /* Interrupt Multiplexor Low Select   */
#define IMH_SEL              0x01       /* Interrupt Multiplexor High Select  */
#define IML_RESET_VAL        0x250718A4
#define IMH_RESET_VAL        0x08202D4B

/*----------------------------------------------------------------------------*/
/* MACRO FUNCTIONS                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* INTR_GLOBAL_ENABLE - enables all masked interrupts by setting GIE in CSR   */
/*----------------------------------------------------------------------------*/
#define INTR_GLOBAL_ENABLE() \
        SET_REG_BIT(CSR, GIE)

/*----------------------------------------------------------------------------*/
/* INTR_GLOBAL_DISABLE - disables all masked interrupts by clearing GIE in CSR*/
/*----------------------------------------------------------------------------*/
#define INTR_GLOBAL_DISABLE() \
        RESET_REG_BIT(CSR, GIE)

/*----------------------------------------------------------------------------*/
/* INTR_GLOBAL_CHECK - checks global interrupt state by returning GIE in CSR  */
/*----------------------------------------------------------------------------*/
#define INTR_GLOBAL_CHECK() \
        (CSR & MASK_BIT(GIE) ? 1 : 0)

/*----------------------------------------------------------------------------*/
/* INTR_ENABLE - enable interrupt by setting flag in IER                      */
/*----------------------------------------------------------------------------*/
#define INTR_ENABLE(bit) \
        SET_REG_BIT(IER,bit) 

/*----------------------------------------------------------------------------*/
/* INTR_DISABLE - disable interrupt by clearing flag in IER                   */
/*----------------------------------------------------------------------------*/
#define INTR_DISABLE(bit) \
        RESET_REG_BIT(IER,bit) 

/*----------------------------------------------------------------------------*/
/* INTR_CHECK_FLAG - checks status of indicated interrupt bit in IFR          */
/*----------------------------------------------------------------------------*/
#define INTR_CHECK_FLAG(bit) \
        (IFR & MASK_BIT(bit) ? 1 : 0)

/*----------------------------------------------------------------------------*/
/* INTR_SET_FLAG - manually sets indicated interrupt by writing to ISR        */
/*----------------------------------------------------------------------------*/
#define INTR_SET_FLAG(bit) \
        (ISR |= MASK_BIT(bit))

/*----------------------------------------------------------------------------*/
/* INTR_CLR_FLAG - manually clears indicated interrupt by writing 1 to ICR    */
/*----------------------------------------------------------------------------*/
#define INTR_CLR_FLAG(bit) \
        (ICR |= MASK_BIT(bit))

/*----------------------------------------------------------------------------*/
/* INTR_SET_MAP  - maps a CPU interrupt specified by intr to the interrupt src*/
/*            specified by val.  Sel is used to select between the low and    */
/*            high interrupt_multiplexor registers.                           */
/*----------------------------------------------------------------------------*/
#define INTR_SET_MAP(intsel,val,sel) \
        (sel ? LOAD_FIELD(INTR_MULTIPLEX_HIGH_ADDR,val,intsel,INTSEL_SZ) : \
               LOAD_FIELD(INTR_MULTIPLEX_LOW_ADDR, val,intsel,INTSEL_SZ ))

/*----------------------------------------------------------------------------*/
/* INTR_GET_ISN - returns the ISN value in the selected Interrupt Multiplexor */
/*                register for the interrupt selected by intsel               */
/*----------------------------------------------------------------------------*/
#define INTR_GET_ISN(intsel,sel) \
        (sel ? GET_FIELD(INTR_MULTIPLEX_HIGH_ADDR,intsel,INTSEL_SZ) : \
               GET_FIELD(INTR_MULTIPLEX_LOW_ADDR, intsel, INTSEL_SZ))

/*----------------------------------------------------------------------------*/
/* INTR_MAP_RESET - resets the interrupt multiplexor maps to their default val*/
/*----------------------------------------------------------------------------*/
#define INTR_MAP_RESET() \
        {CONTENTS_OF(INTR_MULTIPLEX_HIGH_ADDR) = IMH_RESET_VAL; \
         CONTENTS_OF(INTR_MULTIPLEX_LOW_ADDR)  = IML_RESET_VAL; }

/*----------------------------------------------------------------------------*/
/* INTR_EXT_POLARITY - assigns external interrupt external priority.          */
/*                    val = 0 (normal), val = 1 (inverted)                    */
/*----------------------------------------------------------------------------*/
#define INTR_EXT_POLARITY(bit,val) \
        (val ? SET_BIT(EXTERNAL_INTR_POL_ADDR,bit) : \
               CLEAR_BIT(EXTERNAL_INTR_POL_ADDR,bit))


/*----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                           */
/*----------------------------------------------------------------------------*/
extern unsigned int istb;

/*----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                  */
/*----------------------------------------------------------------------------*/
extern void interrupt c_int00(void);

void intr_reset(void);
void intr_init(void);
void intr_hook(void (*fp)(void),int intr_num);

__INLINE void intr_map(int cpu_intr,int isn);
__INLINE int intr_isn(int cpu_intr);
__INLINE int intr_get_cpu_intr(int isn);


#ifdef _INLINE
/* intr_map() - Place isn value in Interrupt Multiplexer Register in INTSEL   */
/*              field indicated by cpu_intr.                                  */
                                          
static inline void intr_map(int cpu_intr,int isn)
{
  int intsel;
  int sel;

  if (cpu_intr > CPU_INT9)
    sel=1;
  else
    sel= 0;

  intsel= ((cpu_intr - CPU_INT4) * INTSEL_SZ) - (sel * 30);
  if (intsel > INTSEL6)
    intsel++;

  INTR_SET_MAP(intsel,isn,sel);
}


/* intr_isn() - return isn in interrupt selector corresponding to cpu_intr    */
static inline int intr_isn(int cpu_intr)
{

  int intsel;
  int sel;

  if (cpu_intr > CPU_INT9)
    sel= 1;
  else
    sel= 0;

  intsel= ((cpu_intr - CPU_INT4) * INTSEL_SZ) - (sel * 30);
  if (intsel > INTSEL6)
    intsel++;

  return(INTR_GET_ISN(intsel,sel));
}

/* intr_get_cpu_intr() - return cpu interrupt corresponding to isn in         */
/*                       interrupt selecter register.  If the isn is not      */
/*                       mapped, return -1                                    */
static inline int intr_get_cpu_intr(int isn)
{
  int i;
  for (i= CPU_INT4;i<=CPU_INT15;i++)
  {
    if (intr_get_isn(i) == isn)
      return(i);
  }
  return(-1);
}

#endif /* _INLINE */

#undef __INLINE    
#endif /* _INTR_H_ */
