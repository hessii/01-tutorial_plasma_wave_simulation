//
//  MessageDispatch.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/3/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MessageDispatch_h
#define MessageDispatch_h

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
class MessageDispatch {
    static_assert((... && std::is_move_constructible_v<Payloads>), "Payloads should be move-constructible");
public:
    MessageDispatch() = default;
    MessageDispatch(MessageDispatch const&) = delete;
    MessageDispatch &operator=(MessageDispatch const&) = delete;

private:
    template <class Payload> class Queue;

public:
    // payload tracker
    //
    template <class Payload>
    class [[nodiscard]] Ticket {
        friend Queue<Payload>;
        std::future<void> future;
        Ticket(std::promise<void>& p) noexcept : future{p.get_future()} {}
    public:
        Ticket(Ticket&&) noexcept = default;
        Ticket& operator=(Ticket&&) noexcept = default;
        void wait() { return future.get(); } // wait for delivery
    };

    // payload wrapper
    //
    template <class Payload>
    class [[nodiscard]] Package {
        friend Queue<Payload>;
        Payload payload;
        std::promise<void> promise{};
    public:
        Package(Package&&) noexcept(std::is_nothrow_move_constructible_v<Payload>) = default;
        Package& operator=(Package&&) noexcept(std::is_nothrow_move_assignable_v<Payload>) = default;

        // pack payload
        //
        Package(Payload&& payload) noexcept(std::is_nothrow_move_constructible_v<Payload>) : payload{std::move(payload)} {}

        // unpack payload
        //
        [[nodiscard]] operator Payload() && {
            Payload payload = std::move(this->payload);
            promise.set_value(); // notify of delivery
            return payload;
        }
        [[nodiscard]] Payload operator*() && {
            return static_cast<Payload>(std::move(*this));
        }
    };

private:
    //per-Payload message queue
    //
    template <class Payload>
    class Queue {
        std::unordered_map<long, std::queue<Package<Payload>>> map{};
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
    private:
        template <class... Args>
        static auto sync(Queue& q, Args&&... args) noexcept(std::is_nothrow_invocable_v<Queue&, Args&&...>) {
            static_assert(std::is_invocable_v<Queue&, Args&&...>);
            // acquire access
            do {} while (q.flag.test_and_set(std::memory_order_acquire));
            // do work
            std::invoke_result_t<Queue&, Args&&...> res =
            std::invoke(q, std::forward<Args>(args)...);
            // relinquish access
            q.flag.clear(std::memory_order_release);
            // return, if any, result
            return res;
        }
    public:
        [[nodiscard]] Ticket<Payload> operator()(long const key, Payload&& payload) & {
            return map[key].emplace(std::move(payload)).promise;
        }
        [[nodiscard]] Ticket<Payload> enqueue(long const key, Payload&& payload) & {
            return sync(*this, key, std::move(payload));
        }
        //
        [[nodiscard]] std::optional<Package<Payload>> operator()(long const key) & {
            // only if the entry is present
            if (map.count(key)) {
                // only if at least one item in the queue
                if (auto &q = map[key]; !q.empty()) {
                    Package<Payload> payload = std::move(q.front());
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

public:
    // envelope
    //
    union [[nodiscard]] Envelope {
    private:
        std::pair<int, int> pair;
        long id;
    public:
        constexpr Envelope(int addresser, int addressee) noexcept : pair{addresser, addressee} {}
        constexpr operator long() const noexcept { return id; }
    };

    // send
    //
    template <long I, class Payload> [[nodiscard]]
    auto send(Envelope const envelope, Payload&& payload) {
        return std::get<I>(pool).enqueue(envelope, std::move(payload));
    }
    template <class Payload> [[nodiscard]]
    auto send(Envelope const envelope, Payload&& payload) {
        using T = Queue<std::remove_reference_t<Payload>>;
        return std::get<T>(pool).enqueue(envelope, std::move(payload));
    }

    // receive
    //
    template <long I> [[nodiscard]]
    auto recv(Envelope const envelope) {
        return std::get<I>(pool).dequeue(envelope);
    }
    template <class Payload> [[nodiscard]]
    auto recv(Envelope const envelope) {
        using T = Queue<std::remove_reference_t<Payload>>;
        return std::get<T>(pool).dequeue(envelope);
    }

private:
    std::tuple<Queue<Payloads>...> pool{};
};

// not for public use
//
void test_message_queue();
PIC1D_END_NAMESPACE

#endif /* MessageDispatch_h */
