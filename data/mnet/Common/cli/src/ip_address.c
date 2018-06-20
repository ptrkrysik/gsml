#include "parser_if.h"

#define IOS_PARSER
#include <inetLib.h>

/*#include <cenetdb.h>		 for inetGetHostByName() reference */
/* #include <cacheapp/utils.h>	 for remove_ws() reference */

static char *ip_addr_short_help (parseinfo *, uint, void * );
static char *parser_xlate_errcode(int errorCode); 

boolean match_ip_addr (parseinfo *, char *, int *, void *, int , uint, void *);

boolean subnetzero_enable;

void ip_address_init( void )
{
	subnetzero_enable = FALSE;
	parser_add_address(ADDR_IP, ip_addr_short_help, match_ip_addr);
}

char *ip_addr_short_help (parseinfo *csb, uint flag, void *idb )
{
    if (flag & ADDR_NAME_LOOKUP) {
    	return("Hostname or A.B.C.D");
    } else {
    	return("A.B.C.D");
    }
}

boolean match_ip_addr (parseinfo *csb, char *buf, int *pi, void *addr_pointer,
                       int in_help, uint flag, void *idb)
{
  addrtype *addrp = (addrtype *)addr_pointer;
    ushort num[4];
    boolean matched;
    int init_ptr = *pi;


    if (match_decimal(buf, 0, 255, pi, &num[0]) &&
        match_char(buf, pi, '.') &&
        match_decimal(buf, 0, 255, pi, &num[1]) &&
        match_char(buf, pi, '.') &&
        match_decimal(buf, 0, 255, pi, &num[2]) &&
        match_char(buf, pi, '.') &&
        match_decimal(buf, 0, 255, pi, &num[3])) {

        addrp->ip_addr = ((((ulong) num[0]) << 24) |
                          (((ulong) num[1]) << 16) |
                          (((ulong) num[2]) << 8)  |
                          ((ulong) num[3]));
        matched = TRUE;
    } else {
      matched = FALSE;
    }

    if (matched) {
      addrp->type = ADDR_IP;
      addrp->length = ADDRLEN_IP;
      return(TRUE);
    }
    return(FALSE);
}
/*
 * remove_ws
 * Remove leading and trailing whitespace
 */

char *
remove_ws (char *s)
{
    char *x;
 
    /* Check for null string */
    if (!s) {
	return(s);
    }
 
    /* Remove leading whitespace */
    while ((*s == ' ') || (*s == '\t')) {
	s++;
    }
 
    /* Move to end of string */
    x = s;
    while (*x) {
	x++;
    }
 
    /* Remove trailing whitespace */
    x--;
    while ((*x == ' ') || (*x == '\t')) {
	*x = '\0';
	x--;
    }
    return(s);
}


/*
 * parser_ipaddr_or_hostname
 *
 * This is a utility function that is used by both the CLI parser and
 * the GUI so that ipaddress and hostnames can be handled in an
 * indentical manner.
 */

/*
??? WHAT SHOULD THE INTERFACE BE - RETURN ERRORCODE OR RETURN ERROR AND
    SET "errno"?
*/

/* If doing cli processing, then pInputStr points to the start of the
 * ipaddr (or possibly hostname) field within the entire cli parse
 * buffer for the command (i.e., other fields may following in the buffer).
 * If doing gui processing, then pInputStr points to the start of
 * a null terminated string which contains only the ipaddr field
 * (or possibly hostname).
 */

