#pragma once

#include "embedded_allocator.h"
#include <zoop/allocators/single_thread_allocator.h>
#include <zoop/allocators/stl_allocator.h>
#include <memory>


namespace zoop
{
    template <typename T, typename Size = std::integral_constant<std::size_t, 128>, typename Alloc = single_thread_allocator<>>
    class local_shared_ptr : public embedded_allocator<T, Alloc>
    {
    public:
        template <typename... Args>
        local_shared_ptr(Args&&... args)
            : embedded_allocator<T, Alloc>{
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<Args>(args)...),
                std::forward_as_tuple(Size::value) }
        {}

        auto operator->() const noexcept
        {
            return &operator*();
        }

        auto operator->() noexcept
        {
            return &operator*();
        }

        auto& operator*() const noexcept
        {
            return std::get<0>(*this);
        }

        auto& operator*() noexcept
        {
            return std::get<0>(*this);
        }

        template <typename Deleter>
        friend std::shared_ptr<T> share(handle<local_shared_ptr, Deleter> h)
        {
            if (auto p = h.get())
            {
                auto ptr = &std::get<0>(*p);
                return { share(std::move(h), stl_allocator<T, Alloc>{ std::get<1>(*p) }), ptr };
            }

            return {};
        }
    };

} // zoop
