
#ifndef MUTEX_HEADER
#define MUTEX_HEADER

enum mutex_types {
        USYNC_THREAD,
        USYNC_PROCESS
};

#define USYNC_THREAD    0

typedef struct semaphore        mutex_t;

mutex_t * MUTEX_CREATE(enum mutex_types a2, void * a3);
int MUTEX_LOCK(mutex_t * pMutex);
int MUTEX_UNLOCK(mutex_t * pMutex);

#define mutex_create    MUTEX_CREATE
#define mutex_lock      MUTEX_LOCK
#define mutex_unlock    MUTEX_UNLOCK

#endif /* MUTEX_HEADER */