int
parser_ipaddr_or_hostname (char *pInputStr, int *pOffset, ulong flagsParm,
                           pIpAddr_t pIpAddrInfo, char **ppErrMsg)
{
	int				currOffset;			/* for match_decimal & match_char apis
						 				 * which increment currOffset as buffer
						 				 * pointed to by pInputStr is parsed
						 			     * NOTE:  Only interesting to cli 
						 			     */
    ushort			num[4];		        /* for match_decimal/match_char apis */
    /*    RESOLV_PARAMS_S resolvParms;         for resolvParamsGet() api */
    struct in_addr  ipaddrTemp;         /* for inet_ntoa_b() api */
    char 		   *pHostName;		    /* ptr to dynamically allocated buff */
    int 			tempOffset;			/* for get_name() api */
	ulong			martianFlags = 0;	/* flags for martian() api */
    int             i;

	/* If doing processing for the GUI,
     * then remove any white space from the string and
	 * return an error if the resultant string is null.
	 * Note that removing white space may alter the
	 * caller's buffer if white space was removed from
	 * the end of the string.
	 * Note that in the case of a null string, the ip address
	 * data structure is reset because the caller may want
	 * to do processing even in the case of a null string.
	 */
	if ((flagsParm & IPADDR_CLI_PROCESSING) != IPADDR_CLI_PROCESSING) {
	  pInputStr = remove_ws(pInputStr); 
	  if (strlen(pInputStr) == 0) {
	    pIpAddrInfo->ipAddrNetOrder = 0;
	    pIpAddrInfo->ipAddrHostOrder = 0;
	    pIpAddrInfo->ipAddrStr[0] = '\0';
	    *ppErrMsg = parser_xlate_errcode(S_parser_SYNTAX_NULL_STRING);
	    return (S_parser_SYNTAX_NULL_STRING);
	  }
	}

	/* Track the current offset into the caller's buffer in a temporary
	 * variable in case match_decimal() or match_char() fails and we need
	 * to start at the beginning of the field again if it can be considered
	 * a hostname.
	 */

	currOffset = *pOffset;

	/* Check if this is an ip address in decimal dotted notation */

	if (match_decimal(pInputStr, 0, 255, &currOffset, &num[0]) &&
        match_char(pInputStr, &currOffset, '.') &&
        match_decimal(pInputStr, 0, 255, &currOffset, &num[1]) &&
        match_char(pInputStr, &currOffset, '.') &&
        match_decimal(pInputStr, 0, 255, &currOffset, &num[2]) &&
        match_char(pInputStr, &currOffset, '.') &&
        match_decimal(pInputStr, 0, 255, &currOffset, &num[3])) {

		/* If doing processing for the GUI, then return an
		 * error if the character after the last valid
		 * ip address character is not either the null
		 * terminator of a white space character (i.e.,
		 * a space or tab).
		 *
		 * Note that for CLI processing, the check is not
		 * done because the parser itself handles it and
		 * puts the help indicator character (^) exactly
		 * at the bad character following a valid ip
		 * address.  If an error was returned here for
		 * CLI processing also, the result would be that
		 * the help indicator character would be at the
		 * beginning of the ip address which is less
		 * desirable.
		 */
		
		if ( (flagsParm & IPADDR_CLI_PROCESSING) != IPADDR_CLI_PROCESSING ) {

			if (( *(pInputStr + currOffset) != '\0' ) &&
				( *(pInputStr + currOffset) != ' '  ) &&
				( *(pInputStr + currOffset) != '\t' )) {
				*ppErrMsg = parser_xlate_errcode(
							S_parser_SYNTAX_INVALID_IPADDR_FORMAT);
				return (S_parser_SYNTAX_INVALID_IPADDR_FORMAT);
			}
		}

		/* Input string an ip address in dotted-decimal notation.
		 * Save the result in the following three different formats:
		 *    1. unsigned long integer in network byte order
		 *    2. unsigned long integer in host byte order
		 *    3. character string in dotted-decimal notation
		 */

		/* Save in network byte order */

#if BYTE_ORDER == BIG_ENDIAN
		pIpAddrInfo->ipAddrNetOrder = ((((ulong) num[0]) << 24) |
                                       (((ulong) num[1]) << 16) |
                                       (((ulong) num[2]) << 8)  |
                                        ((ulong) num[3]));

#elif BYTE_ORDER == LITTLE_ENDIAN
		pIpAddrInfo->ipAddrNetOrder = ((((ulong) num[3]) << 24) |
                                       (((ulong) num[2]) << 16) |
                                       (((ulong) num[1]) << 8)  |
                                        ((ulong) num[0]));

#else  /* force a compilation error if BTYE_ORDER is not defined */
#error Unknown BYTE_ORDER
#endif

		/* Save ip address in host byte order */
		pIpAddrInfo->ipAddrHostOrder = ntohl(pIpAddrInfo->ipAddrNetOrder);

		/* Save ip address as a null terminated dotted-decimal string. */
		ipaddrTemp.s_addr = pIpAddrInfo->ipAddrNetOrder;
		inet_ntoa_b(ipaddrTemp, pIpAddrInfo->ipAddrStr);

		/* Update the caller's offset into the input string. */
		*pOffset = currOffset;
 
    } 
#if 0
else {

		/* If input string can be not considered a hostname,
		 * then update the caller's offset into the parsed string
		 * and return an error.
		 */

		if ((flagsParm & IPADDR_HOSTNAME_ALLOWED) != IPADDR_HOSTNAME_ALLOWED) {
			*pOffset = currOffset;
			*ppErrMsg =
				parser_xlate_errcode(S_parser_SYNTAX_INVALID_IPADDR_FORMAT);
			return (S_parser_SYNTAX_INVALID_IPADDR_FORMAT);
		}

		/* Treat the input string as a hostname.
		 * Make a copy of the hostname from the beginning of the field and
		 * verify the string is well-formed for a hostname. 
		 */

		pHostName = PARSERmalloc(PARSEBUF + 1);
		if (pHostName == NULL) {
			*ppErrMsg =
				parser_xlate_errcode(S_parser_RESOURCE_ALLOCATION_ERROR);
			if ((flagsParm & IPADDR_CLI_PROCESSING) == IPADDR_CLI_PROCESSING) {
				printf("%s\n", *ppErrMsg);
            }
			return (S_parser_RESOURCE_ALLOCATION_ERROR);
		}
 
		if (get_name(&pInputStr[*pOffset],
					 pHostName, &tempOffset, PARSEBUF) == FALSE) {
			PARSERfree(pHostName);
			*ppErrMsg =
				parser_xlate_errcode(S_parser_SYNTAX_INVALID_HOSTNAME_FORMAT);
			return (S_parser_SYNTAX_INVALID_HOSTNAME_FORMAT);
		}

		/* If the cli is just processing help, there is no more work.
         * Update the caller's offset into the input string and return
		 */
		if ((flagsParm & IPADDR_CLI_IN_HELP) == IPADDR_CLI_IN_HELP) {
			*pOffset += tempOffset;
			PARSERfree(pHostName);
			return (OK);	/* note there is no ipaddr in info structure */
		}

		if (name_verify(pHostName) == FALSE) {
			PARSERfree(pHostName);
			*ppErrMsg =
				parser_xlate_errcode(S_parser_SYNTAX_INVALID_HOSTNAME_FORMAT);
			return (S_parser_SYNTAX_INVALID_HOSTNAME_FORMAT);
		}

		/* If there are no ip name servers configured to do the DNS lookup,
		 * then return an error.
		 */
		resolvParms.nameServersAddr[0][0] = NULL;  /* init before api call */
		resolvParamsGet(&resolvParms);             /* DNS resolver api call */
		if (resolvParms.nameServersAddr[0][0] == NULL) {
			*ppErrMsg = parser_xlate_errcode(S_parser_IPADDR_NO_NAME_SERVER);
			if ((flagsParm & IPADDR_CLI_PROCESSING) == IPADDR_CLI_PROCESSING) {
				printf("%s \"%s\"\n", *ppErrMsg, pHostName);
			}
			PARSERfree(pHostName);
			return (S_parser_IPADDR_NO_NAME_SERVER);
		}

        for (i = 0; i < MAXNS; i++)
        {
            if (resolvParms.nameServersAddr[i][0] != '\0')
            {
                if (OK == 
                    ping(resolvParms.nameServersAddr[i], 1, PING_OPT_SILENT))
	                break;
            }
        }

        if (i == MAXNS)
        {
	        printf("Cannot communicate with any ip name-server\n");
	        return (S_parser_IPADDR_NO_NAME_SERVER);
        }

		if ((flagsParm & IPADDR_CLI_PROCESSING) == IPADDR_CLI_PROCESSING) {
			printf("Translating \"%s\" using configured ip name-server(s)...",
					pHostName);
		}

		/* Do the DNS lookup on the input hostname.
		 * The inetGetHostByName() function looks in the local DNS
		 * cache first (if it is on) and if the hostname is not found,
		 * then it consults the DNS resolver.
		 * If successful, the return value is the ip address u_long
		 * in network byte order.
		 * The inetGetHostByName() function is thread-safe.
		 */

		pIpAddrInfo->ipAddrNetOrder = NULL;
		/*RAMS: commented this for now
		 * inetGetHostByName(pHostName,strlen(pHostName));
		 */

		if (pIpAddrInfo->ipAddrNetOrder == NULL) {
			if ((flagsParm & IPADDR_CLI_PROCESSING) == IPADDR_CLI_PROCESSING) {
				printf("Failed\n");
			}
			PARSERfree(pHostName);
			*ppErrMsg =
				parser_xlate_errcode(S_parser_IPADDR_HOSTNAME_LOOKUP_FAILED);
			return (S_parser_IPADDR_HOSTNAME_LOOKUP_FAILED);
		}

		/* Save the ip address in two other formats:
		 *    1. unsigned long integer in host byte order
		 *    2. character string in dotted-decimal notation
		 */

		/* Save ip address in host byte order */
		pIpAddrInfo->ipAddrHostOrder = ntohl(pIpAddrInfo->ipAddrNetOrder);

		/* Save ip address in dotted-decimal notation. */
		ipaddrTemp.s_addr = pIpAddrInfo->ipAddrNetOrder;
		inet_ntoa_b(ipaddrTemp, pIpAddrInfo->ipAddrStr);

		if ((flagsParm & IPADDR_CLI_PROCESSING) == IPADDR_CLI_PROCESSING) {
			printf("%s[OK]\n", pIpAddrInfo->ipAddrStr);
		}

		/* Update the offset into the caller's buffer. */
        *pOffset += tempOffset;

		PARSERfree(pHostName);
	}
#endif

	/* Check if we test Default Routing
	 * Default Routing allows 0.0.0.0 ip address
	 * test it and return OK if address is 0.0.0.0
	 */
	 if ((flagsParm & IPADDR_DEFAULT_ROUTING ) ==
					IPADDR_DEFAULT_ROUTING ) {

		if ( (num[3] == 0) && (num[2] == 0) && (num[1] == 0) && (num[0] == 0) ) {
			*ppErrMsg = parser_xlate_errcode(OK);
			return (OK);
		}
	 }

	/* The caller's structure pointed to by pIpAddrInfo is updated.
	 * If caller did not request the Martian ip address test,
	 * then there is no more work to do so return success.
	 */

	if ((flagsParm & IPADDR_MARTIAN_TEST_REQUIRED) !=
				     IPADDR_MARTIAN_TEST_REQUIRED) {
		*ppErrMsg = parser_xlate_errcode(OK);
		return (OK);
	}

	/* Perform Martian ip address test based on the caller's input flags. */

	if ((flagsParm & IPADDR_MARTIAN_IS_HOST_ADDRESS) ==
				     IPADDR_MARTIAN_IS_HOST_ADDRESS) {
		martianFlags |= MARTIAN_FLAG_IS_HOST; 
	}

	if ((flagsParm & IPADDR_MARTIAN_MULTICAST_ALLOWED) ==
				     IPADDR_MARTIAN_MULTICAST_ALLOWED) {
		martianFlags |= MARTIAN_FLAG_MULTICAST_ALLOWED; 
	}

	if (martian(pIpAddrInfo->ipAddrHostOrder, martianFlags) == TRUE) {
		*ppErrMsg = parser_xlate_errcode(S_parser_IPADDR_RESERVED);
		return (S_parser_IPADDR_RESERVED);
	}
	else {
		*ppErrMsg = parser_xlate_errcode(OK);
		return (OK);
	}
}

