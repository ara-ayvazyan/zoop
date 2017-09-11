#pragma once

#include "lockfree_fixed_allocator.h"
#include <boost/config.hpp>
#include <mutex>
#include <atomic>
#include <cassert>


namespace zoop
{
    template <typename Buffer = std::unique_ptr<char[]>>
    class lockfree_allocator
    {
    public:
        explicit lockfree_allocator(std::size_t fixed_size)
            : m_head{ fixed_size },
              m_cur{ &m_head },
              m_fixed_size{ fixed_size }
        {}

        void* allocate(std::size_t size)
        {
            if (size <= m_fixed_size)
            {
                auto cur = m_cur.load(std::memory_order::memory_order_acquire);

                while (true)
                {
                    if (auto ptr = cur->allocator().allocate(size, std::nothrow))
                    {
                        return ptr;
                    }

                    if (cur->is_tail())
                    {
                        cur->try_append(m_fixed_size);
                    }

                    assert(!cur->is_tail() && cur->next());

                    if (m_cur.compare_exchange_strong(cur, cur->next()))
                    {
                        cur = cur->next();
                    }
                }
            }

            throw std::invalid_argument{ "Cannot allocate more than the fixed size." };
        }

        void reset() noexcept
        {
            for (auto cur = &m_head; cur; cur = cur->next())
            {
                assert(cur);
                cur->allocator().reset();
            }

            m_cur.store(&m_head, std::memory_order::memory_order_release);
        }

    private:
        class node
        {
        public:
            explicit node(std::size_t capacity)
                : m_alloc{ capacity }
            {}

            ~node()
            {
                auto n = std::move(m_next);

                while (n)
                {
                    auto next = std::move(n->m_next);
                    n = std::move(next);
                }
            }

            bool is_tail() const noexcept
            {
                return m_is_tail.load(std::memory_order::memory_order_acquire);
            }

            lockfree_fixed_allocator<Buffer>& allocator() noexcept
            {
                return m_alloc;
            }

            node* next() noexcept
            {
                return m_next.get();
            }

            BOOST_NOINLINE void try_append(std::size_t capacity)
            {
                std::call_once(
                    m_append_next_flag,
                    [&]
                    {
                        assert(!m_next);
                        m_next = std::make_unique<node>(capacity);
                        assert(m_is_tail);
                        m_is_tail.store(false, std::memory_order::memory_order_release);
                    });
            }

        private:
            lockfree_fixed_allocator<Buffer> m_alloc;
            std::atomic_bool m_is_tail{ true };
            std::once_flag m_append_next_flag{};
            std::unique_ptr<node> m_next;
        };

        node m_head;
        std::atomic<node*> m_cur;
        const std::size_t m_fixed_size;
    };

    extern template lockfree_allocator<>;

} // zoop
