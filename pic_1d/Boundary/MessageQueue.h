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
/// MPI-like message (payload) passing machinary
///
template <class... Payloads>
class MessageQueue {
    static_assert((... && std::is_move_constructible_v<Payloads>), "Payloads should be move-constructible");
public:
    MessageQueue() = default;
    MessageQueue(MessageQueue const&) = delete;
    MessageQueue &operator=(MessageQueue const&) = delete;

public: // helpers
    template <class Payload>
    class Queue;
    using Ticket = std::future<void>;

    // payload wrapper ensuring delivery notification
    //
    template <class Payload>
    class Package {
        Payload payload;
        std::promise<void> promise{};
    private:
        friend Queue<Payload>;
    public:
        Package(Package&&) noexcept(std::is_nothrow_move_constructible_v<Payload>) = default;
        Package& operator=(Package&&) noexcept(std::is_nothrow_move_assignable_v<Payload>) = default;
        explicit Package(Payload&& payload) noexcept(std::is_nothrow_move_constructible_v<Payload>)
        : payload{std::move(payload)} {}
        //
        [[nodiscard]] operator Payload() && {
            Payload payload = std::move(this->payload);
            promise.set_value(); // notify of delivery
            return payload;
        }
        [[nodiscard]] Payload operator*() && {
            return std::move(*this);
        }
    };

    //per-payload-type message queue
    //
    template <class Payload>
    class Queue {
        std::unordered_map<long, std::queue<Package<Payload>>> map{};
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
    private:
        template <class F, class... Args>
        auto sync(F&& f, Args&&... args) & noexcept(std::is_nothrow_invocable_v<F&&, Args&&...>) {
            static_assert(std::is_invocable_v<F&&, Args&&...>);
            // acquire access
            do {} while (flag.test_and_set(std::memory_order_acquire));
            // do work
            std::invoke_result_t<F&&, Args&&...> res =
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            // relinquish access
            flag.clear(std::memory_order_release);
            // return, if any, result
            return res;
        }
    public:
        [[nodiscard]] Ticket operator()(long const key, Payload&& payload) & {
            return map[key].emplace(std::move(payload)).promise.get_future();
        }
        [[nodiscard]] Ticket enqueue(long const key, Payload&& payload) & {
            return sync(*this, key, std::move(payload));
        }
        //
        [[nodiscard]] std::optional<Package<Payload>> operator()(long const key) & {
            // only if the entry is present
            if (map.count(key)) {
                // only if at least one item in the queue
                if (auto &q = map[key]; !q.empty()) {
                    auto payload = std::move(q.front());
                    q.pop();
                    return std::move(payload);
                }
            }
            return std::nullopt;
        }
        [[nodiscard]] Package<Payload> dequeue(long const key) & {
            auto opt = sync(*this, key);
            while (!opt) {
                std::this_thread::yield();
                opt = sync(*this, key);
            }
            return *std::move(opt);
        }
    };

    //
    //
    union [[nodiscard]] Envelope {
    private:
        std::pair<int, int> pair;
        long id;
    public:
        constexpr Envelope(int addresser, int addressee) noexcept : pair{addresser, addressee} {}
        constexpr operator long() const noexcept { return id; }
    };

private:
    std::tuple<Queue<Payloads>...> pool{};

public: // sender
    template <long I, class Payload> [[nodiscard]]
    Ticket send(Envelope const envelope, Payload&& payload) {
        return std::get<I>(pool).enqueue(envelope, std::move(payload));
    }
    template <class Payload> [[nodiscard]]
    Ticket send(Envelope const envelope, Payload&& payload) {
        using T = Queue<std::remove_reference_t<Payload>>;
        return std::get<T>(pool).enqueue(envelope, std::move(payload));
    }

public: // receiver
    template <long I> [[nodiscard]]
    auto recv(Envelope const envelope) {
        return std::get<I>(pool).dequeue(envelope);
    }
    template <class Payload> [[nodiscard]]
    auto recv(Envelope const envelope) {
        using T = Queue<std::remove_reference_t<Payload>>;
        return std::get<T>(pool).dequeue(envelope);
    }
};

// not for public use
//
void test_message_queue();
PIC1D_END_NAMESPACE

#endif /* MessageQueue_h */
