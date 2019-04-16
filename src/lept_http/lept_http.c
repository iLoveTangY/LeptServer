//
// Created by tang on 19-3-19.
//

/* 为啥定义这个宏？ 看这里：http://stackoverflow.com/questions/15334558/compiler-gets-warnings-when-using-strptime-function-ci */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "lept_http.h"
#include "../lept_utils/lept_utils.h"
#include "../lept_epoll/lept_epoll.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

typedef struct mime_type_s
{
    const char *type;
    const char *value;
} lept_mime_type_t;

lept_mime_type_t mimes[] =
        {
                {".html",  "text/html"},
                {".xml",   "text/xml"},
                {".xhtml", "application/xhtml+xml"},
                {".txt",   "text/plain"},
                {".rtf",   "application/rtf"},
                {".pdf",   "application/pdf"},
                {".word",  "application/msword"},
                {".png",   "image/png"},
                {".gif",   "image/gif"},
                {".jpg",   "image/jpeg"},
                {".jpeg",  "image/jpeg"},
                {".au",    "audio/basic"},
                {".mpeg",  "video/mpeg"},
                {".mpg",   "video/mpeg"},
                {".avi",   "video/x-msvideo"},
                {".gz",    "application/x-gzip"},
                {".tar",   "application/x-tar"},
                {".css",   "text/css"},
                {NULL,     "text/plain"}
        };

typedef int (*header_handler_pt)(lept_http_request_t *request, lept_http_header_out_t *out, char *data, int len);

static const char *get_shortmsg_from_status_code(int status_code)
{
    if (status_code == HTTP_OK)
        return "OK";

    if (status_code == HTTP_NOT_MODIFIED)
        return "Not Modified";

    if (status_code == HTTP_NOT_FOUND)
        return "Not Found";

    return "Unknown";
}

typedef struct header_handler
{
    char *name;
    header_handler_pt handler;
} lept_header_handler_t;

static int lept_http_process_connection(lept_http_request_t *r, lept_http_header_out_t *out, char *data, int len)
{
    (void) r;
    if (strncasecmp("keep-alive", data, len) == 0)
    {
        out->keep_alive = 1;
    }
    return 0;
}

