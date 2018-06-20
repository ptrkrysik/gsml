/*
 * The foo struct is the interface to PCI as defined by sysFooPci.c
 */

struct foo
{
    int pciBus;                         /* the pci bus number              */
    int pciDevice;                      /* the pci device number           */
    int pciFunc;                        /* the pci function number         */
    char irq;                           /* the pci irq                     */
    FUNCPTR intRoutine;                 /* interrupt routine               */
    struct
    {
        UINT32 pciBAR;                  /* pci base address register       */
        UINT32 pciBARSize;              /* pci bar address size            */
    } bar[5];
};
