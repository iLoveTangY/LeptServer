//
// Created by tang on 19-3-19.
//

#ifndef LEPTSERVER_LEPT_HTTP_H
#define LEPTSERVER_LEPT_HTTP_H

#include <strings.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include "../lept_definition.h"

typedef struct head_list
{
    char *key_start;
    char *key_end;
    char *value_start;
    char *value_end;
    struct head_list *next;
} lept_head_list_t;

typedef struct lept_http_request
{
    int epfd, fd;

    char buf[MAXBUF];  // ring buffer
    size_t pos, last;  // 最后一个字符索引

    enum HTTP_METHOD_T method;

    char *method_start;
    char *method_end;
    char *uri_start;
    char *uri_end;
    int http_version_major;
    int http_version_minor;
    char *request_end;

    char *cur_key_start;
    char *cur_key_end;
    char *cur_value_start;
    char *cur_value_end;

    int state;

    lept_head_list_t *head;  // 链表存储header
} lept_http_request_t;

typedef struct lept_http_header_out
{
    int keep_alive;
    time_t mtime;  // 修改时间
    int modified;  // 是否修改

    int status;
} lept_http_header_out_t;

int lept_http_request_init(lept_http_request_t *request, int epfd, int fd);

void process_request(void *argp);

int lept_prase_request_line(lept_http_request_t *request);

int lept_parse_request_header(lept_http_request_t *request);

int parse_uri(char *uri, int uri_length, char *filename, char *cgiargs);

int lept_out_init(lept_http_header_out_t *o);

int lept_http_header_handler(lept_http_request_t *request, lept_http_header_out_t *out);

void serve_static(int fd, char *filename, size_t filesize, lept_http_header_out_t *out);

#endif //LEPTSERVER_LEPT_HTTP_H
