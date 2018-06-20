/*
JetCell modification history
----------------------------
99/06/01  ck   Initial Version
*/

/*
 * The routines in this file are designed to support the DS2401 Silicon
 * Serial Number part. The DS2401 is a single line part returning 64-bits or 8 
 * bytes of data. The single line is driven and read on an 860 PORTB bit,
 * DS2401BIT. The external calling routine is readDs2401, which returns the
 * data in the global 8 byte buffer ds2401. The buffer will return with an
 * 8 bit Family Code (01h), 48 bit serial number and an 8 bit CRC. Upon
 * completion, the global variable ds2401CRC, which contains the calculated CRC
 * should return zero. The programs in this file use timing loops rather than
 * vxWorks taskDelays. Consequently, interrupts are disabled for substantial
 * periods of time. While the routines can be run in a tasking environment,
 * the intent is that readDs2401 be invoked once on startup to initialize
 * the ds2401 buffer and ds2401CRC checksum. Applications in tasking space can
 * then read the global buffer and checksum from memory.
 */
#include "vxWorks.h"
#include "arch/ppc/vxPpcLib.h"
#include "drv/multi/ppc860Cpm.h"
#include "drv/multi/ppc860Siu.h"
#include "intLib.h"
#include "logLib.h"

#define DS2401BIT 0x0001
#define DS2401DELAY(x) { int n; for (n = 0; n < x; n++); }
#define DS2401DELAY500 500*20
#define DS2401DELAY5   5*20
#define DS2401DELAY150 150*20
#define DS2401DELAY15  15*20
#define DS2401DELAY10  10*20
#define DS2401DELAY60  60*20

char ds2401[8];        /* global 8 byte buffer                      */
int ds2401CRC;         /* calculated checksum                       */

/*
 * ds2401Crc calculates the 2401 running checksum stored in the ds2401CRC
 * global. The checksum polynomial is x^8 + x^5 + x^4 + 1.
 */

void ds2401Crc(int bit)
{
    int bit0 = ds2401CRC >> 7;
    bit0 ^= bit;
    bit0 &= 1;
    ds2401CRC ^= (bit0 << 3) | (bit0 << 4);
    ds2401CRC <<= 1;
    ds2401CRC |= bit0;
}

/*
 * ds2401Pulse emits the 2401 Master Reset Pulse, senses and returns the
 * Presence Pulse from the 2401.
 */

int ds2401Pulse(void)
{
    int lockKey;
    unsigned int presence;
    void *vxImmr = (void *) vxImmrGet();
    lockKey = intLock();
    *PBPAR(vxImmr) &= ~DS2401BIT;
    *PBDIR(vxImmr) &= ~DS2401BIT;
    *PBODR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) |=  DS2401BIT;

    *PBDIR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY500);
    *PBDAT(vxImmr) |=  DS2401BIT;
    *PBDIR(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY60);
    presence = *PBDAT(vxImmr) & DS2401BIT;  /* get the 2401 presence bit    */
    intUnlock(lockKey);
    DS2401DELAY(DS2401DELAY500);
    ds2401CRC = 0;                          /* clear the calculated CRC     */
    return presence;
}

/*
 * ds2401WriteOne emits a 1 bit to the 2401.
 */

void ds2401WriteOne(void)
{
    int lockKey;
    void *vxImmr = (void *) vxImmrGet();
    lockKey = intLock();
    *PBPAR(vxImmr) &= ~DS2401BIT;
    *PBDIR(vxImmr) &= ~DS2401BIT;
    *PBODR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) |=  DS2401BIT;

    *PBDIR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY5);
    *PBDAT(vxImmr) |=  DS2401BIT;
    DS2401DELAY(DS2401DELAY150);
    *PBDIR(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY15);
    intUnlock(lockKey);
}

/*
 * ds2401WriteZero emits a 0 bit to the 2401.
 */

void ds2401WriteZero(void)
{
    int lockKey;
    void *vxImmr = (void *) vxImmrGet();
    lockKey = intLock();
    *PBPAR(vxImmr) &= ~DS2401BIT;
    *PBDIR(vxImmr) &= ~DS2401BIT;
    *PBODR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) |=  DS2401BIT;

    *PBDIR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY5);
    *PBDAT(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY150);
    *PBDIR(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY15);
    intUnlock(lockKey);
}

/*
 * ds2401Read reads and returns a single bit from the 2401.
 */

int ds2401Read(void)
{
    int lockKey;
    int data;
    void *vxImmr = (void *) vxImmrGet();
    lockKey = intLock();
    *PBPAR(vxImmr) &= ~DS2401BIT;
    *PBDIR(vxImmr) &= ~DS2401BIT;
    *PBODR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) |=  DS2401BIT;

    *PBDIR(vxImmr) |=  DS2401BIT;
    *PBDAT(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY5);
    *PBDAT(vxImmr) |=  DS2401BIT;
    *PBDIR(vxImmr) &= ~DS2401BIT;
    DS2401DELAY(DS2401DELAY10);
    data = *PBDAT(vxImmr) & DS2401BIT;
    intUnlock(lockKey);
    DS2401DELAY(DS2401DELAY150);
    ds2401Crc(data);                    /* update running CRC              */
    return data;
}

/*
 * ds2401ReadByte calls ds2401Read 8 times and aggregates the data to return a
 * ds2401 byte.
 */

int ds2401ReadByte(void)
{
    int n, data, c;
    c = 0;
    for (n = 0; n < 8; n++)
    {
        data = ds2401Read();
        c |= data << n;
    }
    return c;
}

/*
 * ds2401Command will send the 8 bit cmd to the 2401, bit-by-bit.
 */

void ds2401Command(int cmd)
{
    int n;

    for (n = 0; n < 8; n++, cmd >>= 1)
    {
        if (cmd & 1)
        {
            ds2401WriteOne();
        } else {
            ds2401WriteZero();
        }
    }
}

/*
 * readDs2401 is the only external routine needed to be called. It will
 * initialize the 8 byte ds2401 global buffer and the global ds2401CRC.
 * If the first byte of ds2401 is 0x01, and ds2401CRC is zero, then the
 * read operation was successful.
 */

void readDs2401(void)
{
    int n, data;
    data = ds2401Pulse();
#if 0
    logMsg("Pulse returns %d\n", data, 0, 0, 0, 0, 0);
#endif
    ds2401Command(0x33);
    for (n = 0; n < 8; n++)
    {
        data = ds2401ReadByte();
#if 0
        logMsg("ReadByte returns %02x\n", data, 0, 0, 0, 0, 0);
#endif
	ds2401[n] = data;
    }
#if 0
    logMsg("CRC ---> %02x\n", ds2401CRC & 0xff, 0, 0, 0, 0, 0);
#endif
}

