#pragma once

#include "limited_queue.h"
#include <new>


namespace zoop
{
    template <typename Queue, typename IsThreadSafe = std::true_type>
    class limited_throw_queue : public limited_queue<Queue, IsThreadSafe>
    {
    public:
        using limited_queue<Queue, IsThreadSafe>::limited_queue;

        template <typename OtherQueue>
        limited_throw_queue(limited_throw_queue<OtherQueue, IsThreadSafe>&& other)
            : limited_queue<Queue, IsThreadSafe>{ std::move(other) }
        {}

        template <typename... Args>
        auto allocate(Args&&... args)
        {
            if (auto n = limited_queue<Queue>::allocate(std::forward<Args>(args)...))
            {
                return n;
            }

            throw std::bad_alloc{};
        }
    };

} // zoop