static void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char header[MAXLINE], body[MAXLINE];

    sprintf(body, "<html><title>LeptServer Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\n", body);
    sprintf(body, "%s%s: %s\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\n</p>", body, longmsg, cause);
    sprintf(body, "%s<hr><em>LeptServer</em>\n</body></html>", body);

    sprintf(header, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    sprintf(header, "%sServer: LeptServer\r\n", header);
    sprintf(header, "%sContent-type: text/html\r\n", header);
    sprintf(header, "%sConnection: close\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int) strlen(body));
    rio_writen(fd, header, strlen(header));
    rio_writen(fd, body, strlen(body));
}

int lept_out_init(lept_http_header_out_t *o)
{
    o->status = 0;
    o->modified = 1;
    o->mtime = 0;
    o->keep_alive = 0;

    return 0;
}

static int lept_http_process_if_modified_since(lept_http_request_t *r, lept_http_header_out_t *out, char *data, int len)
{
    (void) r;
    (void) len;
    struct tm tm;
    if (strptime(data, "%a, %d %b %Y %H:%M:%S GMT", &tm) == (char *) NULL)
        return 0;
    time_t cliemt_time = mktime(&tm);  // 将时间转换为自1970年1月1日以来持续时间的秒数，发生错误时返回-1

    double time_diff = difftime(out->mtime, cliemt_time);
    if (fabs(time_diff) < 1e-6)
    {
        out->modified = 0;
        out->status = HTTP_NOT_MODIFIED;
    }
    return 0;
}

lept_header_handler_t lept_header_handlers[] = {
        {"Connection",        lept_http_process_connection},
        {"If-Modified-Since", lept_http_process_if_modified_since},
        {"", NULL}
};

static const char *get_file_type(const char *type)
{
    if (type == NULL)
        return "text/plain";
    int i = 0;
    for(; mimes[i].type != NULL; ++i)
    {
        if (strcmp(type, mimes[i].type) == 0)
            return mimes[i].value;
    }
    return mimes[i].value;
}

int lept_http_request_init(lept_http_request_t *request, int epfd, int fd)
{
    request->epfd = epfd;
    request->fd = fd;

    request->last = 0;
    request->pos = 0;
    request->state = LINE_START;
    request->head = (lept_head_list_t *) malloc(sizeof(lept_head_list_t));  // dummy head
    CHECK(request->head != NULL, "Error when malloc memory to request->head.");
    request->head->next = NULL;

    return 0;
}

void process_request(void *argp)
{
    lept_http_request_t *request = (lept_http_request_t *) argp;
    int fd = request->fd;
    int should_close = 0;

    for (;;)
    {
        size_t remain_size = MIN(MAXBUF - (request->last - request->pos) - 1, MAXBUF - request->last % MAXBUF);
        CHECK(remain_size > 0, "Empty buffer for request.");
        char *plast = &request->buf[(request->last)%MAXBUF];
        ssize_t n = read(fd, plast, remain_size);

        if (n == 0)  // EOF 表示客户端已经关闭了连接
        {
            log_info("Finish read client");
            should_close = 1;  // 如果这里不关闭会怎么样？
            break;
        }
        if (n < 0)
        {
            if (errno != EAGAIN)
            {
                log_info("Error when read client, closed.");
                should_close = 1;
            }
            break;  // EAGAIN 表示暂时无数据可读，跳出循环等待下一次fd可读
        }

        request->last += n;

        int rc = lept_prase_request_line(request);  // 解析请求行
        if (rc == EAGAIN)
            continue;
        else if (rc != 0)
        {
            client_error(fd, "", "400", "Invalid Request", "LeptServer can't understand request");
            should_close = 1;
            break;
        }

        rc = lept_parse_request_header(request);  // 解析请求报头
        if (rc == EAGAIN)
            continue;
        else if (rc != 0)
        {
            log_info("bad request: \n%s\n", request->buf);
            client_error(fd, "", "400", "Invalid Request", "LeptServer can't understand request");
            should_close = 1;
            break;
        }
        char filename[MAXLINE], cgiargs[MAXLINE];
        int is_static = parse_uri(request->uri_start, (int) (request->uri_end - request->uri_start), filename, cgiargs);
        log_info("Request filename: %s\n", filename);
        struct stat sbuf;
        if (stat(filename, &sbuf) < 0)
        {
            client_error(fd, filename, "404", "Not Found", "zaver can't find the file");
            continue;
        }
        if (is_static)
        {
            lept_http_header_out_t *out = (lept_http_header_out_t *) malloc(sizeof(lept_http_header_out_t));
            if (out == NULL)
            {
                log_err("No Enough Space for lept_http_header_out_t");
                should_close = 1;
                break;
            }
            lept_out_init(out);
            // 无权访问
            if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
            {
                free(out);
                client_error(fd, filename, "403", "Forbidden", "Can't read the file");
                continue;
            }
            out->mtime = sbuf.st_mtime;
            // 处理请求报头（Connection, If-Modified-Since, ... ） TODO 响应更多请求报头
            lept_http_header_handler(request, out);

            serve_static(fd, filename, sbuf.st_size, out);

            if (!out->keep_alive)
            {
                should_close = 1;
                free(out);
                break;
            }
            free(out);
        }
        else
        {
            // TODO 处理动态内容
        }
    }
    // 需要分是读完数据还是还是没读完数据或者是keep-alive来做不同的处理
    if (should_close)
    {
        close(request->fd);  // 关闭描述符之后会自动从epoll的描述符集合中移除
        free(request);
    }
    else
    {
        struct epoll_event event;
        event.data.ptr = argp;
        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

        lept_epoll_mod(request->epfd, request->fd, &event);
    }
}

void serve_static(int fd, char *filename, size_t filesize, lept_http_header_out_t *out)
{
    const char *dot_pos = strrchr(filename, '.');
    const char *filetype = get_file_type(dot_pos);

    char header[MAXLINE];
    char buf[MAXBUF];
    sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, get_shortmsg_from_status_code(out->status));

    if (out->keep_alive)
    {
        sprintf(header, "%sConnection: keep-alive\r\n", header);
        sprintf(header, "%sKeep-live: timeout=%d\r\n", header, DEFAULT_TIMEOUT);
    }

    if (out->modified)
    {
        sprintf(header, "%sContent-type: %s\r\n", header, filetype);
        sprintf(header, "%sContent_length: %zu\r\n", header, filesize);
        struct tm tm;
        localtime_r(&(out->mtime), &tm);
        strftime(buf, MAXLINE, "%a, %d %b %Y %H:%M:%S GMT", &tm);
        sprintf(header, "%sLast-Modified: %s\r\n", header, buf);
    }

    sprintf(header, "%sServer: LeptServer\r\n", header);
    sprintf(header, "%s\r\n", header);

    size_t n = (size_t) rio_writen(fd, header, strlen(header));
    if (n != strlen(header))
    {
        // TODO 记录日志
    }
    if (out->modified)
    {
        int srcfd = open(filename, O_RDONLY, 0);
        if (srcfd <= 2)
        {
            // TODO error in here
        }
        char *srcaddr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
        if (srcaddr == (void *)-1)
        {
            // TODO error in here
        }
        close(srcfd);
        n = rio_writen(fd, srcaddr, filesize);
        munmap(srcaddr, filesize);
    }
}

