#pragma once


namespace zoop
{
    class null_reset
    {
    public:
        template <typename T>
        void operator()(T&&) const noexcept
        {}
    };

} // zoop
