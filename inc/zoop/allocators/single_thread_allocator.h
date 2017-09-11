#pragma once

#include "fixed_allocator.h"
#include <boost/config.hpp>
#include <memory>
#include <forward_list>


namespace zoop
{
    template <typename Buffer = std::unique_ptr<char[]>>
    class single_thread_allocator
    {
    public:
        explicit single_thread_allocator(std::size_t fixed_size)
            : m_cur{ m_list.emplace_after(m_list.before_begin(), fixed_size) },
              m_fixed_size{ fixed_size }
        {}

        void* allocate(std::size_t size)
        {
            if (size <= m_fixed_size)
            {
                if (auto ptr = m_cur->allocate(size, std::nothrow))
                {
                    return ptr;
                }

                return allocate_next(size);
            }

            throw std::invalid_argument{ "Cannot allocate more than the fixed size." };
        }

        void reset() noexcept
        {
            m_cur = m_list.begin();
        }

    private:
        using allocator_list = std::forward_list<fixed_allocator<false, Buffer>>;

        BOOST_NOINLINE void* allocate_next(std::size_t size)
        {
            if (std::next(m_cur) != m_list.end())
            {
                ++m_cur;
            }
            else
            {
                m_cur = m_list.emplace_after(m_cur, m_fixed_size);
            }

            return m_cur->allocate(size, std::nothrow);
        }

        allocator_list m_list;
        typename allocator_list::iterator m_cur;
        const std::size_t m_fixed_size;
    };

    extern template single_thread_allocator<>;

} // zoop
