#include <arpa/inet.h>
#include <assert.h>
#include <liburing.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"
#include "io_uring.h"
#include "logger.h"

/* the length of the struct epoll_events array pointed to by *events */
#define MAXEVENTS 1024

#define LISTENQ 1024

static int open_listenfd(int port)
{
    int listenfd, optval = 1;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminate "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
                   sizeof(int)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to given port. */
    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons((unsigned short) port),
        .sin_zero = {0},
    };
    if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;

    return listenfd;
}

/* TODO: use command line options to specify */
#define PORT 8081
#define WEBROOT "./www"

int main()
{
    /* when a fd is closed by remote, writing to this fd will cause system
     * send SIGPIPE to this process, which exit the program
     */
    if (sigaction(SIGPIPE,
                  &(struct sigaction){.sa_handler = SIG_IGN, .sa_flags = 0},
                  NULL)) {
        log_err("Failed to install sigal handler for SIGPIPE");
        return 0;
    }

    int listenfd = open_listenfd(PORT);

    struct io_uring *ring = malloc(sizeof(*ring));
    io_uring_init(ring);
    http_request_t *r = malloc(sizeof(*r));
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    add_accept_request(ring, r, listenfd, (struct sockaddr *) &client_addr,
                       &client_len);

    while (1) {
        struct io_uring_cqe *cqe;
        io_uring_wait_cqe(ring, &cqe);
        http_request_t *cqe_req = io_uring_cqe_get_data(cqe);
        enum event_types type = cqe_req->event_type;

        if (type == ACCEPT) {
            add_accept_request(ring, r, listenfd,
                               (struct sockaddr *) &client_addr, &client_len);
            int clientfd = cqe->res;
            if (clientfd >= 0) {
                http_request_t *request = malloc(sizeof(http_request_t));
                init_http_request(request, clientfd, WEBROOT, ring);
                add_read_request(ring, request);
            }
        } else if (type == READ) {
            int read_bytes = cqe->res;
            if (read_bytes <= 0) {
                if (read_bytes < 0) {
                    fprintf(stderr, "Async request failed: %s for event: %d\n",
                            strerror(-cqe->res), cqe_req->event_type);
                }
                int ret = http_close_conn(cqe_req);
                assert(ret == 0 && "http_close_conn");
            } else {
                do_request(cqe_req, read_bytes);
            }
        } else if (type == WRITE) {
            int write_bytes = cqe->res;
            if (write_bytes <= 0) {
                if (write_bytes < 0) {
                    fprintf(stderr, "Async request failed: %s for event: %d\n",
                            strerror(-cqe->res), cqe_req->event_type);
                }
                int ret = http_close_conn(cqe_req);
                assert(ret == 0 && "http_close_conn");
            } else {
                if (cqe_req->keep_alive == false)
                    http_close_conn(cqe_req);
                else
                    add_read_request(ring, cqe_req);
            }
        } else if (type == TIMEOUT) {
            free(cqe_req);
        }
        io_uring_cq_advance(ring, 1);
    }
    io_uring_queue_exit(ring);

    return 0;
}