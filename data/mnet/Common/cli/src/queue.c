#include <vxworks.h>
#include <stdioLib.h>
#include <tyLib.h>
#include <taskLib.h>
#include <errnoLib.h>
#include <lstLib.h>
#include <inetLib.h>    /* for INET_ADDR_LEN */
#include <timers.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <msgQLib.h>
#include <sigLib.h>


#include <../include/cli_parser_private.h>
#include <../include/pstypes.h>
#include <../include/psvxif.h>
#include <../include/imsg.h>
#include <../include/imc.h>
#include <../include/address.h>
#include <../include/parsertypes.h>
#include <../include/config.h>
#include <../include/parser_sim.h>
#include <../include/parser.h>

boolean queueempty_inline (queuetype *q)
{
    return(q->qhead == NULL);
}


/*
 * enqueue_inline - add an element to a fifo queue.
 */

void enqueue_inline (queuetype *qptr, void *eaddr)
{
    nexthelper *p, *ptr;

    p = qptr->qtail;                                    /* last element pointer */
    ptr = eaddr;

    /*
     * Make sure the element isn't already queued or the last
     * element in this list
     */
    if ((ptr->next != NULL) || (p == ptr)) {
                perror("enqueue_inline:Error\n");
                return;
    }
    if (!p)                                                             /* q empty */
                qptr->qhead = ptr;
    else                                                                /* not empty */
                p->next = ptr;                                  /* update link */
    qptr->qtail = ptr;                                  /* tail points to new element */
    qptr->count++;
}

/*
 * dequeue_inline - remove first element of a fifo queue.
 */

static void *dequeue_inline (queuetype *qptr)
{
    nexthelper *p;

    
    if (qptr == NULL)
                return(NULL);
    p = qptr->qhead;                    /* points to head of queue */
    if (p) {                                                    /* is there a list? */
                qptr->qhead = p->next;                  /* next link */
                if (!p->next)
                        qptr->qtail = NULL;                     /* this was last guy, so zap tail */
                p->next = NULL;                                 /* clear link, just in case */
    }
    if (p && (--qptr->count < 0) && qptr->maximum) {
                perror("dequeue_inline:Error\n");
                /* (*kernel_errmsg)(&msgsym(QCOUNT, SYS), "dequeue", qptr,
                   qptr->count);
                   */

                qptr->count = 0;
    }
    return(p);
}

/*
 * requeue_inline - add an element to the *head* of a "fifo" queue.
 */

static void requeue_inline (queuetype *qptr, void *eaddr)
{
    nexthelper *ptr;

    ptr = eaddr;
    if (ptr->next != NULL) {
                perror("requeue:Error\n");
                return;
    }
    if (qptr->qhead == NULL)                    /* if list is empty */
                qptr->qtail = ptr;                              /* update tail pointer */
    else
                ptr->next = qptr->qhead;                /* new head points to old head */
    qptr->qhead = ptr;                                  /* list head points to new head */
    qptr->count++;
}

/*
 * queue_init
 * Initialize a queuetype
 */

void queue_init (queuetype *q, int maximum)
{
    q->qhead = NULL;
    q->qtail = NULL;
    q->count = 0;
    q->maximum = maximum;
}

/*
 * enqueue - add an element to a fifo queue.
 * Note no interrupt interlocking.
 */

void enqueue (queuetype *qptr, void *eaddr)
{
    enqueue_inline(qptr, eaddr);
}

/*
 * dequeue - remove first element of a fifo queue.
 * Note no interrupt interlocking.
 */

void *dequeue (queuetype *qptr)
{
    return(dequeue_inline(qptr));
}

/*
 * requeue - add an element to the *head* of a "fifo" queue.
 * Performs the inverse operation of dequeue()
 * Note no interrupt interlocking.
 */

void requeue (queuetype *qptr, void *eaddr)
{
    requeue_inline(qptr, eaddr);
}

/*
 * peekqueuehead -- Return address of element at head of queue.
 * Does not lock out interrupts.
 */

void *peekqueuehead (queuetype* q)
{
        nexthelper* p;

        p = (nexthelper *)q->qhead;                     /* first member */
        return(p);

}

/*
 * queryqueuedepth -- Query the length of the queue.
 * Does not lock out interrupts.
 */

int queryqueuedepth (queuetype* q)
{
        return(q->count);
}