int lept_http_header_handler(lept_http_request_t *request, lept_http_header_out_t *out)
{
    lept_head_list_t *prev = request->head;
    lept_head_list_t *pos = request->head->next;
    while (pos)
    {
        for (lept_header_handler_t *h = lept_header_handlers; strlen(h->name) > 0; ++h)
        {
            if (strncmp(pos->key_start, h->name, pos->key_end - pos->key_start) == 0)
            {
                size_t value_len = pos->value_end - pos->value_start;
                (*(h->handler))(request, out, pos->value_start, value_len);
            }
        }
        lept_head_list_t *tmp = pos;
        prev->next = pos->next;
        pos = pos->next;
        free(tmp);
    }

    return 1;
}

// 状态机解析http请求行
int lept_prase_request_line(lept_http_request_t *request)
{
    enum HTTP_PARSE_REQUEST_LINE_STATE_T state = request->state;

    int finish_parse = 0;
    size_t i;
    for (i = request->pos; i < request->last && !(finish_parse); ++i)
    {
        char *p = &request->buf[i];
        char ch = *p;
        switch (state)
        {
            case LINE_START:
                request->method_start = p;
                if (ch == '\r' || ch == '\n' || ch == ' ')
                    break;
                if (ch < 'A' || ch > 'Z')
                    return HTTP_PARSE_INVALID_METHOD;
                state = METHOD;
                break;
            case METHOD:
                if (ch == ' ')  // 遇到空格表示方法的解析已经完成
                {
                    request->method_end = p;
                    char *m = request->method_start;
                    switch (p - m)
                    {
                        case 3:
                            if (lept_str_n_cmp(m, "GET", 3) == 0)
                                request->method = METHOD_GET;
                            else
                                request->method = METHOD_UNKNOWN;
                            break;
                        case 4:
                            if (lept_str_n_cmp(m, "POST", 4) == 0)
                                request->method = METHOD_POST;
                            else if (lept_str_n_cmp(m, "HEAD", 4))
                                request->method = METHOD_HEAD;
                            else
                                request->method = METHOD_UNKNOWN;
                            break;
                        default:
                            request->method = METHOD_UNKNOWN;
                            break;
                    }
                    state = SPACE_BEFORE_URI;
                }
                else if (ch < 'A' || ch > 'Z')
                    return HTTP_PARSE_INVALID_METHOD;
                break;
            case SPACE_BEFORE_URI:
                if (ch == '/')
                {
                    request->uri_start = p;
                    state = SLASH_IN_URI;
                    break;
                }
                else if (ch == ' ')
                    break;
                else
                    return HTTP_PARSE_INVALID_URI;
            case SLASH_IN_URI:
                if (ch == ' ')
                {
                    request->uri_end = p;
                    state = SPACE_BEFORE_HTTP;
                    break;
                }
                break;
            case SPACE_BEFORE_HTTP:
                if (ch == 'H')
                    state = HTTP_H;
                else if (ch == ' ')
                    break;
                else
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                break;
            case HTTP_H:
                if (ch == 'T')
                    state = HTTP_HT;
                else
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                break;
            case HTTP_HT:
                if (ch == 'T')
                    state = HTTP_HTT;
                else
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                break;
            case HTTP_HTT:
                if (ch == 'P')
                    state = HTTP_HTTP;
                else
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                break;
            case HTTP_HTTP:
                if (ch == '/')
                    state = SLASH_AFTER_HTTP;
                else
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                break;
            case SLASH_AFTER_HTTP:
                if (!ISNUMBER(ch))
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                request->http_version_major = ch - '0';
                state = MAJOR_DIGIT;
                break;
            case MAJOR_DIGIT:
                if (ch == '.')
                {
                    state = DOT_IN_DIGIT;
                    break;
                }
                if (!ISNUMBER(ch))
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                request->http_version_major = request->http_version_major * 10 + ch - '0';
                break;
            case DOT_IN_DIGIT:
                if (!ISNUMBER(ch))
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                request->http_version_minor = ch - '0';
                state = MINOR_DIGIT;
                break;
            case MINOR_DIGIT:
                if (ch == '\r')
                {
                    state = LINE_DONE;
                    break;
                }
                else if (ch == ' ')
                {
                    state = SPACE_AFTER_DIGIT;
                    break;
                }
                else if (ch == '\n')
                {
                    finish_parse = 1;
                    break;
                }
                else if (!ISNUMBER(ch))
                    return HTTP_PARSE_INVALID_HTTP_VERSION;
                request->http_version_minor = request->http_version_minor * 10 + ch - '0';
                break;
            case SPACE_AFTER_DIGIT:
                if (ch == ' ')
                    break;
                else if (ch == '\r')
                {
                    state = LINE_DONE;
                    break;
                }
                else if (ch == '\n')
                {
                    finish_parse = 1;
                    break;
                }
                else
                    return HTTP_PARSE_INVALID_REQUEST_LINE;
            case LINE_DONE:
                request->request_end = p;
                if (ch == '\n')
                {
                    finish_parse = 1;
                    break;
                }
                else
                    return HTTP_PARSE_INVALID_REQUEST_LINE;
        }
    }

    if (!finish_parse)  // 这次没有解析完请求行
    {
        request->pos = i;  // 下一次开始位置
        request->state = state;  // 下一次开始的状态等于当前状态
        return EAGAIN;
    }

    request->pos = i;  // 下一行开始位置
    request->state = HEADER_START;  // 准备解析请求报头

    return 0;
}

