//
// Created by tang on 19-6-16.
//
#include <iostream>
#include <Thread.h>
#include <cassert>
#include "CurrentThreadInfo.h"
#include "MutexLock.h"

using namespace std;
using namespace lept_server;

int sum = 0;

MutexLock mutex;

void add_one()
{
    cout << "Thread Name: ";
    cout << CurrentThreadInfo::get_thread_name() << endl;
    cout << "Thread id: ";
    cout << CurrentThreadInfo::get_tid_string() << endl;
    for (int i = 1; i <= 100000; ++i)
    {
        MutextLockGuard lock(mutex);
        sum++;
    }
}

int main()
{
    Thread t1(add_one);
    t1.start();
//    t.join();

    Thread t2(add_one);
    t2.start();

    t1.join();
    t2.join();

    cout << sum << endl;
    assert(sum == 200000);

    return 0;
}

