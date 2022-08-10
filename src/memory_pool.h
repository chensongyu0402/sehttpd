#ifndef _MEM_POOL_
#define _MEM_POOL_

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include "thread_pool.h"

int init_req_pool(int nt);
int init_job_pool(int nt);
void get_request(http_request_t **r);
void get_job(job **j);
int free_request(http_request_t *r);
int free_job(job *j);

#endif