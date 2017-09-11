#pragma once

#include <boost/lockfree/stack.hpp>
#include <memory>


namespace zoop
{
    template <typename T>
    class lockfree_stack
    {
    public:
        using value_type = T;

        using node_ptr = std::shared_ptr<T>;

        lockfree_stack() = default;

        template <typename U>
        lockfree_stack(lockfree_stack<U>&& /*other*/)
            : lockfree_stack{}
        {}

        template <typename... Args>
        static auto allocate(Args&&... args)
        {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }

        template <typename... Args>
        void reserve(std::size_t n, Args&&... args)
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                push(allocate(args...));
            }
        }

        auto try_pop()
        {
            node_ptr n;
            return m_stack->pop(n) ? std::move(n) : nullptr;
        }

        auto pop()
        {
            return try_pop();
        }

        void push(node_ptr n)
        {
            m_stack->push(std::move(n));
        }

    private:
        using stack = boost::lockfree::stack<node_ptr>;

        std::unique_ptr<stack> m_stack{ std::make_unique<stack>(0) };
    };

} // zoop
