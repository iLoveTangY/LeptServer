cmake_minimum_required(VERSION 2.6)
project(LeptServer C)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -Wall -Werror -Wextra -pthread -DNDEBUG")

add_executable(LeptServer src/main.c src/lept_utils/lept_utils.c src/lept_definition.h src/lept_threadpool/lept_thread_pool.c src/lept_threadpool/lept_thread_pool.h src/lept_min_heap/lept_min_heap.c src/lept_min_heap/lept_min_heap.h src/lept_epoll/lept_epoll.c src/lept_epoll/lept_epoll.h src/lept_http/lept_http.c src/lept_http/lept_http.h src/log.h)

add_executable(thread_pool_test src/lept_threadpool/threadpool_test.c src/lept_threadpool/lept_thread_pool.c)

add_executable(lept_http_test src/lept_http/lept_http_test.c src/lept_http/lept_http.c src/lept_epoll/lept_epoll.c src/lept_utils/lept_utils.c)
