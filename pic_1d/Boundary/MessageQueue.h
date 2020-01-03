//
//  MessageQueue.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/3/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MessageQueue_h
#define MessageQueue_h

#include "../Macros.h"

#include <unordered_map>
#include <type_traits>
#include <optional>
#include <utility>
#include <future>
#include <thread>
#include <atomic>
#include <tuple>
#include <queue>

PIC1D_BEGIN_NAMESPACE
/// MPI-like message queue
///
template <class... Payloads>
class MessageQueue {
    MessageQueue(MessageQueue const&) = delete;
    MessageQueue &operator=(MessageQueue const&) = delete;

public:
    template <class Payload>
    class Queue;
    //
    template <class Payload>
    class Package {
        Payload payload;
        std::promise<void> p{};
    private:
        friend Queue<Payload>;
        Package(Payload&& payload) noexcept(std::is_nothrow_move_constructible_v<Payload>)
        : payload{std::move(payload)} {}
    public:
        ~Package() { p.set_value(); }
        [[nodiscard]] operator Payload() && noexcept(std::is_nothrow_move_constructible_v<Payload>) {
            return std::move(this->payload);
        }
    };
    //
    template <class Payload>
    class Queue {
        std::unordered_map<long, std::queue<Package<Payload>>> q{};
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
    private:
        template <class F, class... Args>
        auto sync(F&& f, Args&&... args) & noexcept(std::is_nothrow_invocable_v<F&&, Args&&...>) {
            static_assert(std::is_invocable_v<F&&, Args&&...>);
            do {} while (flag.test_and_set(std::memory_order_acquire));
            std::invoke_result_t<F&&, Args&&...> res = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            flag.clear(std::memory_order_release);
            return res;
        }
    public:
        [[nodiscard]] std::future<void> enqueue(long const key, Payload payload) {
            return sync([&q = this->q](long const key, Payload payload) {
                q[key].emplace(std::move(payload)).p.get_future();
            }, key, std::move(payload));
        }
        [[nodiscard]] Package<Payload> dequeue(long const key) {
            using Opt = std::optional<Package<Payload>>;
            auto f = [&map = this->q](long const key) -> Opt {
                if (map.count(key)) {
                    if (auto &q = map[key]; !q.empty()) {
                        Opt payload{std::move(q.front())};
                        q.pop();
                        return payload;
                    }
                }
                return std::nullopt;
            };
            //
            auto&& opt = sync(f, key);
            while (!opt) {
                std::this_thread::yield();
                opt = sync(f, key);
            }
            return *std::move(opt);
        }
    };
};
PIC1D_END_NAMESPACE

#endif /* MessageQueue_h */
