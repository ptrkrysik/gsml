/* Global commands */
#define HISTORY                  0
#define HISTORY_ON               1
#define HISTORY_OFF              2
#define HISTORY_PRINT            3
#define HISTORY_SIZE             4
#define DUMP                     5

/* Exec and privileged command function defines */

#define CMD_CONNECT_DEFAULT           1
#define CMD_CONNECT_RLOGIN            2
#define CMD_CONNECT_TELNET            3
#define CMD_CONNECT_TN3270            4
#define CMD_DISABLE                   5
#define CMD_ENABLE                    6
#define CMD_END                       7         /* Analogous to exit, but is NVGEN'ed */
#define CMD_EXIT                      8
#define CMD_QUIT                      9

/* Write command destinations */
#define WRITE_UNUSED 0
#define WRITE_CORE 1
#define WRITE_ERASE 2
#define WRITE_MEMORY 3
#define WRITE_NETWORK 4
#define WRITE_TERMINAL 5



/* BEGIN FROM parser_defs_flash.h */

/* Copy command defines */
/* Source and destinations for copy command */
#define COPY_UNUSED                     0
#define COPY_FLASH_TFTP                 1
#define COPY_TFTP_FLASH                 2
#define COPY_TFTP_BOOTFLASH             3
#define COPY_BOOTFLASH_TFTP             4
#define COPY_RUNNING_CONFIG_TFTP        5
#define COPY_STARTUP_CONFIG_TFTP        6
#define COPY_TFTP_RUNNING_CONFIG        7
#define COPY_TFTP_STARTUP_CONFIG        8
#define COPY_FFFFFFFF_FLASH             9   /* OBSOLETE */
#define COPY_VERIFY_FLASH               10
#define COPY_SSE_MEMORY             11
#define COPY_ERASE_FLASH                12
#define COPY_ERASE_BOOTFLASH            13
#define COPY_VERIFY_BOOTFLASH           14
#define COPY_MOP_FLASH                  15
#define COPY_MOP_BOOTFLASH              16
#define COPY_FLASH_RCP                  17
#define COPY_RCP_FLASH                  18
#define COPY_RCP_BOOTFLASH              19
#define COPY_BOOTFLASH_RCP              20
#define COPY_RUNNING_CONFIG_RCP         21
#define COPY_STARTUP_CONFIG_RCP         22
#define COPY_RCP_RUNNING_CONFIG         23
#define COPY_RCP_STARTUP_CONFIG         24
#define COPY_RUNNING_CONFIG             25
#define COPY_STARTUP_CONFIG             26
#define COPY_TFTP           27
#define COPY_RCP            28
#define COPY_FLHLOG         29
#define COPY_RUNNING_CONFIG_DISK 30
#define COPY_STARTUP_CONFIG_DISK 31
#define COPY_DISK_STARTUP_CONFIG 32
#define COPY_DISK_FLASH 33
#define COPY_FLASH_DISK 34
#define COPY_DISK_TFTP  35
#define COPY_TFTP_DISK  46
#define COPY_DISK_DISK  47
#define COPY_TECHSUPPORT_DISK  48
#define COPY_TECHSUPPORT_TFTP  49

#define COPY_DISK_BIOS                                          50

/* END FROM parser_defs_flash.h */


#define DEFAULT_PING_PACKETS 5

#define SHOW_MEM_TERSE       1
#define SHOW_MEM_FREE        2
#define SHOW_MEM_PAGES       3

