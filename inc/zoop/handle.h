#pragma once

#include <memory>
#include <type_traits>


namespace zoop
{
    namespace detail
    {
        template <typename T>
        struct check_type : std::true_type
        {};

        template <typename T, typename = void>
        struct has_ptr_access : std::false_type
        {};

        template <typename T>
        struct has_ptr_access<T, std::enable_if_t<check_type<decltype(std::declval<T>().operator->())>::value>> : std::true_type
        {};

        template <typename T, typename = void>
        struct has_deref_access : std::false_type
        {};

        template <typename T>
        struct has_deref_access<T, std::enable_if_t<check_type<decltype(std::declval<T>().operator*())>::value>> : std::true_type
        {};

    } // detail


    template <typename T, typename Deleter>
    class handle : private std::unique_ptr<T, Deleter>
    {
    private:
        using base = std::unique_ptr<T, Deleter>;

    public:
        using base::base;

        using base::get;
        using base::reset;
        using base::operator bool;

        template <typename U = T, std::enable_if_t<detail::has_ptr_access<U>::value>* = nullptr>
        decltype(auto) operator->() const noexcept
        {
            return this->get()->operator->();
        }

        template <typename U = T, std::enable_if_t<!detail::has_ptr_access<U>::value>* = nullptr>
        decltype(auto) operator->() const noexcept
        {
            return base::operator->();
        }

        template <typename U = T, std::enable_if_t<detail::has_deref_access<U>::value>* = nullptr>
        decltype(auto) operator*() const
        {
            return this->get()->operator*();
        }

        template <typename U = T, std::enable_if_t<!detail::has_deref_access<U>::value>* = nullptr>
        decltype(auto) operator*() const
        {
            return base::operator*();
        }

        friend bool operator==(const handle& h1, const handle& h2)
        {
            return h1.get() == h2.get();
        }

        friend bool operator!=(const handle& h1, const handle& h2)
        {
            return !(h1 == h2);
        }
    };

} // zoop