/* The following table is used by parser_xlate_errcode() below to
 * translate a parser error code that can possibly be returned by the
 * parser_ipaddress_or_hostname() function into a string that can be
 * used to format an error message.
 * This feature of parser_ipaddress_or_hostname() is used by the GUI
 * so that it can display specific error messages to the administrator.
 */

struct xlateInfo {
	int		errCode;		/* error code */
	char	*pErrMsg;		/* pointer to error message for errcode */
} static parser_xlateTable[] = {
	{ OK,
	  "Success" },
	{ S_parser_SYNTAX_NULL_STRING,
	  "Host is a null string" },
	{ S_parser_SYNTAX_INVALID_IPADDR_FORMAT,
	  "IP address is malformed" },
	{ S_parser_SYNTAX_INVALID_HOSTNAME_FORMAT,
	  "Hostname is malformed" },
	{ S_parser_IPADDR_RESERVED,
	  "Host address is not valid because it is reserved" },
	{ S_parser_IPADDR_NO_NAME_SERVER,
	  "No ip name-server configured for DNS lookup of hostname" },
	{ S_parser_IPADDR_HOSTNAME_LOOKUP_FAILED,
	  "DNS lookup failed for hostname" },
	{ S_parser_RESOURCE_ALLOCATION_ERROR,
	  "Internal resource allocation error" },
	{ S_parser_SYNTAX_INVALID_NETMASK_FORMAT,
	  "Netmask is malformed" },
	{ S_parser_SYNTAX_INVALID_NETMASK_CONTIGUOUS,
	  "Netmask binary format should have contiguous 1" },
	{ S_parser_SYNTAX_INVALID_NETMASK_IPADDR_MISMATCH,
	  "Netmask inconsistent with destination address" },
};