/*
 * insqueue - add an element to the *middle* of a "fifo" queue.
 *            'eaddr' goes immediately after 'paddr'.
 *
 * If 'paddr' is NULL, requeue is called.
 * Note no interrupt interlocking.
 */

void insqueue (queuetype *qptr, void *eaddr, void *paddr)
{
    nexthelper *eptr, *pptr;

    eptr = (nexthelper *)eaddr;
    pptr = (nexthelper *)paddr;
    if (pptr == NULL) {
                requeue(qptr, eptr);
                return;
    }
    if (eptr->next != NULL) {
                perror("insqueue:Error!\n");
                return;
    }
    eptr->next = pptr->next;
    pptr->next = eptr;
    if (eptr->next == NULL) 
                qptr->qtail = eptr;                             /* tail points to new element */
    qptr->count++;
}









#ifdef IOS_PARSER


/*
 * checkqueue
 * Return TRUE if specified element is in queue, FALSE otherwise
 * Does not lock out interrupts.
 */

boolean checkqueue (queuetype *q, void *e)
{
    nexthelper *p;

    if (q == NULL)
        return(FALSE);
    p = q->qhead;                                               /* first member */
    while (p) {
                if (p == e)                                             /* match? */
                        return(TRUE);                           /* yes, return TRUE */
                else
                        p = p->next;                            /* no, cdr the list */
    }
    return(FALSE);                                              /* not in queue, return FALSE */
}





/*
 * p_dequeue -- remove first element from a fifo queue
 * Locks out interrupts.
 */

void *p_dequeue (queuetype *qptr)
{
    void      *p;

    p = dequeue_inline(qptr);
    return (p);
}

/*
 * p_enqueue -- put an element into a fifo queue
 * Locks out interrupts.
 */

void p_enqueue (queuetype *qptr, void *eaddr)
{
    enqueue_inline(qptr, eaddr);
}

/*
 * p_requeue - add an element to the *head* of a "fifo" queue.
 * Locks out interrupts.
 */

void p_requeue (queuetype *qptr, void *eaddr)
{
    requeue_inline(qptr, eaddr);
}

/*
 * p_unqueue -- delete a specified element from a fifo queue.
 * Lock out interrupts.
 */

void p_unqueue (queuetype *q, void *e)
{
    unqueue_inline(q, e);
}

/*
 * p_swapqueue - remove an old element from a fifo queue AND add a 
 * new element into the fifo queue at the old elements' position in
 * the fifo queue.
 * Locks out interrupts.
 */
 
void p_swapqueue (queuetype *qptr, void *enew, void *eold)
{
    swapqueue_inline(qptr, enew, eold);
}


/*
 * p_unqueuenext -- delete the next element from a fifo queue.
 * Lock out interrupts.
 */

void p_unqueuenext (queuetype *q, void **prev)
{
    nexthelper *nextelement;

    
    nextelement = (nexthelper *)*prev;
    
    if (nextelement) {     /* else we're at the end of the list - do nothing */
                *prev = nextelement->next;
                nextelement->next = NULL;
                
                if (!*prev) {                                   /* prev now at the end of the list; fix the header */
                        if (prev == (void **)&q->qhead)
                                q->qtail = NULL;                /* prev is the header, now an empty list */
                        else
                                q->qtail = prev;                /* real prev , it's at the end */
                }
                if ((--q->count < 0) && (q->maximum)) {
                        /* (*kernel_errmsg)(&msgsym(QCOUNT, SYS), "p_unqueuenext",
                           q, q->count);
                           */
                        printf("p_unqueuenext:Error\n");
                        q->count = 0;
                }
                
    }
    return;
}


/*
 * unqueue -- delete a specified element from a fifo queue.
 * Does not lock out interrupts.
 */

void unqueue (queuetype *q, void *e)
{
    unqueue_inline(q, e);
}


/*
 * remqueue - remove a specified element from a fifo queue. 
 * The previous queue element is supplied (paddr) to allow a direct lift 
 * (eaddr) and repair. If paddr is NULL, the first member in queue is removed.
 * Note no interrupt interlocking.
 */

