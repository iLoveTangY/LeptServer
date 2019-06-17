//
// Created by tang on 19-6-16.
//

#ifndef LEPTSERVER_NONCOPYABLE_H
#define LEPTSERVER_NONCOPYABLE_H

namespace lept_server
{
    class noncopyable
    {
    protected:
        noncopyable() = default;

        ~noncopyable() = default;

    public:
        // 禁止拷贝
        noncopyable(const noncopyable &) = delete;

        const noncopyable &operator=(const noncopyable &) = delete;
    };
}

#endif //LEPTSERVER_NONCOPYABLE_H
