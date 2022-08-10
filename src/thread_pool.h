#ifndef _THPOOL_
#define _THPOOL_

#ifdef __cplusplus
extern "C" {
#endif

/* ===== Structures ===== */

typedef struct bsem {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int v;
} bsem;

typedef struct job {
    struct job *prev;
    void (*function)(void *arg);
    void *arg;         
} job;


/* Job queue */
typedef struct jobqueue {
    pthread_mutex_t rwmutex;
    job *front;
    job *rear;
    bsem *has_jobs;
    int len;
} jobqueue;


/* Thread */
typedef struct thread {
    int id;
    pthread_t pthread;
    struct thpool_ *thpool_p;
} thread;


/* Threadpool */
typedef struct thpool_ {
    thread **threads;
    volatile int num_threads_alive;
    volatile int num_threads_working;
    pthread_mutex_t thcount_lock;
    pthread_cond_t threads_all_idle;
    jobqueue jobqueue;
} thpool_;

/* ===== API ===== */

typedef struct thpool_ *threadpool;
threadpool thpool_init(int num_threads);
int thpool_add_work(threadpool, void (*function_p)(void *), void *arg_p);
void thpool_wait(threadpool);
void thpool_pause(threadpool);
void thpool_resume(threadpool);
void thpool_destroy(threadpool);
int thpool_num_threads_working(threadpool);

#ifdef __cplusplus
}
#endif

#endif