#define ERRMSG_UNKNOWN	"Unknown error occurred"
#define MAXXLATES (sizeof (parser_xlateTable) / sizeof (parser_xlateTable[0]))

static char *
parser_xlate_errcode(int errorCode)
{
    register int i;

    for (i = 0; i < MAXXLATES; i++) {
        if (errorCode == parser_xlateTable[i].errCode) {
			return (parser_xlateTable[i].pErrMsg);
		}
	}

	/* errorCode was not in table, so return a default error message */
	return (ERRMSG_UNKNOWN);
}




/*
 * martian
 * Returns "TRUE" if the address is one of Mill's Martians, i.e., it is not
 * a valid IP address for a host (if MARTIAN_FLAG_IS_HOST is TRUE) or network
 * if (MARTIAN_FLAG_IS_HOST is FALSE).  An IP address is not valid if it
 * is one of the reserved IP addresses.
 *
 * "martianFlag" contains various flags provided by the caller which dictate
 * how the martian checking is performed.  See h/parser/address.h for the
 * flag definitions and meaning.
 */
 
/* The following table is used by martian() to determine which IP addresses
 * are considered invalid because they are reserved.
 */

struct martiantype {
    ipaddrtype address;     /* potential bad address */
    ipaddrtype mask;        /* address mask */
    tinybool net;			/* network check */
    tinybool valid_entry;   /* TRUE  == this entry in table is valid */
                            /* FALSE == this entry in table is not valid */
} static martians[] = {

/* Description of special addresses
 * Each entry in martian() table notes which case applies (if any)
 *
 * case i   - network id = all bits 0; host id = any
 *            Note:  For Class A (00.x.x.x), these are reserved addresses,
 *                   including the address 0.0.0.0
 *                   For Class B (128.0.x.x) and Class C (192.0.0.x),
 *                   IANA has assigned registered users.
 *
 * case ii  - network id = all bits 1; host id = any
 *            Note:  For Class A (127.x.x.x) these address are used
 *                   as internal host loopback addresses.
 *                   For Class B (191.255.x.x) and Class C (223.255.255.x),
 *                   these addresses are reserved.
 *
 * case iii - network id = any valid netid; host id = all bits 0 
 *            Note: Identifies an entire network; not a valid host address.

 * case iv  - network id = any valid netid; host id = all bits 1 
 *            Directed broadcast to specified network.
 *
 * case v   - network id = all bits 1; host id = all bits 1
 *            Limited broadcast address.
 *            A datagram with this address is never forwared outside
 *            the network or subnetwork of the source.
 *
 * case vi  - Class D (224.x.x.x) - the entire class is reserved for
 *            multicast addresses (the low order 24 bits represent
 *            the multicast group ID).
 *
 * case vii - Class E (240.x.x.x - 255.x.x.x) - the entire class is
 *            reserved for future use.
 */

/*
 * Limited Broadcast Address
 */
 {0xFFFFFFFF, 0xFFFFFFFF, FALSE, TRUE}, /* 255.255.255.255, 255.255.255.255
                                         * case v - reserved */

/*
 * Class A specials
 */
 {0x00000000, 0xFF000000, FALSE, TRUE}, /* 0.0.0.0,  255.0.0.0
                                         * case i - reserved */
 {0x7F000000, 0xFF000000, FALSE, TRUE}, /* 127.0.0.0,  255.0.0.0
                                         * case ii - reserved
                                         * internal host loopback */
 {0x00000000, 0x80FFFFFF, TRUE,  TRUE}, /* 0.0.0.0,  128.255.255.255
                                         * case iii - network address */
 {0x00FFFFFF, 0x80FFFFFF, FALSE, TRUE}, /* 0.255.255.255, 128.255.255.255
                                         * case iv - directed broadcast */
 
/*
 * Class B specials
 */
 {0xBFFF0000, 0xFFFF0000, FALSE, TRUE}, /* 191.255.0.0,  255.255.0.0
                                         * case ii - reserved */
 {0x80000000, 0xC000FFFF, TRUE,  TRUE}, /* 128.0.0.0, 192.0.255.255
                                         * case iii - network address */
 {0x8000FFFF, 0xC000FFFF, FALSE, TRUE}, /* 128.0.255.255, 192.0.255.255
                                         * case iv - directed broadcast */
 
/*
 * Class C specials
 */
 {0xDFFFFF00, 0xFFFFFF00, FALSE, TRUE}, /* 223.255.255.0, 255.255.255.0
                                         * case ii - reserved */
 {0xC0000000, 0xE00000FF, TRUE,  TRUE}, /* 192.0.0.0,  224.0.0.255
                                         * case iii - network address */
 {0xC00000FF, 0xE00000FF, FALSE, TRUE}, /* 192.0.0.255, 224.0.0.255
                                         * case iv - directed broadcast */
 
/*
 * Class D
 */
 {0xE0000000, 0xF0000000, FALSE, TRUE}, /* 224.0.0.0, 240.0.0.0
                                         * case vi - the entire class is
                                         * reserved for multicast addresses */

/*
 * Class E
 */
 {0xF0000000, 0xF0000000, FALSE, TRUE}, /* 240.0.0.0, 240.0.0.0
                                         * case vii - the entire class is
                                         * reserved for future use */
};

