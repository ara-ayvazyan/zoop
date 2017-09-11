#pragma once

#include "object_pool.h"
#include "null_reset.h"
#include "detail/rebind_queue.h"
#include <type_traits>


namespace zoop
{
    template <typename Queue, typename Reset = null_reset>
    class object_pool_builder
    {
    public:
        explicit object_pool_builder(Queue queue = {}, Reset reset = {})
            : m_queue{ std::move(queue) },
              m_reset{ std::move(reset) }
        {}

        template <typename OtherQueue, typename... QueueArgs>
        auto wrap_queue(QueueArgs&&... args)
        {
            return object_pool_builder<OtherQueue, Reset>{ OtherQueue{ std::move(m_queue), std::forward<QueueArgs>(args)... }, std::move(m_reset) };
        }

        template <template <typename, typename...> typename OtherQueue, typename... T, typename... QueueArgs>
        auto wrap_queue(QueueArgs&&... args)
        {
            return wrap_queue<OtherQueue<Queue, T...>>(std::forward<QueueArgs>(args)...);
        }

        template <template <typename, typename...> typename OtherQueue, typename... QueueArgs>
        auto wrap_queue_bind(QueueArgs&&... args)
        {
            return wrap_queue<OtherQueue, std::decay_t<QueueArgs>...>(std::forward<QueueArgs>(args)...);
        }

        template <typename T, typename OtherReset>
        auto bind_object(OtherReset&& reset)
        {
            return object_pool_builder<detail::rebind_queue_t<Queue, T>, OtherReset>{ std::move(m_queue), std::move(reset) };
        }

        template <template <typename, typename...> typename W, typename... T, typename... ResetArgs>
        auto wrap_object(ResetArgs&&... args)
        {
            using X = W<typename Queue::value_type, T...>;
            return bind_object<X>(X::make_reset(std::move(m_reset), std::forward<ResetArgs>(args)...));
        }

        auto build() &&
        {
            return object_pool<Queue, Reset>{ std::move(m_reset), std::move(m_queue) };
        }

    private:
        Queue m_queue;
        Reset m_reset;
    };

} // zoop
