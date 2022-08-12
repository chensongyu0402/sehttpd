#include "memory_pool.h"

#define POOL_SZ 8192

/* Pool as stack */
struct {
    http_request_t **pool;
    int next;
    pthread_mutex_t lock;
    int size;
} req_pool;

struct {
    job **pool;
    int next;
    pthread_mutex_t lock;
    int size;
} job_pool;

int init_req_pool(int nt)
{
    pthread_mutex_init(&req_pool.lock, NULL);
    http_request_t ***pool = &req_pool.pool;
    *pool = malloc(nt * POOL_SZ * sizeof(http_request_t *));
    for (int i = 0; i < nt * POOL_SZ; i++) {
        (*pool)[i] = malloc(sizeof(http_request_t));
    }
    req_pool.next = 0;
    req_pool.size = nt * POOL_SZ;

    return 0;
}

int init_job_pool(int nt)
{
    pthread_mutex_init(&job_pool.lock, NULL);
    job ***pool = &job_pool.pool;
    *pool = malloc(nt * POOL_SZ * sizeof(job *));
    for (int i = 0; i < nt * POOL_SZ; i++) {
        (*pool)[i] = malloc(sizeof(job));
    }
    job_pool.next = 0;
    job_pool.size = nt * POOL_SZ;

    return 0;
}

void get_request(http_request_t **r)
{
    pthread_mutex_t *lock = &req_pool.lock;
    pthread_mutex_lock(lock);
    int idx = req_pool.next++;
    if (idx == req_pool.size) {
        printf("Too much client connected!\n");
        pthread_mutex_unlock(lock);
        *r = NULL;
    } else {
        *r = req_pool.pool[idx];
        pthread_mutex_unlock(lock);
    }
}

void get_job(job **j)
{
    pthread_mutex_t *lock = &job_pool.lock;
    pthread_mutex_lock(lock);
    int idx = job_pool.next++;
    if (idx == job_pool.size) {
        printf("Too much client connected!\n");
        pthread_mutex_unlock(lock);
        *j = NULL;
    } else {
        *j = job_pool.pool[idx];
        pthread_mutex_unlock(lock);
    }
}

int free_request(http_request_t *r)
{
    pthread_mutex_lock(&req_pool.lock);
    int idx = --req_pool.next;
    req_pool.pool[idx] = r;
    pthread_mutex_unlock(&req_pool.lock);
    return 0;
}

int free_job(job *j)
{
    pthread_mutex_lock(&job_pool.lock);
    int idx = --job_pool.next;
    job_pool.pool[idx] = j;
    pthread_mutex_unlock(&job_pool.lock);
    return 0;
}