#define MAXMARTIANS (sizeof (martians) / sizeof (martians[0]))
#define CLASS_D_ADDRESS 0xE0000000

boolean
martian (register ipaddrtype address, ulong martianFlags)
{
    register int i;

    for (i = 0; i < MAXMARTIANS; i++) {
        if ((address & martians[i].mask) == martians[i].address) {

            /* If the table entry is marked invalid, then skip it */
            if (martians[i].valid_entry == FALSE) {
                continue;
            }

            /* If this is a Class D multicast address and
             * caller specified multicast address are allowed,
             * then this address is not consider an invalid address.
             */ 
            if (martians[i].address == CLASS_D_ADDRESS) {
                if ((martianFlags & MARTIAN_FLAG_MULTICAST_ALLOWED) ==
                                    MARTIAN_FLAG_MULTICAST_ALLOWED) {
                    return (FALSE);  /* caller's address is not a martian */
                }
                else {
                    return (TRUE);   /* caller's address is a martian */
                }
            }

            /* If the caller specified that the address
             * being check may be a network address and
             * this address is OK as a network address
             * then continue checking the rest of the table.
             * Else, the caller's address is considered a martian.
             */
            if (((martianFlags & MARTIAN_FLAG_IS_HOST) !=
                                 MARTIAN_FLAG_IS_HOST) &&
                 (martians[i].net == TRUE)) {
                continue;  /* continue checking the table */
            }
            else {
                return (TRUE);  /* it's a martian */
            }
        }
    }
    /* Passed all table checks, so address must not be reserved ip address. */
    return (FALSE);
}


