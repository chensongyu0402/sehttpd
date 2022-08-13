#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>
#include <stdbool.h>
#include "http.h"

#define TIMEOUT_DEFAULT 500 /* ms */

typedef int (*timer_callback)(http_request_t *req);

typedef struct {
    size_t key;
    bool deleted; /* if remote client close socket first, set deleted true */
    timer_callback callback;
    http_request_t *request;
} timer_node;

int timer_init();
int find_timer();
void handle_expired_timers();

void add_timer(http_request_t *req, size_t timeout, timer_callback cb);
void del_timer(http_request_t *req);

extern pthread_mutex_t timer_lock;
extern pthread_cond_t timer_notify;
#endif