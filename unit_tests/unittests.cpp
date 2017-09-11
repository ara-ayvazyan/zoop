#include "stdafx.h"

#include <zoop/object_pool.h>
#include <zoop/object_pool_builder.h>
#include <zoop/queues/growable_queue.h>
#include <zoop/queues/limited_queue.h>
#include <zoop/queues/limited_throw_queue.h>
#include <zoop/queues/wait_queue.h>
#include <zoop/queues/lockfree_win32_list.h>
#include <zoop/queues/lockfree_stack.h>
#include <zoop/allocators/lockfree_allocator.h>
#include <zoop/allocators/single_thread_allocator.h>
#include <zoop/allocators/stl_allocator.h>
#include <zoop/objects/composite_object.h>
#include <zoop/objects/embedded_allocator.h>
#include <zoop/objects/local_shared_ptr.h>
#include <zoop/objects/local_intrusive_ptr.h>

#include <list>
#include <vector>


template <typename T, typename Reset>
auto make_pool(Reset&& reset)
{
    auto new_reset = [reset = std::forward<Reset>(reset)](auto& pair)
    {
        reset(pair.first);
        pair.second.reset();
    };

    return zoop::object_pool_builder<zoop::lockfree_win32_list<std::pair<T, zoop::single_thread_allocator<>>>, decltype(new_reset)>{ {}, std::move(new_reset) }
        .wrap_queue<zoop::limited_queue>(std::size_t(10))
        .wrap_queue_bind<zoop::growable_queue>(
            std::piecewise_construct,
            std::make_tuple(),
            std::make_tuple(128))
        .build();
}

template <typename T, typename Reset = zoop::null_reset>
auto make_fixed_pool(Reset&& reset = {})
{
    zoop::lockfree_win32_list<T> list;
    list.reserve(100);

    return zoop::object_pool_builder<decltype(list), Reset>{ std::move(list), std::forward<Reset>(reset) }
        //.wrap_queue<zoop::growable_queue>()
        .build();
}


template <typename T, typename Reset = zoop::null_reset>
auto make_intrusive_ptr_pool(Reset&& reset = {})
{
    return zoop::object_pool_builder<zoop::lockfree_win32_list<T>, Reset>{ {}, std::forward<Reset>(reset) }
        .wrap_object<zoop::local_intrusive_ptr>()
        .wrap_queue<zoop::limited_queue>(std::size_t(1))
        .wrap_queue_bind<zoop::growable_queue>(128)
        .build();
}

int main()
{
    {
        auto pool = make_intrusive_ptr_pool<int>();
        auto h1 = share(pool.take());
        auto h2 = h1;
        auto h3 = share(pool.take());
        h3 = std::move(h1);
    }

    {
        auto pool = make_fixed_pool<int>();
        auto h1 = pool.take();
        auto h2 = pool.take();
    }

    static_assert(std::is_same<
        zoop::detail::rebind_queue_t<zoop::growable_queue<zoop::lockfree_stack<int>>, float>,
        zoop::growable_queue<zoop::lockfree_stack<float>>>::value, "?");

    auto pool2 = make_pool<std::vector<int>>([](auto&& v) { v.clear(); });

    auto item = pool2.take();

    auto shared_item = share(std::move(item), zoop::stl_allocator<void, zoop::single_thread_allocator<>>{ item->second });

    auto reset = [](auto& alloc) { alloc.reset(); };

    auto pool = zoop::object_pool_builder<zoop::lockfree_win32_list<zoop::lockfree_allocator<>>, decltype(reset)>{ {}, std::move(reset) }
        .wrap_object<zoop::local_shared_ptr>()
        .wrap_queue<zoop::limited_queue>(10u)
        .wrap_queue_bind<zoop::growable_queue>(4096)
        //.bind<shared<zoop::lockfree_allocator>>()
        //.reset(shared<zoop::lockfree_allocator>::make_reset([](auto& alloc) { alloc.reset(); }))
        
        .build();

    /*{
        zoop::fixed_allocator a{ 100 };
        a.allocate(50);
        a.allocate(50);
        a.allocate(1);
    }*/

    auto hh = pool.take();
    hh->allocate(1);
    hh == hh;

    //auto h = pool.take(1024);

    {
        auto a = share(pool.take());

        zoop::stl_allocator<int[256], zoop::lockfree_allocator<>> alloc(a);

        std::list<int[256], zoop::stl_allocator<int[256], zoop::lockfree_allocator<>>> v(alloc);
        v.emplace_back();
        v.emplace_back();
        v.emplace_back();
    }
    {
        auto a = share(pool.take());
        std::list<int[256], zoop::stl_allocator<int[256], zoop::lockfree_allocator<>>> v{ a };
        v.emplace_back();
        v.emplace_back();
        v.emplace_back();
    }

    pool->size();
    pool->capacity();
    pool->max_size();

    /*zoop::fixed_allocator a{ 100 };
    a.allocate(80);
    a.allocate(80);*/

    return 0;
}