/*
 * mask_contiguous
 * Return TRUE if the given mask is left contiguous, FALSE otherwise.
 */
 
boolean mask_contiguous (ipaddrtype mask)
{
    while ((mask & 0xff000000) == 0xff000000)
    mask <<= 8;
    while (mask & 0x80000000)
        mask <<= 1;
    if (mask)
        return(FALSE);
    return(TRUE);
}

/*
 * get_majormask
 * Given an address, return the mask of major network number.
 */
 
ipaddrtype get_majormask (ipaddrtype address)
{
    if (!address)           /* net 0.0.0.0 */
    return(0);          /* mask 0.0.0.0 */
    if (!(address & CLASS_A_BIT))   /* Class A network */
    return(CLASS_A_MASK);
    else if (!(address & CLASS_B_BIT))  /* Class B network */
    return(CLASS_B_MASK);
    else if (!(address & CLASS_C_BIT))  /* Class C network */
    return(CLASS_C_MASK);
    else                /* unknown network class */
    return(MASK_HOST);
}

#if 0

/*
 * ValidateNetMask
 * Validate a network mask
 * return OK or Error Number.
 * Checks:
 *  - a netmask should have valid IP address
 *  - the mask must be left contiguous
 *  - the mask must be consistent with the destination address
 */
 
