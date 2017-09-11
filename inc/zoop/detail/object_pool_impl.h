#pragma once

#include <memory>
#include <cassert>


namespace zoop
{
    namespace detail
    {
        template <typename Queue, typename Reset>
        class object_pool_impl
        {
        private:
            class deleter;

        public:
            using value_type = typename Queue::value_type;

            template <typename U>
            using handle_for = std::unique_ptr<U, deleter>;

            object_pool_impl(Reset reset, Queue queue)
                : m_queue{ std::move(queue) },
                  m_reset{ std::move(reset) }
            {}

            template <typename U>
            auto take(const std::shared_ptr<object_pool_impl>& this_shared)
            {
                assert(this == this_shared.get());
                return to_handle<U>(this_shared, pop());
            }

            const auto& queue() const noexcept
            {
                return m_queue;
            }

        private:
            using node_ptr = typename Queue::node_ptr;

            class deleter
            {
            public:
                deleter() = default;

                deleter(const std::shared_ptr<object_pool_impl>& pool, node_ptr node) noexcept
                    : m_pool{ pool },
                      m_node{ std::move(node) }
                {}

                void operator()(value_type* obj)
                {
                    assert(obj);
                    (void)obj;

                    if (auto pool = m_pool.lock())
                    {
                        pool->push(std::move(m_node));
                    }
                    else
                    {
                        m_node = {};
                    }
                }

            private:
                std::weak_ptr<object_pool_impl> m_pool;
                node_ptr m_node{};
            };

            template <typename U>
            static handle_for<U> to_handle(const std::shared_ptr<object_pool_impl>& this_shared, node_ptr n) noexcept
            {
                if (n)
                {
                    auto ptr = &*n;
                    return { ptr, { this_shared, std::move(n) } };
                }

                return {};
            }

            auto pop()
            {
                return m_queue.pop();
            }

            void push(node_ptr n)
            {
                assert(n);
                m_reset(*n);
                m_queue.push(std::move(n));
            }

            Queue m_queue;
            Reset m_reset;
        };

    } // detail
} // zoop
