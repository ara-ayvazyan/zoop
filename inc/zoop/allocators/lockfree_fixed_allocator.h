#pragma once

#include "fixed_allocator.h"


namespace zoop
{
    template <typename Buffer = std::unique_ptr<char[]>>
    using lockfree_fixed_allocator = fixed_allocator<true, Buffer>;

    extern template lockfree_fixed_allocator<>;

} // zoop
