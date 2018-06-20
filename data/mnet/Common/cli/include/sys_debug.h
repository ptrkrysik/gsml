#ifndef  __SYS_DEBUG_H
#define  __SYS_DEBUG_H

typedef void (*DEBUG_ALL_FUNCTION_PTR)(boolean);
typedef void (*DEBUG_SHOW_FUNCTION_PTR)(void);

typedef struct { 
        NODE debugNode;
        void *debugFunction;
        debug_item_type *debug_items;
        char *heading;
} tDEBUG_FUNC_NODE;

extern void sys_debug_init(void);
extern void invoke_debug_all(boolean);
extern void invoke_debug_show(void);
extern STATUS add_debug_all(DEBUG_ALL_FUNCTION_PTR, const debug_item_type*);
extern STATUS add_debug_show(DEBUG_SHOW_FUNCTION_PTR, const debug_item_type*,
                             const char*);
#endif __SYS_DEBUG_H
