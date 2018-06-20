#ifndef __PARSER_PRIVILEGE_H__
#define __PARSER_PRIVILEGE_H__

typedef struct privq_ {
    struct privq_ *next;                /* Next queue entry */
    trans_priv *priv;                   /* Priv pointer */
} privq;

typedef struct priv_nvgenq_ {
    struct priv_nvgenq_ *next;          /* Next queue entry */
    uint level;                         /* Priv level */
    char command[PARSEBUF];             /* Current command */
} priv_nvgenq;


extern void priv_push(parseinfo *, trans_priv *);
extern boolean nvgen_privilege (parseinfo *, trans_priv *, const char *);

#endif /*  __PARSER_PRIVILEGE_H__ */
