#pragma once

#include <zoop/detail/overflow_counter.h>
#include <memory>
#include <type_traits>


namespace zoop
{
    template <bool IsThreadSafe, typename Buffer = std::unique_ptr<char[]>>
    class fixed_allocator;


    template <bool IsThreadSafe>
    class fixed_allocator<IsThreadSafe, void>
    {
    public:
        fixed_allocator(char* buffer, std::size_t capacity) noexcept
            : m_offset{ capacity },
              m_ptr{ buffer }
        {}

        template <typename T, std::size_t N, std::enable_if_t<std::is_trivially_copyable<T>::value>* = nullptr>
        explicit fixed_allocator(T (&buffer)[N]) noexcept
            : fixed_allocator{ reinterpret_cast<char*>(buffer), sizeof(T) * N }
        {}

        void* allocate(std::size_t size, std::nothrow_t) noexcept
        {
            if (auto offset = m_offset.try_add(size))
            {
                return m_ptr + *offset;
            }

            return nullptr;
        }

        void* allocate(std::size_t size)
        {
            if (auto ptr = allocate(size, std::nothrow))
            {
                return ptr;
            }

            throw std::bad_alloc{};
        }

        void reset() noexcept
        {
            m_offset.value(0);
        }

    private:
        using offset_counter = detail::overflow_counter<std::size_t, IsThreadSafe>;

        offset_counter m_offset;
        char* const m_ptr;
    };


    template <bool IsThreadSafe, typename Buffer>
    class fixed_allocator : public fixed_allocator<IsThreadSafe, void>
    {
    public:
        explicit fixed_allocator(std::size_t capacity)
            : fixed_allocator{ Buffer{ new char[capacity] }, capacity }
        {}

        fixed_allocator(Buffer buffer, std::size_t capacity) noexcept
            : fixed_allocator<IsThreadSafe, void>{ buffer.get(), capacity },
              m_buffer{ std::move(buffer) }
        {}

    private:
        Buffer m_buffer;
    };

} // zoop
