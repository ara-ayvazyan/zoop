#pragma once

#include "fixed_allocator.h"


namespace zoop
{
    template <typename Buffer = std::unique_ptr<char[]>>
    using single_thread_fixed_allocator = fixed_allocator<false, Buffer>;

    extern template single_thread_fixed_allocator<>;

} // zoop
