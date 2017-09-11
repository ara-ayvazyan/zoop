#pragma once

#include <boost/optional.hpp>
#include <memory>
#include <atomic>
#include <limits>
#include <type_traits>


namespace zoop
{
    namespace detail
    {
        template <typename T>
        class overflow_counter_base
        {
        public:
            static_assert(std::is_integral<T>::value, "Counter must be of integral type");

            overflow_counter_base(T max_count = (std::numeric_limits<T>::max)()) noexcept
                : m_max_value{ max_count }
            {}

            T max_value() const noexcept
            {
                return m_max_value;
            }

        private:
            const T m_max_value;
        };


        template <typename T, bool IsThreadSafe>
        class overflow_counter;


        template <typename T>
        class overflow_counter<T, true> : public overflow_counter_base<T>
        {
        public:
            using overflow_counter_base<T>::overflow_counter_base;

            T value() const noexcept
            {
                return m_value->load(std::memory_order::memory_order_relaxed);
            }

            void value(T val) noexcept
            {
                m_value->store(val, std::memory_order::memory_order_release);
            }

            boost::optional<T> try_add(T n) noexcept
            {
                if (n <= this->max_value())
                {
                    const auto max_val = this->max_value() - n;

                    for (auto value = m_value->load(std::memory_order::memory_order_relaxed); value <= max_val; )
                    {
                        if (m_value->compare_exchange_weak(
                            value,
                            value + n,
                            std::memory_order::memory_order_release,
                            std::memory_order::memory_order_relaxed))
                        {
                            return value;
                        }
                    }
                }

                return {};
            }

        private:
            std::unique_ptr<std::atomic<T>> m_value{ std::make_unique<std::atomic<T>>() };
        };


        template <typename T>
        class overflow_counter<T, false> : public overflow_counter_base<T>
        {
        public:
            using overflow_counter_base<T>::overflow_counter_base;

            T value() const noexcept
            {
                return m_value;
            }

            void value(T val) noexcept
            {
                m_value = val;
            }

            boost::optional<T> try_add(T n) noexcept
            {
                if (n <= this->max_value() && m_value <= this->max_value() - n)
                {
                    auto value = m_value;
                    m_value += n;
                    return value;
                }

                return {};
            }

        private:
            T m_value{};
        };

    } // detail
} // zoop
