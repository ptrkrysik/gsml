/* $Id: address.h,v 10.7 1999/08/17 01:05:18 sutton Exp $
 * $Source: /export/sbcc/cepro/h/parser/address.h,v $
 *------------------------------------------------------------------
 * Cisco MicroWeb Server Version 2
 *
 * File: address.h
 * Description: Address(es) Header
 * Owner: Prasanna Parthasarathy
 * Notes:
 *
 * Copyright (c) 1997 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: address.h,v $
 * Revision 10.7  1999/08/17 01:05:18  sutton
 * Replaced red-black tree implementation with IOS code.
 * CSCdm75775:  Assert in cepro_201b11_dbg_cus1
 *
 * Revision 10.6  1999/08/05 20:53:51  vernikov
 * CSCdm69411:  delete of defualt gateway from GUI is BAD
 *
 * Revision 10.5.14.2  1999/08/16 23:18:33  sutton
 * Replaced red-black tree implementation with code from IOS.
 * CSCdm75775:  Assert in cepro_201b11_dbg_cus1
 * Branch: ROBIN202BR
 *
 * Revision 10.5.14.1  1999/08/05 20:51:27  vernikov
 * CSCdm69411:  delete of defualt gateway from GUI is BAD
 * Branch: ROBIN202BR
 *
 * Revision 10.5  1999/03/30 02:15:17  vernikov
 * CSCdm08933:  Need validation during configuration of certain address
 * fields
 *
 * Revision 10.4  1999/02/19 09:55:45  loril
 * CSCdm02735:  Add hostname support & martian validation in cli & gui
 * where needed
 *
 * Revision 10.3  1998/10/07 03:03:41  pradeep
 * CSCdk50280:  ICP CLI needes to be checked in
 *
 * Revision 10.2  1998/07/11 16:51:04  sutton
 * Straighten out some of the spaghetti dependencies between modules.
 * CSCdk13101:  merge MicroServer and Cache Engine source trees
 *
 * Revision 10.1  1998/06/09 17:41:39  sutton
 * code merge
 *
 * Revision 1.2  1997/10/09 18:14:14  pparthas
 * First Attempt to cleanup dead-code to reduce code-size; Included debug
 * scheme in place
 *
 * Revision 1.1  1997/06/25  00:36:54  pparthas
 * Initial Commit
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

#ifndef __ADDRESS_H__
#define __ADDRESS_H__


#define CLASS_A_BIT  0x80000000     /* off for class A, on for B, C, D */
#define CLASS_B_BIT  0x40000000     /* off for class B, on for C, D */
#define CLASS_C_BIT  0x20000000     /* off for class C, on for D  */
#define CLASS_D_BIT  0x10000000     /* off for class D */
 
#define CLASSA_BIT         0x80     /* off for class A, on for B, C, D */
#define CLASSB_BIT         0x40     /* off for class B, on for C, D */
#define CLASSC_BIT         0x20     /* off for class C, on for D  */
#define CLASSD_BIT         0x10     /* off for class D */
 
#define CLASS_A_MASK 0xFF000000     /* mask for class A net */
#define CLASS_B_MASK 0xFFFF0000     /* mask for class B net */
#define CLASS_C_MASK 0xFFFFFF00     /* mask for class C net */
#define CLASS_D_MASK 0x00000000     /* mask for class D net??? */
 
#define CLASS_A_SHIFT   24      /* A: bits shift to make int */
#define CLASS_B_SHIFT   16      /* B: bits shift to make int */
#define CLASS_C_SHIFT   8       /* C: bits shift to make int */

#define ADDR_ILLEGAL    0       /* illegal address */
#define ADDR_IP     1       /* IP address */

#define ADDRLEN_IP  4

/*
 * Flag bit definitions for use in martian() api
 */
#define MARTIAN_FLAG_IS_HOST            0x01  /* TRUE  => ipaddr is a host  */
                                              /* FALSE => ipaddr may be a   */
                                              /*          network address   */
