#ifndef __MACROS_H
#define __MACROS_H

/*
 *  Common help strings
 */
extern const char parser_help_default_keyword[];
extern const char parser_help_no_keyword[];
extern const char parser_default_keyword[];
extern const char parser_no_keyword[];


#ifdef  BOOT
#define PHELP(str)      NULL
#else   BOOT
#define PHELP(str)      str
#endif  BOOT

#define OBJ(type,num) PTYPE_ ## type ## ( ## num ## )

#define pfdecl(x) extern void x (parseinfo *)

#define TRANS_PRIV(name, priv)\
    static trans_priv P ## name =\
        {\
            (priv),\
            (priv),\
            TRANS_PRIV_FLAGS_SET(priv),\
            0\
        }


#define NOP(name, acc, alt)\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func) NOP_action, NULL}

#define LINK_POINT(name, acc)\
    static dynamic_transition pname(name) =\
    {\
        (dynamic_transition *)&pname(acc),\
        (dynamic_transition *)&pname(link_point),\
        (trans_func) NOP_action,\
        NULL\
    }

#define LINK_TRANS(name, acc)\
    static dynamic_transition pname(L ## name) =\
    {\
        (dynamic_transition *)&pname(NONE),\
        (dynamic_transition *)&pname(link_trans),\
        (trans_func) NOP_action,\
        NULL\
    };\
    static transition pname(name) = {(transition *) &pname(L ## name),\
                              &pname(acc),\
                              (trans_func) NOP_action, NULL}

#define LINK_EXIT(name, acc)\
    LINK_TRANS(name, acc)

#define NVGENS(name, acc, func, subfunc)\
  pfdecl(func);\
  static nvgens_struct L ## name = {func, subfunc};\
  static transition pname(name) = {&pname(acc), NULL,\
                            (trans_func) nvgens_action, & L ## name}

#define NVGENNS(name, acc, func)\
    pfdecl(func);\
    static transition pname(name) =\
    {\
        &pname(acc),\
        NULL,\
        (trans_func) nvgenns_action,\
        func\
    }

#define EOLNS(name,func)\
    pfdecl(func);\
    static transition pname(name) =\
    {\
        NULL,\
        NULL,\
        (trans_func) eolns_action,\
        func\
    }

#define EOLS(name,func,subfunc)\
  pfdecl(func);\
  static eols_struct L ## name = {func, subfunc};\
  static transition pname(name) = {NULL, NULL, (trans_func) eols_action, & L ## name}

#define EOLI(name,func,subfunc)\
  pfdecl(func);\
  static eols_struct L ## name = {func, subfunc};\
  static transition pname(name) = {NULL, NULL, (trans_func) eoli_action, & L ## name}

#define HELP(name,alt,str)\
  static help_struct L ## name = {PHELP(str), NULL};\
  static transition pname(name) = {NULL, &pname(alt), (trans_func) help_action, & L ## name}

#define GENERAL_STRING(name, acc, alt, str, help, flags)\
    static general_string_struct L ## name =\
    {\
        CSBOFF(str),\
        PHELP(help),\
        flags\
    };\
    static transition pname(name) =\
    {\
        &pname(acc),\
        &pname(alt),\
        (trans_func) general_string_action,\
        & L ## name\
    }

/* A string delimited by whitespace. */
#define STRING(name, acc, alt, str, help)\
    GENERAL_STRING(name, acc, alt, str, help,\
                   STRING_QUOTED | STRING_HELP_CHECK)

/* A string delimited by EOL. */
#define TEXT(name, acc, alt, str, help)\
    GENERAL_STRING(name, acc, alt, str, help,\
                   STRING_WS_OK | STRING_HELP_CHECK)

/*
 * Testing Macros
 */

/* ASSERT - assert that a C expression is true */

#define TEST_EXPR(name, true, false, alt, expr)\
    static boolean F ## name (parseinfo *csb, transition *mine)\
    {\
        return((expr) != 0);\
    }\
    static testexpr_struct L ## name = {&pname(false), F ## name};\
    static transition pname(name) = {&pname(true), &pname(alt),\
                              (trans_func) testexpr_action, & L ## name}

#define ASSERT(name, acc, alt, expr)\
    TEST_EXPR(name, acc, NONE, alt, expr)

#define IFELSE(name, if, else, expr)\
    TEST_EXPR(name, if, else, NONE, expr)

/* EVAL - always accept, and evaluate a C expression */

#define EVAL(name,acc,expr)\
  static void F ## name (parseinfo *csb, transition *mine)\
  { expr;push_node(csb, mine->accept); }\
  static transition pname(name) = {&pname(acc), NULL, (trans_func) F ## name, NULL}

/* TESTVAR - test a variable relative to a constant value */

#define TESTVAR(name, equal, notequal, lt, gt, alt, var, val)\
  static test_struct L ## name = {CSBOFF(var), (uint) val,\
                           &pname(equal), &pname(notequal),\
                           &pname(lt), &pname(gt)};\
  static transition pname(name) = {NULL, &pname(alt),\
                            (trans_func) test_action, & L ## name}


/* Relations simpler than TESTVAR in its entirity */

#define EQUAL(name, acc, alt, var, val)\
  TESTVAR(name, acc, NONE, NONE, NONE, alt, var, val)
#define NEQ(name, acc, alt, var, val)\
  TESTVAR(name, NONE, acc, NONE, NONE, alt, var, val)
#define SPLIT(name, acc, alt, var, val)\
  TESTVAR(name, acc, alt, NONE, NONE, NONE, var, val)

/* SET - always accept, setting a variable to a constant value */

#define SET(name, acc, var, val)\
    static set_struct L ## name = {CSBOFF(var), (uint) val};\
    static transition pname(name) = {&pname(acc), NULL,\
                              (trans_func) set_action, & L ## name}

/*
 * KEYWORD is a complete token, with help, error handling and whitespace.o
 * KEYWORD_OPTWS has optional trailing white-space.
 * KEYWORD_NOWS has no trailing white-space.
 * KEYWORD_MM takes an extra argument for the minimum number of chars required
 *  to match.
 */

#define GENERAL_KEYWORD(name,acc,alt,str,help,priv,var,val,match,flags)\
    TRANS_PRIV(name, priv);\
    static general_keyword_struct L ## name = {str, PHELP(help), & P ## name,\
                                            var, val, match, flags};\
    static transition pname(name) = {&pname(acc),&pname(alt),\
                              (trans_func) general_keyword_action, & L ## name}

#define KEYWORD(name, acc, alt, str, help, priv)\
    TRANS_PRIV(name, priv);\
    static keyword_struct L ## name = {str, PHELP(help), & P ## name};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func) keyword_action, & L ## name}

#define KEYWORD_OPTWS(name,acc,alt,str,help, priv)\
    TRANS_PRIV(name, priv);\
    static keyword_struct L ## name = {str, PHELP(help), & P ## name};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func) keyword_optws_action, & L ## name}

#define KEYWORD_OPTIONS(name, acc, alt, options, var, priv, flags)\
    TRANS_PRIV(name, priv);\
    static keyword_option_struct L ## name =\
    {\
        options,\
        CSBOFF(var),\
        & P ## name,\
        flags\
    };\
    static transition pname(name) =\
    {\
        &pname(acc),\
        &pname(alt),\
        (trans_func) keyword_option_action,\
        & L ## name\
    }

#define KEYWORD_MM(name,acc,alt,str,help, priv, count)\
    TRANS_PRIV(name, priv);\
    static keyword_mm_struct L ## name = {str, PHELP(help), & P ## name, count};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func) keyword_mm_action, & L ## name}

#define KEYWORD_ID(name, acc, alt, var, val, str, help, priv)\
    TRANS_PRIV(name, priv);\
    static keyword_id_struct L ## name = {str,PHELP(help),&P##name,CSBOFF(var),val};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func)  keyword_id_action, & L ## name}
/*
 * If parse keyword
 *    var |= val
 */
#define KEYWORD_OR(name, acc, alt, var, val, str, help, priv)\
    TRANS_PRIV(name, priv);\
    static keyword_id_struct L ## name = {str,PHELP(help),&P##name,CSBOFF(var),val};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func)  keyword_orset_action, & L ## name}

/*
 * Number macros
 */

#define GENERAL_NUMBER(name,acc,alt,var,lower,upper,help,flags)\
    static number_struct L ## name = {CSBOFF(var), lower, upper, PHELP(help), flags};\
    static transition pname(name) =\
      {&pname(acc), &pname(alt), (trans_func) general_number_action, & L##name}

#define NUMBER(name, acc, alt, var, lower, upper, help)\
    GENERAL_NUMBER(name, acc, alt, var, lower, upper, help,\
                   (NUMBER_HEX | HEX_ZEROX_OK | NUMBER_OCT | OCT_ZERO_OK |\
                    NUMBER_DEC | NUMBER_WS_OK | NUMBER_HELP_CHECK))

#define DECIMAL(name, acc, alt, var, lower, upper, help)\
    GENERAL_NUMBER(name, acc, alt, var, lower, upper, help,\
                   (NUMBER_DEC | NUMBER_WS_OK | NUMBER_HELP_CHECK))

/* HEXADECIMAL matches an unsigned hex number, minimum one
 * digit, and assigns it to a int.  This function works on numbers
 * which don't begin with '0x'. */

#define HEXADECIMAL(name,acc,alt,var)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, NULL,\
    (NUMBER_HEX | HEX_NO_ZEROX_OK | NUMBER_NO_WS_OK))

#define HEXNUM(name, acc, alt, var, help)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, help,\
        (NUMBER_HEX | HEX_NO_ZEROX_OK | NUMBER_WS_OK | NUMBER_HELP_CHECK))


/*
 * This is an OCTAL number, with help, error handling and
 * whitespace
 */
#define OCTAL(name,acc,alt,var,help)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, help,\
   (NUMBER_OCT | OCT_ZERO_OK | OCT_NO_ZERO_OK |\
    NUMBER_WS_OK | NUMBER_HELP_CHECK))

#define SIGNED_DEC(name,acc,alt,var,lower,upper,help)\
    static signed_number_struct L ## name = {CSBOFF(var), lower, upper, PHELP(help)};\
    static transition pname(name) =\
      {&pname(acc), &pname(alt), (trans_func) signed_number_action, & L##name}

/* NOPREFIX
 * Go to 'accept' if there isn't a 'no' prefix.
 * Consume the remainder of the input and go to 'alt' if there
 * was a 'no' prefix.  This is somewhat backwards from other macros
 * in that the major action takes place before going to the alternate
 * state, but it keeps the major thread of the command token chains
 * similar to other tokens.
 */
#define NOPREFIX(name, acc, alt)\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                            (trans_func) noprefix_action, NULL}


/* IPADDR gets an IP address and stores it in addr.
 * IPADDR_NAME gets an IP address or hostname and 
 * converts to an address stored in addr.
 */
#define IPADDR_NAME(name, acc, alt, addr, help)\
    static addrparse_struct L ## name = {CSBOFF(addr), PHELP(help), ADDR_IP,\
                                    ADDR_NAME_LOOKUP};\
    static transition pname(name) = {& pname(acc), & pname(alt),\
                                     (trans_func) generic_addr_action, & L ## name}

#define IPADDR(name,acc,alt,addr, help)\
    static addrparse_nf_struct L ## name = {CSBOFF(addr), PHELP(help), ADDR_IP};\
    static transition pname(name) = {& pname(acc), & pname(alt),\
                                   (trans_func) generic_addr_noflag_action, & L ## name}


/* DAY - A day of the week */

#define DAY(name,acc,alt,var,help) \
    static day_struct L ## name = {CSBOFF(var), PHELP(help)};\
    static transition pname(name) =\
  {&pname(acc), &pname(alt), (trans_func) day_action, & L##name}

/* MONTH - A month of the year */

#define MONTH(name,acc,alt,var,help) \
    static month_struct L ## name = {CSBOFF(var), PHELP(help)};\
    static transition pname(name) =\
  {&pname(acc), &pname(alt), (trans_func) month_action, & L##name}

#define GENERAL_TIME(name, acc, alt, var1, var2, var3, help)\
    static timesec_struct L ## name =\
    {\
        var1,\
        var2,\
        var3,\
        help\
    };\
    static transition pname(name) =\
    {\
        &pname(acc),\
        &pname(alt),\
        (trans_func) timesec_action,\
        & L ## name\
    }

/* TIME - A time (24 hours, without seconds) */
#define ATIME(name, acc, alt, var1, var2, help)\
    GENERAL_TIME(name, acc, alt, CSBOFF(var1),\
                 CSBOFF(var2), MAXUINT, help)

/* TIMESEC - A time (24 hours, with seconds) */
#define TIMESEC(name, acc, alt, var1, var2, var3, help)\
    GENERAL_TIME(name, acc, alt, CSBOFF(var1),\
                 CSBOFF(var2), CSBOFF(var3), help)

#define TEST_MULTIPLE_FUNCS(name, equal, notequal, alt)\
    static test_struct L ## name = {0, 0, &pname(equal), &pname(notequal), NULL, NULL};\
    static transition pname(name) = {NULL, &pname(alt),\
                            (trans_func) multiple_funcs_action, & L ## name}

#define PRIV_TEST(name, acc, fail, alt, priv)\
    TRANS_PRIV(name, priv);\
    static priv_struct L ## name = {& P ## name, &pname(fail)};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func) priv_action, & L ## name}

#define NVCOMMENT(name, acc)\
    static transition pname(name) =\
    {\
        &pname(acc),\
        &pname(NONE),\
        (trans_func) comment_action,\
        NULL\
    }

#define NO_OR_DEFAULT(name, acc, flags) \
   SET       (name ## setsense, acc, sense, FALSE); \
   KEYWORD   (name, name ## setsense, acc,  \
              parser_no_keyword, parser_help_no_keyword, flags);



#define IPADDR_MASK(name, acc, alt, addr, mask, help)\
    static ipaddrmask_struct L ## name = { CSBOFF(addr), CSBOFF(mask), help};\
    static transition pname(name) = {& pname(acc), & pname(alt),\
                              (trans_func) ipaddrmask_action, & L ## name}

#define IPMASK(name,acc,alt,mask, help)\
    static ipmask_struct L ## name = {CSBOFF(mask), PHELP(help)};\
    static transition pname(name) = {& pname(acc), & pname(alt),\
                                   (trans_func) ipmask_action, & L ## name}

#define FUNC(name, acc, func)\
  pfdecl(func);\
  static transition pname(name) = {&pname(acc), NULL, (trans_func) func_action, func}

#ifdef IOS_PARSER

#define WHITESPACE(name,acc,alt)\
  static transition pname(name) = {&pname(acc), &pname(alt),\
                                (trans_func) whitespace_action, NULL}

#define PRINT(name,acc,str)\
  static print_struct L ## name = {str};\
  static transition pname(name) = {&pname(acc), NULL, (trans_func) print_action, & L ## name}

#define DISPLAY(name,acc,str)\
  static help_struct L ## name = {PHELP(str), NULL};\
  static transition pname(name) = {NULL, &pname(acc),\
                                (trans_func) display_action, & L ## name}

#define VAR_STRING(name, acc, alt, str)  \
    GENERAL_STRING(name, acc, alt, str, NULL, STRING_QUOTED)

#define CHARACTER(name,acc,alt,character)\
  static char_struct L ## name = {character};\
  static transition pname(name) = {&pname(acc), &pname(alt),\
                            (trans_func) char_action, & L ## name}

#define CHAR8_NUMBER(name, acc, alt, var, help)\
  static number_struct L ## name = {CSBOFF(var), 0, 255, PHELP(help), 0};\
  static transition pname(name) = \
        {&pname(acc), &pname(alt), (trans_func) char_number_action, & L ## name}

#define FUNC(name, acc, func)\
  pfdecl(func);\
  static transition pname(name) = {&pname(acc), NULL, (trans_func) func_action, func}


#define TXT_LINE(name, acc, alt, str)\
    GENERAL_STRING(name, acc, alt, str, NULL, STRING_WS_OK)

#define CHAR_NUMBER(name, acc, alt, var, help)\
  static number_struct L ## name = {CSBOFF(var), 0, 127, PHELP(help), 0};\
  static transition pname(name) = \
        {&pname(acc), &pname(alt), (trans_func) char_number_action, & L ## name}

#define TEST_BOOLEAN(name, nonzero, zero, alt, var)\
    static test_bool_struct L ## name = {&var, &pname(zero), &pname(nonzero)};\
    static transition pname(name) = {NULL, &pname(alt),\
                              (trans_func) test_boolean_action, & L ## name}

#define TEST_INT(name, nonzero, zero, alt, var)\
    static test_int_struct L ## name = {&var, &pname(zero), &pname(nonzero)};\
    static transition pname(name) = {NULL, &pname(alt),\
                              (trans_func) test_int_action, & L ## name}

#define TEST_FUNC(name, nonzero, zero, alt, func)\
    static test_func_struct L ## name = {func, &pname(zero), &pname(nonzero)};\
    static transition pname(name) = {NULL, &pname(alt),\
                              (trans_func) test_func_action, & L ## name}

#define TEST_IDBSTATUS(name, true, false, alt, val)\
    static test_idbstatus_struct L ## name = {&pname(false), val};\
    static transition pname(name) = {&pname(true), &pname(alt),\
                              (trans_func) test_idbstatus_action, & L ## name}

#define KEYWORD_DEBUG(name, acc, alt, var, val, str, help, priv)\
    KEYWORD_ID(name, acc, alt, var, (uint) val, str, help, priv)

#define KEYWORD_ID_MM(name, acc, alt, var, val, str, help, priv, count)\
    TRANS_PRIV(name, priv);\
    static keyword_id_mm_struct L ## name = {str,PHELP(help),&P##name,CSBOFF(var),val,count};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func)  keyword_id_mm_action, & L ## name}

#define KEYWORD_TRANS(name, link, acc, alt, str, help, priv)\
    LINK_TRANS(link, acc);\
    TRANS_PRIV(name, priv);\
    static keyword_struct L ## name =\
    {\
        str,\
        PHELP(help),\
        & P ## name\
    };\
    static transition pname(name) =\
    {\
        &pname(link),\
        &pname(alt),\
        (trans_func) keyword_trans_action,\
        & L ## name\
    }

/*
 * If !(var & val) and parse keyword
 *     var |= val
 */
#define KEYWORD_ORTEST(name, acc, alt, var, val, str, help, priv)\
    TRANS_PRIV(name, priv);\
    static keyword_id_struct L ## name = {str,PHELP(help),&P##name,CSBOFF(var),val};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func)  keyword_ortest_action, & L ## name}

#define KEYWORD_OPTIONS(name, acc, alt, options, var, priv, flags)\
    TRANS_PRIV(name, priv);\
    static keyword_option_struct L ## name =\
    {\
        options,\
        CSBOFF(var),\
        & P ## name,\
        flags\
    };\
    static transition pname(name) =\
    {\
        &pname(acc),\
        &pname(alt),\
        (trans_func) keyword_option_action,\
        & L ## name\
    }

#define KEYWORD_NOWS(name,acc,alt,str,help, priv)\
    TRANS_PRIV(name, priv);\
    static keyword_struct L ## name = {str, PHELP(help), & P ## name};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                              (trans_func) keyword_nows_action,  & L ## name}


#define PERMIT_DENY(name, acc, alt, var, priv)\
    KEYWORD_OPTIONS(name, acc, alt, permitdeny_options, var, priv, 0);

#define NUMBER_NV(name, acc, alt, var, lower, upper, help)\
    GENERAL_NUMBER(name, acc, alt, var, lower, upper, help,\
                   (NUMBER_HEX | HEX_ZEROX_OK | NUMBER_OCT | OCT_ZERO_OK |\
                    NUMBER_DEC | NUMBER_WS_OK | NUMBER_HELP_CHECK |\
                    NUMBER_NV_RANGE))

/* Handle a decimal number, without help and whitespace. */
#define IDECIMAL(name,acc,alt,var,lower,upper)\
    GENERAL_NUMBER(name, acc, alt, var, lower, upper, NULL,\
    (NUMBER_DEC | NUMBER_NO_WS_OK))

/* Handle a decimal number, without help and whitespace. */
#define INUMBER(name,acc,alt,var,lower,upper)\
    GENERAL_NUMBER(name, acc, alt, var, lower, upper, NULL,\
    (NUMBER_HEX | HEX_ZEROX_OK | NUMBER_OCT | OCT_ZERO_OK |\
     NUMBER_DEC | NUMBER_NO_WS_OK))

/* HEXADECIMAL matches an unsigned hex number, minimum one
 * digit, and assigns it to a int.  This function works on numbers
 * which don't begin with '0x'. */

#define HEXADECIMAL(name,acc,alt,var)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, NULL,\
    (NUMBER_HEX | HEX_NO_ZEROX_OK | NUMBER_NO_WS_OK))

#define HEXNUM(name, acc, alt, var, help)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, help,\
        (NUMBER_HEX | HEX_NO_ZEROX_OK | NUMBER_WS_OK | NUMBER_HELP_CHECK))

#define HEXDIGIT(name, acc, alt, var, lower, upper, help)\
    GENERAL_NUMBER(name, acc, alt, var, lower, upper, help,\
        (NUMBER_HEX | HEX_NO_ZEROX_OK | NUMBER_WS_OK | NUMBER_HELP_CHECK))

#define OPT_HEXNUM(name, acc, alt, var, help)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, help,\
       (NUMBER_HEX | HEX_NO_ZEROX_OK | HEX_ZEROX_OK |\
        NUMBER_WS_OK | NUMBER_HELP_CHECK))

#define OPT_HEXDIGIT(name, acc, alt, var, lower, upper, help)\
    GENERAL_NUMBER(name, acc, alt, var, lower, upper, help,\
       (NUMBER_HEX | HEX_NO_ZEROX_OK | HEX_ZEROX_OK |\
        NUMBER_WS_OK | NUMBER_HELP_CHECK))

#define ZEROXHEXNUM(name, acc, alt, var, help)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, help,\
       (NUMBER_HEX | HEX_ZEROX_OK | NUMBER_WS_OK | NUMBER_HELP_CHECK))

/* 
 * IOCTAL matches an unsigned octal number, minimum one
 * digit, and assigns it to a int
 */
#define IOCTAL(name,acc,alt,var)\
    GENERAL_NUMBER(name, acc, alt, var, 0, -1, NULL,\
    (NUMBER_OCT | OCT_ZERO_OK | OCT_NO_ZERO_OK |\
     NUMBER_NO_WS_OK))

#define NUMBER_FUNC(name, acc, alt, var, func, help)\
    static number_func_struct L ## name =\
      {CSBOFF(var), func, PHELP(help)};\
    static transition pname(name) =\
      {&pname(acc), &pname(alt), (trans_func) number_func_action, & L ## name}

#define NUMBER_HELP_FUNC(name, acc, alt, var, func)\
    static number_help_func_struct L ## name =\
      {CSBOFF(var), func};\
    static transition pname(name) =\
      {&pname(acc), &pname(alt), (trans_func) number_help_func_action, & L ## name} 


#define GENERAL_PARAMS(name, alt, keyword, var, lower, upper, func, subfunc, keyhelp, varhelp, priv, flags)\
    pfdecl(func);\
    TRANS_PRIV(name, priv);\
    static params_struct L ## name = {keyword, CSBOFF(var), lower, upper,\
                             func, subfunc, PHELP(keyhelp),\
                             PHELP(varhelp),\
                             & P ## name, flags};\
    static transition pname(name) = {NULL, & pname(alt),\
                                (trans_func)  params_action, & L ## name}

#define PARAMS(name, alt, keyword, var, lower, upper, func, subfunc, keyhelp,varhelp,priv)\
    GENERAL_PARAMS(name, alt, keyword, var, lower, upper, func,\
                   subfunc, keyhelp, varhelp, priv,\
                   (KEYWORD_WS_OK | KEYWORD_HELP_CHECK | NUMBER_DEC |\
                    NUMBER_WS_OK | NUMBER_HELP_CHECK))

#define PARAMS_KEYONLY(name, alt, keyword, var, lower, upper, func, subfunc, keyhelp, varhelp, priv)\
    GENERAL_PARAMS(name, alt, keyword, var, lower, upper, func,\
                   subfunc, keyhelp, varhelp, priv,\
                   (KEYWORD_WS_OK | KEYWORD_HELP_CHECK | NUMBER_DEC |\
                    NUMBER_WS_OK | NUMBER_HELP_CHECK | NO_NUMBER))

#define IPMASK(name,acc,alt,mask, help)\
    static ipmask_struct L ## name = {CSBOFF(mask), PHELP(help)};\
    static transition pname(name) = {& pname(acc), & pname(alt),\
                                   (trans_func) ipmask_action, & L ## name}

#define IPADDR_MASK(name, acc, alt, addr, mask, help)\
    static ipaddrmask_struct L ## name = { CSBOFF(addr), CSBOFF(mask), help};\
    static transition pname(name) = {& pname(acc), & pname(alt),\
                              (trans_func) ipaddrmask_action, & L ## name}

/* Negative 1 (-1), used as the wild-card in many protocols */
#define NEG1(name, acc, alt, net, help)\
    KEYWORD_ID(name, acc, alt, net, -1L, "-1", help, PRIV_MIN)


#define ALT_HELPS(name,func,subfunc)\
  pfdecl(func);\
  static eols_struct L ## name = {func, subfunc};\
  static transition pname(name) = {NULL, NULL, (trans_func) alt_helps_action, & L ## name}

/*
 * Hexadecimal data stream.  Used in 'llc2 send' command.
 */

#define HEXDATA(name, acc, alt, buf, count, help, bufmax)\
    static hexdata_struct L ## name = {CSBOFF(buf), CSBOFF(count),\
                                  PHELP(help), bufmax};\
    static transition pname(name) = {&pname(acc), &pname(alt),\
                                       (trans_func) hexdata_action, & L ## name}

#define KEYWORD_NEG(name,acc, alt,str,help, priv)\
     SET(name ## setsense, acc , sense, FALSE); \
     KEYWORD(name, name ## setsense, alt, str, help, priv)


#endif /* IOS_PARSER */

#endif /* __MACROS_H__ */
