#pragma once

#include "detail/object_pool_impl.h"
#include <boost/config.hpp>
#include <memory>


namespace zoop
{
    template <typename Queue, typename Reset>
    class object_pool
    {
    private:
        using impl = detail::object_pool_impl<Queue, Reset>;

    public:
        using value_type = typename impl::value_type;

        template <typename U>
        using handle_for = typename impl::handle_for<U>;

        using handle = handle_for<value_type>;

        explicit object_pool(Reset reset, Queue queue = {})
            : m_impl{ std::make_shared<impl>(std::move(reset), std::move(queue)) }
        {}

        template <typename U = value_type>
        BOOST_NOINLINE auto take()
        {
            return m_impl->template take<U>(m_impl);
        }

        auto operator->() const noexcept
        {
            return &m_impl->queue();
        }

    private:
        std::shared_ptr<impl> m_impl;
    };


    template <typename T, typename Deleter>
    inline std::shared_ptr<T> share(handle<T, Deleter> h)
    {
        if (auto ptr = h.get())
        {
            return { std::make_shared<decltype(h)>(std::move(h)), ptr };
        }

        return {};
    }

    template <typename T, typename Deleter, typename Alloc>
    inline std::shared_ptr<T> share(handle<T, Deleter> h, const Alloc& alloc)
    {
        if (auto ptr = h.get())
        {
            return { std::allocate_shared<decltype(h)>(alloc, std::move(h)), ptr };
        }

        return {};
    }

} // zoop
