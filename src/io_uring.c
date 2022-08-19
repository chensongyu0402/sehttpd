#include <arpa/inet.h>
#include <assert.h>
#include <liburing.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "io_uring.h"

void io_uring_init(struct io_uring *ring)
{
    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    int ret = io_uring_queue_init_params(QUEUE_DEPTH, ring, &params);
    assert(ret >= 0 && "io_uring_queue_init");
}

void add_accept_request(struct io_uring *ring,
                        http_request_t *r,
                        int listenfd,
                        struct sockaddr *client_addr,
                        socklen_t *client_len)
{
    r->event_type = ACCEPT;
    r->fd = listenfd;

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe, listenfd, client_addr, client_len, 0);
    io_uring_sqe_set_flags(sqe, 0);
    io_uring_sqe_set_data(sqe, r);
    io_uring_submit(ring);
}

void add_read_request(struct io_uring *ring, http_request_t *r)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    int clientfd = r->fd;
    r->event_type = READ;

    io_uring_prep_recv(sqe, clientfd, r->buf, MAX_BUF, 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data(sqe, r);

    r = malloc(sizeof(http_request_t));
    add_timeout_request(ring, r);
    io_uring_submit(ring);
}

void add_write_request(struct io_uring *ring, http_request_t *r, void *buf)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    char *src = buf;
    r->event_type = WRITE;
    size_t len = strlen(src);

    io_uring_prep_send(sqe, r->fd, src, len, 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data(sqe, r);

    r = malloc(sizeof(http_request_t));
    add_timeout_request(ring, r);
    io_uring_submit(ring);
}

void add_timeout_request(struct io_uring *ring, http_request_t *r)
{
    struct __kernel_timespec ts;
    ts.tv_sec = TIMEOUT_DEFAULT / 1000;
    r->event_type = TIMEOUT;

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_link_timeout(sqe, &ts, 0);
    io_uring_sqe_set_flags(sqe, 0);
    io_uring_sqe_set_data(sqe, r);
}