int ValidateNetMask (ipaddrtype address, char *sMask, char **ppErrMsg)
{
    int		offset;
    ipAddr_t	ipAddrInfo;
    char	*pErrMsg;
    int		errcode;
    ipaddrtype	mask;
	
    /* Call the parser to process the sMask field */
    offset = 0;
    errcode = parser_ipaddr_or_hostname(sMask,
                                        &offset,
                                        0,
                                        &ipAddrInfo,
                                        &pErrMsg);
    if (errcode != OK) {
	*ppErrMsg = parser_xlate_errcode(
	            S_parser_SYNTAX_INVALID_NETMASK_FORMAT);
	return (S_parser_SYNTAX_INVALID_NETMASK_FORMAT);
    }
    mask = ipAddrInfo.ipAddrHostOrder;

    if (!mask_contiguous(mask)) 
    {
	*ppErrMsg = parser_xlate_errcode(
	            S_parser_SYNTAX_INVALID_NETMASK_CONTIGUOUS);
	return (S_parser_SYNTAX_INVALID_NETMASK_CONTIGUOUS);
    }

    if (address & ~mask)
    {
    *ppErrMsg = parser_xlate_errcode(
                S_parser_SYNTAX_INVALID_NETMASK_IPADDR_MISMATCH);
    return (S_parser_SYNTAX_INVALID_NETMASK_IPADDR_MISMATCH);
    }

    *ppErrMsg = parser_xlate_errcode(OK);
    return (OK);
}

#endif
/*
 * good_address
 * Validate an address and/or network mask, return TRUE/FALSE.
 * Checks:
 *  - an address or mask is never all zeros or all ones
 *  - an address must not be a reserved IP address, i.e., a martian
 *  - the mask may not have zeros in the major network portion, that is,
 *    it must be consistent with the address class type.
 *  - the mask must be left contiguous
 */
 
boolean good_address (ipaddrtype address, ipaddrtype mask, boolean maskcheck)
{
    ipaddrtype basicmask, result;
 
    if (martian(address, MARTIAN_FLAG_IS_HOST) == TRUE) {
    	return(FALSE);
	}

    if (maskcheck) {
    	if ((mask == 0L) || (mask == -1L))
        	return(FALSE);
    	if (!mask_contiguous(mask))
        	return(FALSE);
    	basicmask = get_majormask(address);
    	if (!basicmask)
        	return(FALSE);
    	if ((mask | address) == -1)
        	return(FALSE);      /* all ones in host part */
    	if ((address & ~mask) == 0)
        	return(FALSE);      /* all zeros in host part */
    	if (!subnetzero_enable && (mask > basicmask)) {
        	result = (address & ~basicmask);    /* mask off major net bits */
        	if ((result & mask) == 0L)  /* mask off host bits */
        	return(FALSE);      /* disallow subnet zero */
    	}
    }
    return(TRUE);
}

/*
 * name_verify
 * Screen out malformed hostname strings.
 * Rejects null strings and strings with illegal chars.  Note that we don't
 * do any length checking since domain style names may be arbitrarily long.
 */
 
boolean name_verify (char *name)
{
    boolean good;
    char c;
    boolean non_numeric_seen = FALSE;
 
    if (null(name))
		return(FALSE);      /* screen out null strings */

    while ((c = *name++) != '\0') {
		if (((c >= 'A') && (c <= 'Z')) ||
			((c >= 'a') && (c <= 'z')) ||
			(c == '-') || (c == '_')) {
			good = TRUE;
			non_numeric_seen = TRUE;
		} else
			good = ((c >= '0') && (c <= '9')) || (c == '.');
		
		if (!good)
			return(FALSE);  /* screen out bad characters */
    };
	
    if (!non_numeric_seen)  /* Check for strings with only numbers */
		return(FALSE);     /* with possibles dots.  Not allowed! */
	
    return(TRUE);         /* string appears well formed */
}


#ifdef IOS_PARSER

