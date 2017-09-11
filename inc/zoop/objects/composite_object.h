#pragma once

#include <tuple>
#include <type_traits>
#include <initializer_list>


namespace zoop
{
    namespace detail
    {
        template <typename Tuple>
        class composite_object_impl : public Tuple
        {
        public:
            using Tuple::Tuple;

            template <typename... Reset>
            static auto make_reset(Reset&&... reset)
            {
                return [reset = std::make_tuple(std::forward<Reset>(reset)...)](composite_object_impl& obj) mutable
                {
                    obj.reset(reset);
                };
            }

        private:
            template <typename... Reset>
            void reset(std::tuple<Reset...>& r)
            {
                static_assert(std::tuple_size<Tuple>::value == sizeof...(Reset), "Incorrect number of reset arguments is passed");

                reset(r, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
            }

            template <typename... Reset, std::size_t... I>
            void reset(std::tuple<Reset...>& r, std::index_sequence<I...>)
            {
                std::initializer_list<int>{ (std::get<I>(r)(std::get<I>(*this)), 0)... };
            }
        };

    } // detail

    template <typename... T>
    using composite_object = detail::composite_object_impl<std::conditional_t<sizeof...(T) == 2, std::pair<T...>, std::tuple<T...>>>;

} // zoop