#define MARTIAN_FLAG_MULTICAST_ALLOWED  0x02  /* TRUE  => do not consider a */
                                              /*          CLASS D ipaddr    */
                                              /*          a martian addr    */
                                              /* FALSE => do consider a     */
                                              /*          CLASS D ipaddr    */
                                              /*          a martian addr    */
 
/*
 * Special networks and addresses.
 */
#define IP_LOOPBACK_NET 0x7f000000  /* 127.0.0.0 */
#define MASK_HOST   0xffffffff  /* 255.255.255.255 */
#define IP_LIMITED_BROADCAST 0xffffffff /* 255.255.255.255 */
#define IP_BSD_LIMITED_BROADCAST 0  /* 0.0.0.0 */
#define IPADDR_ZERO ((ipaddrtype) 0)    /* for use as a default address */
 

union addresses {
    ipaddrtype ip_address;
};

#define ip_addr     addr.ip_address

typedef struct addrtype_ {
    uchar type;
    uchar length;
    union addresses addr;
} addrtype;

/* Structure and #defines used in parser_ipaddr_or_hostname() api */

typedef struct ipAddr_ {
        ipaddrtype      ipAddrNetOrder;                   /* network byte order    */
        ipaddrtype      ipAddrHostOrder;                  /* host byte order       */
        char            ipAddrStr[INET_ADDR_LEN]; /* dotted-decimal string */
} ipAddr_t, *pIpAddr_t;

/* Values of flags as the calling parameter to parser_ipaddr_or_hostname() */

#define IPADDR_CLI_PROCESSING                           0x01 /* TRUE  => processing for cli
                                                  * FALSE => processing for gui
                                                  */
#define IPADDR_CLI_IN_HELP                                      0x02 /* Flag is valid only if 
                                                  * IPADDR_CLI_HANDLING == TRUE
                                                  * TRUE  => in cli help
                                                  *          processing
                                                  * FALSE => not in cli help 
                                                  *          processing
                                                  */
#define IPADDR_HOSTNAME_ALLOWED                         0x04 /* TRUE  => input string can
                                                  *          be ipaddr or
                                                  *          hostname
                                                  * FALSE => input string can
                                                  *          an ipaddr only
                                                  */
#define IPADDR_MARTIAN_TEST_REQUIRED            0x08 /* TRUE  => do martian test
                                                  * FALSE => skip martian test
                                                  */
#define IPADDR_MARTIAN_IS_HOST_ADDRESS          0x10 /* Flag is valid only if
                                                  * IPADDR_MARTIAN_TEST_REQUIRED
                                                  *                  == TRUE
                                                  * TRUE  => input is host addr
                                                  * FALSE => input can be a
                                                  *          network addr
                                                  */

#define IPADDR_MARTIAN_MULTICAST_ALLOWED        0x20 /* Flag is valid only if
                                                  * IPADDR_MARTIAN_TEST_REQUIRED
                                                  *                  == TRUE
                                                  * TRUE  => multicast addr
                                                  *          wil not be 
                                                  *          consider a 
                                                  *          martian
                                                  * FALSE => multicast addr
                                                  *          will be considered
                                                  *          a martian
                                                  */

#define IPADDR_DEFAULT_ROUTING                  0x40 /* Flag is valid only for
                                                  *  default routing 
                                                  * if ip address is 0.0.0.0
                                                  */

/* The default flags represent what the GUI uses in most cases. */

#define IPADDR_GUI_DEFAULT_FLAGS (IPADDR_HOSTNAME_ALLOWED      | \
                                  IPADDR_MARTIAN_TEST_REQUIRED | \
                                  IPADDR_MARTIAN_IS_HOST_ADDRESS)

extern void ip_address_init( void );
extern boolean good_address (ipaddrtype , ipaddrtype , boolean );
extern boolean martian (ipaddrtype , ulong);
extern int parser_ipaddr_or_hostname (char *, int *, ulong, pIpAddr_t, char **);
extern int ValidateNetMask (ipaddrtype, char *, char **);
#endif /* __ADDRESS_H__ */
