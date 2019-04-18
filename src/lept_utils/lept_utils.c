//
// Created by tang on 19-3-18.
//
#include <stddef.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>
#include "lept_utils.h"
#include "../lept_definition.h"
#include "../log.h"

handler_t *lept_signal(int signo, handler_t *p_func)
{
    struct sigaction action, old_action;

    action.sa_handler = p_func;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    int rc = sigaction(signo, &action, &old_action);
    CHECK(rc >= 0, "Error in sigaction");
    return old_action.sa_handler;
}

int lept_open_listenfd(const char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd = -1, optval = 1;
    int rc;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    rc = getaddrinfo(NULL, port, &hints, &listp);
    CHECK(rc == 0, "Error in getaddrinfo");

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, //line:netp:csapp:setsockopt
                   (const void *)&optval, sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        rc = close(listenfd);
        CHECK(rc >= 0, "Error when close listen fd");
    }

    /* Clean up */
    freeaddrinfo(listp);
    CHECK(p != NULL, "No address worked");

    /* Make it a listening socket ready to accept connection requests */
    rc = listen(listenfd, LISTENQ);
    CHECK(rc >= 0, "Error when make listening socket");
    return listenfd;
}

int lept_make_fd_unblocked(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    CHECK(flags >= 0, "Error in fcntl, fd = %d", fd);

    flags |= O_NONBLOCK;
    int s = fcntl(fd, F_SETFL, flags);
    CHECK(s != -1, "Error in fcntl");
    return 0;
}

int lept_str_n_cmp(const char *m, const char *s, int n)
{
    for (int i = 0; i < n; ++i)
        if (m[i] != s[i])
            return 1;
    return 0;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)  /* interrupted by sig handler return */
                nwritten = 0;    /* and call write() again */
            else {
                fprintf(stderr, "errno == %d\n", errno);
                return -1;       /* errorno set by write() */
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }

    return n;
}
