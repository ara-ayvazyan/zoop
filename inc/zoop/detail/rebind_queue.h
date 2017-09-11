#pragma once


namespace zoop
{
    namespace detail
    {
        template <typename Queue, typename T, typename = void>
        struct rebind_queue;

        template <typename Queue, typename T>
        using rebind_queue_t = typename rebind_queue<Queue, T>::type;

        template <template <typename, typename...> typename Queue, typename X, typename... R, typename T>
        struct rebind_queue<Queue<X, R...>, T, std::enable_if_t<!std::is_same<X, typename Queue<X, R...>::value_type>::value>>
        {
            using type = Queue<rebind_queue_t<X, T>, R...>;
        };

        template <template <typename, typename...> typename Queue, typename X, typename... R, typename T>
        struct rebind_queue<Queue<X, R...>, T, std::enable_if_t<std::is_same<X, typename Queue<X, R...>::value_type>::value>>
        {
            using type = Queue<T, R...>;
        };

    } // detail
} // zoop