void *remqueue (queuetype *qptr, void *eaddr, void *paddr)
{

    /*
     * If previous member is null and member is at the head of queue,
     * remove from head 
     */
    if ((eaddr == qptr->qhead) && 
        (paddr == NULL)) {
                return(dequeue(qptr));
    } else {
                /*
                 * Check to make sure the previous member is really what it 
                 * claims to be or that the current member or previous member are 
                 * non-null
                 */
                if ((paddr == NULL) || (eaddr == NULL) || 
                        (((nexthelper *)paddr)->next != eaddr)){
                        /* (*kernel_errmsg)(&msgsym(LINKED, SYS), "remqueue", eaddr,
                           qptr);
                           */
                        printf("remqueue: Error!!\n");
                        return(NULL);
                }

                /* 
                 * Lift the candidate eaddr and repair pointers
                 */
                ((nexthelper *)paddr)->next = ((nexthelper *)eaddr)->next;
                ((nexthelper *)eaddr)->next = NULL;

                /* 
                 * Repair tail of queue, if necessary
                 */
                if (((nexthelper *)paddr)->next == NULL)
                        qptr->qtail = paddr;

        /*
         * Decrement queue count and do sanity check
         */
                if ((--qptr->count < 0) && qptr->maximum) {
                        /* (*kernel_errmsg)(&msgsym(QCOUNT, SYS), "remqueue", qptr,
                           qptr->count);
                           */
                        printf("remqueue:Error\n");
                        qptr->count = 0;
                }   
                return(eaddr);  
        }
}

/*
 * queueBLOCK
 * Schedule test for blocking on queue empty
 */

boolean queueBLOCK (queuetype *qptr)
{
    return(qptr->qhead ? FALSE : TRUE);
}

static boolean queuefull_reserve_inline (queuetype *q, int reserve)
{
    return((q->maximum != 0) && ((q->count + reserve) >= q->maximum));
}

static boolean queuefull_inline (queuetype *q)
{
    return((q->maximum != 0) && (q->count >= q->maximum));
}

static int queuesize_inline (queuetype *q)
{
    return(q->count);
}


static int queuemax_inline(queuetype *q)
{
        return(q->maximum);
}


/*
 * swapqueue_inline -- swap a new element for an old element on a fifo queue.
 */

static void swapqueue_inline (queuetype *q, void *enew, void *eold)
{
    nexthelper *p;
    nexthelper *prev;
    nexthelper *eoldptr;
    nexthelper *enewptr;
 
    enewptr = enew;
    eoldptr = eold;
    p = q->qhead;                                               /* first member */
    prev = (nexthelper *)q;
    while (p) {
        if (p == eoldptr) {                             /* delete this guy */
            prev->next = enewptr;               /* tie in new element */
            enewptr->next = eoldptr->next; /* keep chain intact */
            eoldptr->next = NULL;               /* clear element pointer */
                        if (q->qtail == eoldptr)        /* last element? */
                                q->qtail = enewptr;
            return;                     /* exit now */
        } else {
            prev = p;
            p = p->next;
        }
    }
    /* (*kernel_errmsg)(&msgsym(NOTQ, SYS), "swapqueue", eoldptr, q);
     */
    printf("swapqueue:Error\n");

}

/*
 * unqueue_inline -- delete a specified element from a fifo queue.
 */

static void unqueue_inline (queuetype *q, void *e)
{
    nexthelper *p;
    nexthelper *prev;
    nexthelper *eptr;

    eptr = e;
    p = q->qhead;                                               /* first member */
    prev = (nexthelper *)q;
    while (p) {
                if (p == eptr) {                                /* delete this guy */
                        prev->next = p->next;           /* keep chain intact */
                        if (!p->next) {                         /* zapped the tail */
                                if (q->qhead == 0)
                                        prev = 0;                       /* tail was head */
                                q->qtail = prev;
                        }
                        eptr->next = NULL;                      /* clear element pointer */
                        if ((--q->count < 0) && (q->maximum)) {
                                /* (*kernel_errmsg)(&msgsym(QCOUNT, SYS), "unqueue", q,
                                   q->count);
                                   */
                                perror("unqueue_inline:Error\n");

                                q->count = 0;
                        }
                        return;                                         /* exit now */
                } else {
                        prev = p;
                        p = p->next;
                }
    }
    /* (*kernel_errmsg)(&msgsym(NOTQ, SYS), "unqueue", eptr, q);
     */
    perror("unqueue:Error\n");

}


#endif  /* IOS_PARSER */
