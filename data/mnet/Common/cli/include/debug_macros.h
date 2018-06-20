

/* This perverted abuse of the C preprocessor is designed to keep us from
 * having to keep too many files consistent to make debugging DEBUG_FLAGs work.
 * To make a standard debugging DEBUG_FLAG, we have to create 3 things:
 *
 * o A boolean flag variable (tested by the routines that print the actual
 *   debugging output)
 *
 * o An array element that associates that variable with a textual
 *   description (used by the parsing and printing routines to talk about
 *   the flag)
 *
 * o A symbolic name for the array index (used by parse chains to designate
 *   which flag is to be set or cleared within an array).
 *
 * Rather than ask a human being to type in all three definitions, we use
 * the macros below and some include file hackery to create them all from
 * the same data. If the magic symbol "__DECLARE_DEBUG_ARR__" is defined,
 * we assume that whatever file includes us wants to actually declare
 * initialized data... presumably we've been included by an xxx_debug.c
 * module. If "__DECLARE_DEBUG_NUMS__" is defined instead, we assume
 * that an include file wants to create the symbolic names for array
 * offsets. If neither magic symbol is defined, we assume that an include
 * file (probably the same one) wants to create external declarations
 * for the booleans themselves, as well as an external declaration for
 * the array.
 *
 * There are some instances in which a debug flag is defined that should
 * not be declared by us;  in such cases the DEBUG_FLAG_NO_DECL macro
 * should be used, rather than the DEBUG_FLAG macro.
 */

/* We've probably been called before, so let's clean up what we did then */
#undef DEBUG_ARRDECL
#undef DEBUG_FLAG
#undef DEBUG_ARRDONE
#undef DEBUG_FLAG_NO_DECL

#if defined(__DECLARE_DEBUG_ARR__)

#define DEBUG_ARRDECL(arrname) const debug_item_type arrname[] = {
#define DEBUG_FLAG(varname,codename,textname) {&varname, textname},
#define DEBUG_FLAG_NO_DECL(varname,codename,textname) {&varname, textname},
#define DEBUG_ARRDONE {(boolean *) NULL, (char *) NULL} };

#elif defined(__DECLARE_DEBUG_NUMS__)

#define DEBUG_ARRDECL(arrname) enum { 
#define DEBUG_FLAG(varname,codename,textname) codename,
#define DEBUG_FLAG_NO_DECL(varname,codename,textname) codename,
#define DEBUG_ARRDONE };

#else

#define DEBUG_ARRDECL(arrname) extern const debug_item_type arrname[];
#if defined(__DECLARE_DEBUG_VARS__)
#define DEBUG_FLAG(varname,codename,textname) boolean varname;
#else
#define DEBUG_FLAG(varname,codename,textname) extern boolean varname;
#endif
#define DEBUG_FLAG_NO_DECL(varname,codename,textname) /* Nothing */
#define DEBUG_ARRDONE /* Nothing */

#endif

/* Some other include file might want to reuse us... */
#undef __DECLARE_DEBUG_VARS__
#undef __DECLARE_DEBUG_ARR__
#undef __DECLARE_DEBUG_NUMS__
