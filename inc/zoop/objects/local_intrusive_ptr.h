#pragma once

#include <zoop/objects/composite_object.h>
#include <zoop/null_reset.h>
#include <boost/intrusive_ptr.hpp>
#include <memory>
#include <atomic>
#include <type_traits>


namespace zoop
{
    template <typename T, typename Size = std::integral_constant<std::size_t, 64>, typename Counter = std::atomic_size_t>
    class local_intrusive_ptr : public composite_object<T, std::aligned_storage_t<Size::value>>
    {
    private:
        template <typename Deleter>
        struct intrusive_ptr;

    public:
        template <typename... Args>
        local_intrusive_ptr(Args&&... args)
            : composite_object<T, std::aligned_storage_t<Size::value>>{
                std::piecewise_construct,
                std::forward_as_tuple(std::forward<Args>(args)...),
                std::forward_as_tuple() }
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

        template <typename Reset>
        static auto make_reset(Reset&& reset)
        {
            return composite_object<T, std::aligned_storage_t<Size::value>>::make_reset(std::forward<Reset>(reset), [](auto&) {});
        }

        template <typename Deleter>
        friend auto share(handle<local_intrusive_ptr, Deleter> h)
        {
            return h ? intrusive_ptr<Deleter>{ std::move(h) } : nullptr;
        }

    private:
        template <typename Deleter>
        class handle_holder
        {
        public:
            handle_holder(handle<local_intrusive_ptr, Deleter> h) noexcept
                : m_handle{ std::move(h) }
            {}

            friend void intrusive_ptr_add_ref(handle_holder* p) noexcept
            {
                p->add_ref();
            }

            friend void intrusive_ptr_release(handle_holder* p) noexcept
            {
                p->release();
            }

        private:
            void add_ref() noexcept
            {
                ++m_refs;
            }

            void release() noexcept
            {
                if (--m_refs == 0)
                {
                    m_handle = {};
                }
            }

            Counter m_refs{ 1 };
            handle<local_intrusive_ptr, Deleter> m_handle;
        };

        class shallow_ptr : public handle<T, null_reset>
        {
        public:
            using handle<T, null_reset>::handle;

            shallow_ptr(shallow_ptr&& other) = default;
            shallow_ptr& operator=(shallow_ptr&& other) = default;

            shallow_ptr(const shallow_ptr& other) noexcept
                : handle<T, null_reset>{ other.get() }
            {}

            shallow_ptr& operator=(const shallow_ptr& other) noexcept
            {
                handle<T, null_reset>::reset(other.get());
                return *this;
            }
        };

        template <typename Deleter>
        class intrusive_ptr : public shallow_ptr
        {
        public:
            static_assert(sizeof(handle_holder<Deleter>) <= Size::value, "Insufficient buffer size");

            using shallow_ptr::shallow_ptr;

            intrusive_ptr(handle<local_intrusive_ptr, Deleter> h) noexcept
                : shallow_ptr{ &*h },
                  m_holder{ new (&std::get<1>(*h.get())) handle_holder<Deleter>{ std::move(h) }, false }
            {}

        private:
            boost::intrusive_ptr<handle_holder<Deleter>> m_holder;
        };
    };

} // zoop
