#pragma once

#include <memory>


namespace zoop
{
    template <typename T, typename Alloc>
    class stl_allocator
    {
    public:
        using value_type = T;

        stl_allocator(Alloc& alloc) noexcept
            : m_alloc{ alloc }
        {}

        stl_allocator(std::shared_ptr<Alloc> alloc) noexcept
            : m_alloc{ *alloc },
              m_inst{ std::move(alloc) }
        {}

        template <typename U>
        stl_allocator(const stl_allocator<U, Alloc>& other) noexcept
            : m_alloc{ other.m_alloc },
              m_inst{ other.m_inst }
        {}

        T* allocate(std::size_t n)
        {
            return static_cast<T*>(m_alloc.allocate(sizeof(T) * n));
        }

        void deallocate(T*, std::size_t) noexcept
        {}

        template <typename U>
        friend bool operator==(const stl_allocator& a1, const stl_allocator<U, Alloc>& a2) noexcept
        {
            return &a1.m_alloc == &a2.m_alloc;
        }

        template <typename U>
        friend bool operator!=(const stl_allocator& a1, const stl_allocator<U, Alloc>& a2) noexcept
        {
            return !(a1 == a2);
        }

    private:
        template <typename U, typename OtherAlloc>
        friend class stl_allocator;

        Alloc& m_alloc;
        std::shared_ptr<Alloc> m_inst;
    };

} // zoop
