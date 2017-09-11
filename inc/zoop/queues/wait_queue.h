#pragma once

#include <boost/config.hpp>
#include <memory>
#include <mutex>
#include <condition_variable>


namespace zoop
{
    template <typename Queue>
    class wait_queue : public Queue
    {
    public:
        wait_queue(Queue queue)
            : Queue{ std::move(queue) }
        {}

        template <typename OtherQueue>
        wait_queue(wait_queue<OtherQueue>&& other)
            : Queue{ std::move(other) }
        {}

        auto pop()
        {
            auto n = Queue::pop();
            return n ? n : do_wait();
        }

        void push(typename Queue::node_ptr n)
        {
            Queue::push(std::move(n));
            m_state->notify_one();
        }

    private:
        struct state : std::mutex, std::condition_variable
        {};

        BOOST_NOINLINE auto do_wait()
        {
            typename Queue::node_ptr n{};
            {
                std::unique_lock<std::mutex> lock{ *m_state };
                m_state->wait(lock, [&] { return !!(n = Queue::try_pop()); });
            }

            return n;
        }

        std::unique_ptr<state> m_state{ std::make_unique<state>() };
    };

} // zoop
