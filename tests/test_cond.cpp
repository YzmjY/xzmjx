//
// Created by 20132 on 2022/3/21.
//

/*
 * @Description:
 * @Author: XZMJX
 * @Date: 2021-12-05 23:20:42
 */
#include<pthread.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<vector>
#define COND_CHECK(func, cond, retv, errv) \
if ( (cond) ) \
{ \
   fprintf(stderr, "\n[CHECK FAILED at %s:%d]\n| %s(...)=%d (%s)\n\n",\
              __FILE__,__LINE__,func,retv,strerror(errv)); \
   exit(EXIT_FAILURE); \
}

#define ErrnoCheck(func,cond,retv)  COND_CHECK(func, cond, retv, errno)
#define PthreadCheck(func,rc) COND_CHECK(func,(rc!=0), rc, rc)
#define FOREVER for(;;)

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t *ptr_cv;
int predicate = 0;
int nthreads;

/*****************************************************************************/
/* thread - tell the shutdown thread that we're done                        */
/*****************************************************************************/
void*
thread(void* ignore)
{
    int rc;

    // this thread now terminate
    rc = pthread_mutex_lock(&mutex);
    PthreadCheck("pthread_mutex_lock", rc);

    nthreads--; // we have one thread less in the pool

    // note: we unlock first, and then signal
    rc = pthread_mutex_unlock(&mutex);
    PthreadCheck("pthread_mutex_unlock", rc);
    rc = pthread_cond_signal(ptr_cv);
    PthreadCheck("pthread_cond_signal", rc);

    // Ok, time to retire
    pthread_exit(NULL);
}


/*****************************************************************************/
/* shutdown_thread- wait all threads in the pool to finish and clean-up      */
/* condvar                                                                   */
/*****************************************************************************/
void*
shutdown_thread(void* ignore)
{
    int rc;

    // wait as long as one thread in the pool is running
    rc = pthread_mutex_lock(&mutex);
    PthreadCheck("pthread_mutex_lock", rc);

    while (nthreads>0) {
        rc = pthread_cond_wait(ptr_cv, &mutex);
        PthreadCheck("pthread_cond_wait", rc);
    }

    // all thread stopped running: we can destroy the condvar
    rc = pthread_cond_destroy(ptr_cv);
    PthreadCheck("pthread_cond_destroy", rc);
    free(ptr_cv);

    // unlock mutex, and bye!
    rc = pthread_mutex_unlock(&mutex);
    PthreadCheck("pthread_mutex_unlock", rc);
    pthread_exit(NULL);
}


/*****************************************************************************/
/* main- main thread                                                         */
/*****************************************************************************/

const int NTHREADS = 8; // # threads in the pool

int
main() {
    pthread_t pool[NTHREADS]; // threads pool
    pthread_t tshd;           // shutdown thread
    unsigned long count = 0;        // counter
    int rc;             // return code

    FOREVER {

        // initialize condvar
        nthreads = NTHREADS;
        ptr_cv = (pthread_cond_t *) malloc(sizeof(*ptr_cv));
        ErrnoCheck("malloc", (ptr_cv == NULL), 0);
        rc = pthread_cond_init(ptr_cv, NULL);
        PthreadCheck("pthread_cond_init", rc);

        // create shutdown thread
        rc = pthread_create(&tshd, NULL, shutdown_thread, NULL);
        PthreadCheck("pthread_create", rc);

        // create threads pool
        for (int i = 0; i < NTHREADS; i++) {
            rc = pthread_create(pool + i, NULL, thread, NULL);
            PthreadCheck("pthread_create", rc);
            rc = pthread_detach(pool[i]);
            PthreadCheck("pthread_detach", rc);
        }

        // wait shutdown thread completion
        rc = pthread_join(tshd, NULL);
        PthreadCheck("pthread_join", rc);

        // great... one more round
        ++count;
        printf("%lu\n", count);
    }

    // should be never reached
    return EXIT_SUCCESS;
}
