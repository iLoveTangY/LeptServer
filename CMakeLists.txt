cmake_minimum_required(VERSION 2.6)
project(LeptServer)

set(CXX_FLAGS -g -Wall -std=c++11 -D_PTHREADS -Wno-unused-parameter)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0")

set(THREAD_DIR src/lept_thread)
set(THREAD_TEST_SRCS ${THREAD_DIR}/thread_test.cpp ${THREAD_DIR}/CountDownLatch.cpp ${THREAD_DIR}/Thread.cpp src/lept_thread/CurrentThreadInfo.cpp)

include_directories(src/lept_utils src/lept_thread src/lept_asynclogger)

add_executable(thread_test ${THREAD_TEST_SRCS})

set(LOGGER_DIR src/lept_asynclogger)
set(LOGGER_TEST_SRCS
        ${LOGGER_DIR}/logger_test.cpp
        ${LOGGER_DIR}/AppendableFile.cpp
        ${LOGGER_DIR}/LogFile.cpp
        ${LOGGER_DIR}/LogStream.cpp
        ${LOGGER_DIR}/AsyncLogger.cpp
        ${LOGGER_DIR}/Logger.cpp
        ${THREAD_DIR}/CountDownLatch.cpp ${THREAD_DIR}/Thread.cpp src/lept_thread/CurrentThreadInfo.cpp)

add_executable(logger_test ${LOGGER_TEST_SRCS})

target_link_libraries(thread_test pthread)

target_link_libraries(logger_test pthread)
