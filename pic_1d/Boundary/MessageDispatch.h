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
#include <functional>
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
    using payload_tuple = std::tuple<Payloads...>;
    class Communicator;

    // payload tracker
    //
    class [[nodiscard]] Ticket {
        template <class Payload>
        friend class Queue;
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

    private:
        Package(Payload&& payload) noexcept(std::is_nothrow_move_constructible_v<Payload>) : payload{std::move(payload)} {}

    public:
        struct Guard {
            std::promise<void>& promise;
            ~Guard() { promise.set_value(); }
        };
        template <class F, class... RestArgs>
        auto unpack(F&& f, RestArgs&&... rest_args) && {
            static_assert(std::is_invocable_v<F&&, Payload&&, RestArgs&&...>);
            // notify of delivery on leaving scope
            Guard const g{promise};
            // invoke the callable with payload as its first argument
            return std::invoke(std::forward<F>(f), std::move(this->payload), std::forward<RestArgs>(rest_args)...);
        }
        //
        [[nodiscard]] operator Payload() && {
            return static_cast<Package&&>(*this).unpack([](Payload payload){ return payload; });
        }
        [[nodiscard]] Payload operator*() && {
            return static_cast<Package&&>(*this).unpack([](Payload payload){ return payload; });
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
        class Guard {
            std::atomic_flag& flag;
        public:
            Guard(std::atomic_flag& f) noexcept : flag{f} { // acquire access
                do {} while (flag.test_and_set(std::memory_order_acquire));
            }
            ~Guard() noexcept { flag.clear(std::memory_order_release); } // relinquish access
        };
        template <class... Args>
        static auto sync(Queue& q, Args&&... args) noexcept(std::is_nothrow_invocable_v<Queue&, Args&&...>) {
            static_assert(std::is_invocable_v<Queue&, Args&&...>);
            Guard const g = q.flag; // synchronous access to queue
            return std::invoke(q, std::forward<Args>(args)...);
        }
    public:
        [[nodiscard]] Ticket operator()(long const key, Payload&& payload) & {
            return map[key].emplace(Package<Payload>{std::move(payload)}).promise;
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
                    auto payload = std::move(q.front()); // must take the ownership of payload
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
        std::pair<int, int> int_pair;
        std::pair<unsigned, unsigned> uint_pair;
        long id;
    public:
        constexpr Envelope(int addresser, int addressee) noexcept : int_pair{addresser, addressee} {}
        constexpr Envelope(unsigned addresser, unsigned addressee) noexcept : uint_pair{addresser, addressee} {}
        constexpr operator long() const noexcept { return id; }
    };
    static_assert(sizeof(Envelope) == sizeof(long));

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

    // communicator
    //
    template <class Int>
    [[nodiscard]] Communicator comm(Int const address) & noexcept {
        static_assert(std::is_same_v<Int, int> || std::is_same_v<Int, unsigned>);
        return {this, address};
    }

private:
    std::tuple<Queue<Payloads>...> pool{};
};

/// MPI-like inter-thread communicator
///
template <class... Payloads>
class MessageDispatch<Payloads...>::Communicator {
    friend MessageDispatch<Payloads...>;
    MessageDispatch<Payloads...> *dispatch;
    long address;
    //
    template <class Int>
    Communicator(MessageDispatch<Payloads...> *dispatch, Int const address) noexcept : dispatch{dispatch}, address{address} {
        static_assert(std::is_same_v<Int, int> || std::is_same_v<Int, unsigned>);
    }

public:
    Communicator() noexcept = default;
    using message_dispatch_t = MessageDispatch<Payloads...>;
    message_dispatch_t& message_dispatch() noexcept { return *dispatch; }
    int rank() const noexcept { return static_cast<int>(address); }

    // send
    //
    template <long I, class To, class Payload> [[nodiscard]]
    auto send(To const to, Payload&& payload) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->template send<I>({static_cast<To>(address), to}, std::move(payload));
    }
    template <class To, class Payload> [[nodiscard]]
    auto send(To const to, Payload&& payload) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->send({static_cast<To>(address), to}, std::move(payload));
    }

    // receive
    //
    template <long I, class From> [[nodiscard]]
    auto recv(From const from) const {
        static_assert(std::is_same_v<From, int> || std::is_same_v<From, unsigned>);
        return dispatch->template recv<I>({from, static_cast<From>(address)});
    }
    template <class Payload, class From> [[nodiscard]]
    auto recv(From const from) const {
        static_assert(std::is_same_v<From, int> || std::is_same_v<From, unsigned>);
        return dispatch->template recv<Payload>({from, static_cast<From>(address)});
    }
};

// not for public use
//
void test_message_queue();
void test_inter_thread_comm();
PIC1D_END_NAMESPACE

#endif /* MessageDispatch_h */
