#pragma once

#include <windows.h>
#include <memory>
#include <cassert>
#include <malloc.h>


namespace zoop
{
    namespace detail
    {
        class aligned_base
        {
        public:
            static void* operator new(std::size_t size)
            {
                return ::_aligned_malloc(size, MEMORY_ALLOCATION_ALIGNMENT);
            }

            static void operator delete(void* ptr)
            {
                ::_aligned_free(ptr);
            }
        };

    } // detail


    template <typename T>
    class lockfree_win32_list
    {
    private:
        class deleter;

    public:
        using value_type = T;

        using node_ptr = std::unique_ptr<T, deleter>;

        lockfree_win32_list() = default;

        template <typename U>
        lockfree_win32_list(lockfree_win32_list<U>&& /*other*/)
            : lockfree_win32_list{}
        {}

        template <typename... Args>
        static auto allocate(Args&&... args)
        {
            return make_node_ptr(std::make_unique<node>(std::forward<Args>(args)...));
        }

        template <typename... Args>
        void reserve(std::size_t n, Args&&... args)
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                push(allocate(args...));
            }
        }

        auto try_pop() noexcept
        {
            return make_node_ptr(std::unique_ptr<node>{ static_cast<node*>(::InterlockedPopEntrySList(*m_head)) });
        }

        auto pop()
        {
            return try_pop();
        }

        void push(node_ptr n) noexcept
        {
            assert(n);
            auto np = n.get_deleter().release();
            ::InterlockedPushEntrySList(*m_head, np);
            n.release();
        }

        std::size_t size() const noexcept
        {
            return ::QueryDepthSList(*m_head);
        }

    private:
        class head : public detail::aligned_base
        {
        public:
            head() noexcept
            {
                ::InitializeSListHead(&m_header);
            }

            operator ::PSLIST_HEADER() noexcept
            {
                return &m_header;
            }

        private:
            ::SLIST_HEADER m_header;
        };

        class node : public ::SLIST_ENTRY, public detail::aligned_base
        {
        public:
            template <typename... Args>
            explicit node(Args&&... args)
                : m_value{ std::forward<Args>(args)... }
            {}

            T& value() noexcept
            {
                return m_value;
            }

        private:
            T m_value;
        };

        class deleter
        {
        public:
            deleter() = default;

            explicit deleter(std::unique_ptr<node> n) noexcept
                : m_node{ std::move(n) }
            {}

            void operator()(T* obj)
            {
                assert(obj);
                (void)obj;
                m_node = {};
            }

            node* release() noexcept
            {
                return m_node.release();
            }

        private:
            std::unique_ptr<node> m_node;
        };

        static auto make_node_ptr(std::unique_ptr<node> n) noexcept
        {
            auto ptr = n ? &n->value() : nullptr;
            return node_ptr{ ptr, deleter{ std::move(n) } };
        }

        std::unique_ptr<head> m_head{ std::make_unique<head>() };
    };

} // zoop