void ipaddrmask_action (parseinfo *csb, transition *mine,
                        ipaddrmask_struct * const arg)
{
    addrtype *addrp = CSBVAR(csb, arg->addr, addrtype);
    addrtype *maskp = CSBVAR(csb, arg->mask, addrtype);
    int i;
    uint mask_number = 0;
    static const char short_help[] = "A.B.C.D  {IP address  A.B.C.D}";
    static const char prefix_help[] = "IP prefix and prefix mask";
 
    push_node(csb, mine->alternate);
    INVALID_NVGEN_CHECK;
 
    if (help_check(csb)) {
    save_help_long(csb, short_help, arg->help, csb->priv);
    return;
    }
 
    i = 0;
    if (match_ip_addr(csb, &csb->line[csb->line_index], &i, addrp,
              csb->in_help, 0, NULL)) {
    if (match_char(&csb->line[csb->line_index], &i, '/')) {
        if (match_number(csb, 0, 32, &i, &mask_number,
                 NUMBER_DEC | NUMBER_WS_OK | NUMBER_HELP_CHECK,
                 "Length of prefix mask")) {
        uint n = 1<<31;
 
        while (mask_number--) {
            maskp->ip_addr |= n;
            n = n>>1;
        }
 
        maskp->type = ADDR_IP;
        maskp->length = ADDRLEN_IP;
 
        csb->line_index += i;
        push_node(csb, mine->accept);
        csb->line_index -= i;
        }
    } else {
        if (match_whitespace2(&csb->line[csb->line_index],
                  &i, csb->in_help)) {
        csb->line_index += i;
        if (help_check(csb)) {
            save_help_long(csb, short_help, prefix_help, csb->priv);
            csb->line_index -= i;
            return;
        }
        csb->line_index -= i;
 
        if (match_ip_addr(csb, &csb->line[csb->line_index], &i, maskp,
                  csb->in_help, 0, NULL)) {
            if (match_whitespace2(&csb->line[csb->line_index],
                      &i, csb->in_help)) {
            csb->line_index += i;
            push_node(csb, mine->accept);
            csb->line_index -= i;
            } else {
            help_or_error(csb, i, prefix_help,
                      NO_COMMAND_COMPLETION);
            }
        } else {
            help_or_error(csb, i, prefix_help,
                  NO_COMMAND_COMPLETION);
        }
        } else {
        /*
         * Two short helps in one string, don't try
         * this at home kids
         */
        help_or_error(csb, i, "/\tIP prefix mask",
                  NO_COMMAND_COMPLETION);
        }
    }
    } else {
    help_or_error(csb, i, short_help, NO_COMMAND_COMPLETION);
    }
}


void ipmask_action (parseinfo *csb, transition *mine,
            ipmask_struct * const arg)
{
    addrtype *maskp = CSBVAR(csb, arg->mask, addrtype);
    int i;
    uint mask_number = 0;
    static const char short_help[] = "/nn or A.B.C.D";
    static const char prefix_help[] = "IP prefix mask";
 
    push_node(csb, mine->alternate);
    INVALID_NVGEN_CHECK;
 
    if (help_check(csb)) {
    save_help_long(csb, short_help, arg->help, csb->priv);
    return;
    }
 
    i = 0;
    if (match_char(&csb->line[csb->line_index], &i, '/')) {
    if (match_number(csb, 0, 32, &i, &mask_number,
             NUMBER_DEC | NUMBER_WS_OK | NUMBER_HELP_CHECK,
             "Length of prefix mask")) {
        uint n = 1<<31;
 
        while (mask_number--) {
        maskp->ip_addr |= n;
        n = n>>1;
        }
 
        maskp->type = ADDR_IP;
        maskp->length = ADDRLEN_IP;
 
        csb->line_index += i;
        push_node(csb, mine->accept);
        csb->line_index -= i;
    }
    } else {
    if (match_whitespace2(&csb->line[csb->line_index], &i, csb->in_help)) {
        csb->line_index += i;
        if (help_check(csb)) {
        save_help_long(csb, short_help, prefix_help, csb->priv);
        csb->line_index -= i;
        return;
        }
        csb->line_index -= i;
 
        if (match_ip_addr(csb, &csb->line[csb->line_index], &i, maskp,
                  csb->in_help, 0, NULL)) {
        if (match_whitespace2(&csb->line[csb->line_index],
                      &i, csb->in_help)) {
            csb->line_index += i;
            push_node(csb, mine->accept);
            csb->line_index -= i;
        } else {
            help_or_error(csb, i, prefix_help, NO_COMMAND_COMPLETION);
        }
        } else {
        help_or_error(csb, i, prefix_help, NO_COMMAND_COMPLETION);
        }
    } else {
        /*
         * Two short helps in one string, don't try
         * this at home kids
         */
        help_or_error(csb, i, "/\tIP prefix mask", NO_COMMAND_COMPLETION);
    }
    }
}

#endif /* IOS_PARSER */
