cmake_minimum_required(VERSION 2.6)
project(LeptServer)

set(CXX_FLAGS -g -Wall -std=c++11 -D_PTHREADS -Wno-unused-parameter)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -Wall -Werror -Wextra -pthread")

set(THREAD_DIR src/lept_thread)

set(THREAD_TEST_SRCS ${THREAD_DIR}/thread_test.cpp ${THREAD_DIR}/CountDownLatch.cpp ${THREAD_DIR}/Thread.cpp src/lept_thread/CurrentThreadInfo.cpp)

include_directories(src/lept_utils src/lept_thread)

add_executable(thread_test ${THREAD_TEST_SRCS})

#add_executable(lept_server src/main.c src/lept_utils/lept_utils.c src/lept_definition.h src/lept_threadpool/lept_thread_pool.c src/lept_threadpool/lept_thread_pool.h src/lept_timer/lept_min_heap.c src/lept_timer/lept_min_heap.h src/lept_epoll/lept_epoll.c src/lept_epoll/lept_epoll.h src/lept_http/lept_http.c src/lept_http/lept_http.h src/log.h src/lept_timer/lept_timer.c src/lept_timer/lept_timer.h)
#
#add_executable(thread_pool_test src/lept_threadpool/threadpool_test.c src/lept_threadpool/lept_thread_pool.c)
#
#add_executable(lept_http_test src/lept_http/lept_http_test.c src/lept_http/lept_http.c src/lept_epoll/lept_epoll.c src/lept_utils/lept_utils.c)
#
#add_executable(lept_min_heap_test src/lept_timer/lept_min_heap_test.c src/lept_timer/lept_min_heap.c)

target_link_libraries(thread_test pthread)