int lept_parse_request_header(lept_http_request_t *request)
{
    enum HTTP_PARSE_REQUEST_BODY_STATE_T state = request->state;
    int finish_parse = 0;
    size_t i;

    for (i = request->pos; i < request->last && !(finish_parse); ++i)
    {
        char *p = &request->buf[i];
        char ch = *p;
        switch (state)
        {
            case HEADER_START:
                if (ch == '\r' || ch == '\n')  // 如果刚开始就是\r\n表明没有请求报头？跳出时是否应该结束解析？
                {
                    finish_parse = 1;
                    break;
                }
                request->cur_key_start = p;
                state = KEY;
                break;
            case KEY:
                if (ch == ' ')
                {
                    request->cur_key_end = p;
                    state = KEY_END;
                    break;
                }
                if (ch == ':')
                {
                    request->cur_key_end = p;
                    state = COLON_AFTER_KEY;
                    break;
                }
                break;
            case KEY_END:
                if (ch == ' ')
                    break;
                if (ch == ':')
                {
                    state = COLON_AFTER_KEY;
                    break;
                }
                return HTTP_PARSE_INVALID_REQUEST_LINE;
            case COLON_AFTER_KEY:
                if (ch == ' ')
                    break;
                state = VALUE_START;
                request->cur_value_start = p;
                break;
            case SPACE_AFTER_COLON:
                break;
            case VALUE_START:
                if (ch == '\r')
                {
                    state = CR;
                    request->cur_value_end = p;
                }
                else if (ch == '\n')
                {
                    request->cur_value_end = p;
                    state = CRLF;
                }
                break;
            case SPACE_AFTER_VALUE:
                if (ch == ' ')
                    break;
                else if (ch == '\r')
                {
                    state = CR;
                    break;
                }
                else if (ch == '\n')
                {
                    state = CRLF;
                    break;
                }
                return HTTP_PARSE_INVALID_REQUEST_LINE;
            case CR:
                if (ch == '\n')
                {
                    state = CRLF;
                    lept_head_list_t *h = (lept_head_list_t *) malloc(sizeof(lept_head_list_t));
                    h->key_start = request->cur_key_start;
                    h->key_end = request->cur_key_end;
                    h->value_start = request->cur_value_start;
                    h->value_end = request->cur_value_end;
                    h->next = request->head->next;
                    request->head->next = h;
                    break;
                }
                return HTTP_PARSE_INVALID_REQUEST_LINE;
            case CRLF:
                if (ch == '\r')
                    state = CRLFCR;
                else
                {
                    request->cur_key_start = p;
                    state = KEY;
                }
                break;
            case CRLFCR:
                if (ch == '\n')
                {
                    finish_parse = 1;
                    break;
                }
                return HTTP_PARSE_INVALID_REQUEST_LINE;
        }
    }

    request->pos = i;
    if (finish_parse)
    {
        request->state = LINE_START;
        return 0;
    }
    request->state = state;

    return EAGAIN;
}

int parse_uri(char *uri, int uri_length, char *filename, char *cgiargs)
{
    // TODO 检查URI是否为空
    uri[uri_length] = '\0';

    if (!strstr(uri, "cgi-bin"))
    {
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "home.html");
        return 1;
    }
    else
    {
        char *ptr = index(uri, '?');
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}
