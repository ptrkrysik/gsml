#ifndef _PARSER_ERRNO_H
#define _PARSER_ERRNO_H

#define S_parser_PROGRAMMING_ERROR      (M_parser | 1)
#define S_parser_NVRAM_WRITE_ERROR      (M_parser | 2)
#define S_parser_NVRAM_NULL_WRITE       (M_parser | 3)
#define S_parser_NVRAM_READ_ERROR       (M_parser | 4)
#define S_parser_NVRAM_BAD                      (M_parser | 5)
#define S_parser_NVRAM_CRC_FAIL         (M_parser | 6)
#define S_parser_SYNTAX_AMBIGUOUS       (M_parser | 7)
#define S_parser_SYNTAX_UNKNOWN         (M_parser | 8)
#define S_parser_SYNTAX_INCOMPLETE      (M_parser | 9)
#define S_parser_SYNTAX_INVALID         (M_parser | 10)
#define S_parser_SYNTAX_NULL_STRING                     (M_parser | 11)
#define S_parser_SYNTAX_INVALID_IPADDR_FORMAT   (M_parser | 12)
#define S_parser_SYNTAX_INVALID_HOSTNAME_FORMAT (M_parser | 13)
#define S_parser_IPADDR_RESERVED                                (M_parser | 14)
#define S_parser_IPADDR_NO_NAME_SERVER                  (M_parser | 15)
#define S_parser_IPADDR_HOSTNAME_LOOKUP_FAILED  (M_parser | 16)
#define S_parser_RESOURCE_ALLOCATION_ERROR              (M_parser | 17)

#define S_parser_SYNTAX_INVALID_NETMASK_FORMAT          (M_parser | 18)
#define S_parser_SYNTAX_INVALID_NETMASK_CONTIGUOUS      (M_parser | 19)
#define S_parser_SYNTAX_INVALID_NETMASK_IPADDR_MISMATCH (M_parser | 20)

/****************  ERROR CODE DOCUMENTATION  ********************************

S_parser_SYNTAX_NULL_STRING - input string is null
S_parser_SYNTAX_INVALID_IPADDR_FORMAT - invalid dotted-decimal notation
S_parser_SYNTAX_INVALID_HOSTNAME_FORMAT - invalid string for a hostname

For parser_ipaddr_or_hostname() api, if the input flag had
IPADDR_MARTIAN_TEST_REQUIRED as TRUE, the following error code
is also possible.

S_parser_IPADDR_RESERVED - the IP address is one of the reserved IP addresses

For parser_ipaddr_or_hostname(0 api, if input flag had
IPADDR_HOSTNAME_ALLOWED as TRUE, the folllowing error codes are also possible.

S_parser_IPADDR_NO_NAME_SERVER - no ip name servers configured to do DNS lookup
S_parser_IPADDR_HOSTNAME_LOOKUP_FAILED - DNS resolver failed on hostname lookup

S_parser_RESOURCE_ALLOCATION_ERROR - internal error - unable to acquire resource


S_parser_SYNTAX_INVALID_NETMASK_FORMAT - Netmask is malformed
S_parser_SYNTAX_INVALID_NETMASK_CONTIGUOUS - Netmask binary format should 
                                                 have contiguous 1

******************************************************************************/

#endif /* _PARSER_ERRNO_H */
