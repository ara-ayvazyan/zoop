#pragma once

#include <boost/config.hpp>
#include <utility>
#include <functional>


namespace zoop
{
    namespace detail
    {
        template <typename... Args>
        inline auto bind_allocate(const Args&... args)
        {
            return std::bind([args...](auto& q) { return q.allocate(args...); }, std::placeholders::_1);
        }

    } // detail


    template <typename Queue, typename... Args>
    class growable_queue : public Queue
    {
    public:
        growable_queue(Queue queue, const Args&... args)
            : Queue{ std::move(queue) },
              m_allocate{ detail::bind_allocate(args...) }
        {}

        template <typename OtherQueue>
        growable_queue(growable_queue<OtherQueue, Args...> other)
            : Queue{ std::move(other) },
              m_allocate{ std::move(other.m_allocate) }
        {}

        auto pop()
        {
            if (auto n = Queue::try_pop())
            {
                return n;
            }

            return do_allocate();
        }

    private:
        template <typename OtherQueue, typename... OtherArgs>
        friend class growable_queue;

        BOOST_NOINLINE auto do_allocate()
        {
            return m_allocate(static_cast<Queue&>(*this));
        }

        decltype(detail::bind_allocate(std::declval<const Args&>()...)) m_allocate;
    };

} // zoop
