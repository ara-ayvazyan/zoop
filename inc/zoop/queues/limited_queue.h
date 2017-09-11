#pragma once

#include <zoop/detail/overflow_counter.h>
#include <memory>
#include <type_traits>


namespace zoop
{
    template <typename Queue, typename IsThreadSafe = std::true_type>
    class limited_queue : public Queue
    {
    public:
        limited_queue(Queue queue, std::size_t max_size)
            : Queue{ std::move(queue) },
              m_capacity{ std::make_unique<capacity_counter>(max_size) }
        {}

        template <typename OtherQueue>
        limited_queue(limited_queue<OtherQueue>&& other)
            : limited_queue{ Queue{ std::move(other) }, other.max_size() }
        {}

        template <typename... Args>
        auto allocate(Args&&... args)
        {
            return m_capacity->try_add(1) ? Queue::allocate(std::forward<Args>(args)...) : nullptr;
        }

        std::size_t capacity() const noexcept
        {
            return m_capacity->value();
        }

        std::size_t max_size() const noexcept
        {
            return m_capacity->max_value();
        }

    private:
        using capacity_counter = detail::overflow_counter<std::size_t, IsThreadSafe::value>;

        std::unique_ptr<capacity_counter> m_capacity;
    };

} // zoop
