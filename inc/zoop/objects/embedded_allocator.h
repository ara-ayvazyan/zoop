#pragma once

#include "composite_object.h"


namespace zoop
{
    template <typename T, typename Alloc>
    class embedded_allocator : public composite_object<T, Alloc>
    {
    public:
        using composite_object<T, Alloc>::composite_object;

        template <typename Reset>
        static auto make_reset(Reset&& reset)
        {
            return composite_object<T, Alloc>::make_reset(std::forward<Reset>(reset), [](Alloc& a) { a.reset(); });
        }
    };

} // zoop
