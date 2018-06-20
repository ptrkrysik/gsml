#ifndef __DEBUG_H__
#define __DEBUG_H__



#define MIN_DEBUG_LEVEL 1
#define MAX_DEBUG_LEVEL 5

/*
 * debug_item_type defines a standard way of describing a debugging
 * flag. Most protocols keep their debugging flags in special
 * cells, which are pointed to by the members of arrays of entries
 * of this type. The generic debugging support provides a few routines
 * for dealing with such arrays. By convention, the end of an array
 * is flagged by a null flag pointer.
 */
typedef struct debug_item_type_ {
    boolean *var;               /* Address of on/off flag */
    char *txt;          /* Descriptive text about what to debug */
} debug_item_type;

/* Initialize the debugging system */
extern void debug_init(void);

/*
 * Determine whether "debug all" is in effect. When a new subsystem
 * is initialized, it uses this routine to determine whether it should
 * immediately turn on its debugging flags or not.
 */
extern boolean debug_all_p(void);

/*
 * Generic routine for "debug" parse chains to call. Only useful if you're
 * using totally stock flags and arrays.
 */
extern void debug_command(parseinfo *csb);

/* Routine to handle "debug all" and "undebug all" */
extern void debug_all_cmd(parseinfo *csb);

/* Routine that implements the "show debug" command */
extern void show_debug(void);

/*
 * Display the value of a debugging flag in the common format used for
 * all such flags. 
 */
extern void debug_show_flag(boolean flag, char *text);

/*
 * Show a whole array of flags. The end of the array is marked by an
 * entry with a null flag pointer.
 */
extern void debug_show_flags(const debug_item_type *array, const char *heading, 
                             boolean printLevel );

/*
 * generic routine to call when anybody issues
 * a "debug all" or "undebug all" command or
 * whenever you want to set the state of all
 * debug flags at once.
 */

extern void generic_debug_all (const debug_item_type *, boolean );
extern void generic_debug_show (const debug_item_type *, const char* );
extern STATUS generic_debug_init (const debug_item_type *, const char* );

extern void debug_init( void );

#endif __DEBUG_H__
