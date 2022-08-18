#ifndef __IO_URING__
#define __IO_URING__

#include <liburing.h>

#include "http.h"

#define QUEUE_DEPTH 8192
#define TIMEOUT_DEFAULT 500

void io_uring_init(struct io_uring *ring);
void add_accept_request(struct io_uring *ring,
                        http_request_t *r,
                        int listenfd,
                        struct sockaddr *client_addr,
                        socklen_t *client_len);
void add_read_request(struct io_uring *ring, http_request_t *r);
void add_write_request(struct io_uring *ring, http_request_t *r, void *buf);
void add_timeout_request(struct io_uring *ring, http_request_t *r);

#endif