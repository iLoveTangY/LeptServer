#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include "lept_definition.h"
#include "lept_utils/lept_utils.h"
#include "lept_epoll/lept_epoll.h"
#include "lept_threadpool/lept_thread_pool.h"
#include "lept_http/lept_http.h"
#include "lept_timer/lept_timer.h"

static runtime_context_t context;

lept_server_timers_t sys_timers;

static void usage()
{
    fprintf(stderr,
            "LeptServer [option]... \n"
            "  -p|--port <port number>  Specify port number. Default is 8080"
            "  -t|--threads <thread num>  Specify thread num. Default is 4\n"
            "  -V|--version             Display program version.\n"
    );
}

void get_cmd_args(int argc, char **argv)
{
    int option_index = 0;
    char *option_str = "c:V";
    int opt;

    // 以下处理命令行参数
    static const struct option long_options[] =
            {
                    {"version", no_argument,       NULL, 'V'},
                    {"threads", required_argument, NULL, 't'},
                    {"port",    required_argument, NULL, 'p'},
                    {NULL,      0,                 NULL, 0}
            };

    context.thread_nums = THREADNUMS;
    context.port_num = PORTNUMBER;

    while ((opt = getopt_long(argc, argv, option_str, long_options, &option_index)) != EOF)
    {
        switch (opt)
        {
            case 'V':
                printf(LEPTSERVER_VERSION"\n");
                exit(0);
            case 't':
                context.thread_nums = atoi(optarg);
                break;
            case 'p':
                context.port_num = optarg;
            case 0:
                break;
            default:
                break;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        exit(0);
    }

    get_cmd_args(argc, argv);

    // 忽略SIGPIPE信号：当一个已连接描述符被关闭时，继续往这个描述符的第二次写操作会引发SIGPIPE信号，导致程序退出
    lept_signal(SIGPIPE, SIG_IGN);

    int rc;

    int listenfd = lept_open_listenfd(context.port_num);
    lept_make_fd_unblocked(listenfd);

    int epfd = lept_epoll_create();

    lept_http_request_t *r = (lept_http_request_t *)malloc(sizeof(lept_http_request_t));
    CHECK(r != NULL, "erro in malloc http request");

    lept_server_timer_init(&sys_timers);
    lept_http_request_init(r, epfd, listenfd);

    lept_epoll_event event;
    event.data.ptr = (void *)r;
    event.events = EPOLLIN | EPOLLET;
    lept_epoll_add(epfd, listenfd, &event);

    lept_thread_pool_t *pool = (lept_thread_pool_t *)malloc(sizeof(lept_thread_pool_t));
    CHECK(pool != NULL, "Error when malloc memory to thread pool");
    lept_threadpool_init(pool, context.thread_nums);

    log_info("Started...\n");

    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);
    char client_hostname[MAXLINE], client_port[MAXLINE];

    for(;;)
    {
        debug("Main thread: wating for new connection...");
        int time = get_epoll_wait_time(&sys_timers);
        int nfd = lept_epoll_wait(epfd, events, MAXEVENTS, time);
        handle_expire_timers(&sys_timers);  // 处理超时连接
        debug("Main thread: %d fd is ready, epfd is %d...", nfd, epfd);

        for (int i = 0; i < nfd; ++i)
        {
            r = (lept_http_request_t *)(events[i].data.ptr);
            int fd = r->fd;
            debug("Main Thread: r->epfd: %d", r->epfd);
            if (fd == listenfd)
            {  // 如果监听描述符已准备好
                debug("Main thread: listen fd is ready...");
                int connfd;
                for (;;)  // 可能不止一个客户端需要连接
                {
                    connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);
                    if (connfd < 0)
                    {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                            break;
                        else
                            CHECK(0, "Error in accept");
                    }
                    rc = getnameinfo((struct sockaddr *)&client_addr, client_len, client_hostname, MAXLINE,
                            client_port, MAXLINE, 0);
                    if (rc != 0)
                    {
                        fprintf(stderr, "%s\n", gai_strerror(rc));
                        exit(0);
                    }

                    debug("Connected from (%s, %s)\n", client_hostname, client_port);

                    lept_make_fd_unblocked(connfd);

                    lept_http_request_t *tmp = (lept_http_request_t *)malloc(sizeof(lept_http_request_t));
                    CHECK(tmp != NULL, "Error when malloc memory to http request.");
                    lept_http_request_init(tmp, epfd, connfd);
                    event.data.ptr = (void*)tmp;
                    // 设置为EPOLLONESHOT时一次触发之后需要显式的再设置一次才会让epoll_wait返回该描述符
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    debug("Main thread: add a new connection(fd is %d) to epoll", connfd);
                    lept_epoll_add(epfd, connfd, &event);
                    add_timer(&sys_timers, tmp, DEFAULT_TIMEOUT, lept_http_close_connection);
                }
            }
            else
            {
                if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
                {
                    fprintf(stderr, "Epoll error in fd : %d\n", fd);
                    close(fd);
                }
                debug("Main thread: connection fd %d can read", fd);
                lept_threadpool_add(pool, process_request, r);
            }
        }
    }
    lept_threadpool_destroy(pool, GRACEFUL_SHUTDWON);
    close(listenfd);
    return 0;
}