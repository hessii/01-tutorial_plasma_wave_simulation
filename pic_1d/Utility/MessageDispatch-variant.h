//
//  MessageDispatch-variant.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/10/20.
//  Copyright © 2020 kyungguk.com. All rights reserved.
//

#ifndef MessageDispatch_variant_h
#define MessageDispatch_variant_h

#include "../Macros.h"

#include <type_traits>
#include <functional>
#include <optional>
#include <utility>
#include <memory>
#include <thread>
#include <atomic>
#include <tuple>
#include <queue>
#include <map>

PIC1D_BEGIN_NAMESPACE
namespace ver_variant {
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
    template <class Payload> class [[nodiscard]] Package;
    using payload_tuple = std::tuple<Payloads...>;
    class Communicator;

    // payload tracker
    //
    class [[nodiscard]] Ticket {
        template <class Payload>
        friend class Package;
        std::unique_ptr<std::atomic_flag> flag;
        Ticket(std::unique_ptr<std::atomic_flag> f) noexcept : flag{std::move(f)} { flag->test_and_set(); }
    public:
        Ticket() noexcept = default;
        Ticket(Ticket&&) noexcept = default;
        Ticket& operator=(Ticket&&) noexcept = default;
        void wait() && noexcept { // wait for delivery
            // deliberately not check flag presence; multiple calls are ill-formed
            while (flag->test_and_set(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            flag.reset();
        }
    };

    // payload wrapper
    //
    template <class Payload>
    class [[nodiscard]] Package {
        friend Queue<Payload>;
        Payload payload;
        std::atomic_flag* flag;
    public:
        Package(Package&&) noexcept(std::is_nothrow_move_constructible_v<Payload>) = default;
    private:
        Package(Payload&& payload) noexcept(std::is_nothrow_move_constructible_v<Payload>) : payload{std::move(payload)} {}
        operator Ticket() & { return std::unique_ptr<std::atomic_flag>{flag = new std::atomic_flag}; }
        //
        class Guard {
            std::atomic_flag& flag;
        public:
            Guard(std::atomic_flag& flag) noexcept : flag{flag} {}
            ~Guard() noexcept { flag.clear(std::memory_order_release); } // notify of delivery
            template <class F, class... Args>
            auto invoke(F&& f, Args&&... args) const // invoke the callable
            noexcept(std::is_nothrow_invocable_v<F&&, Args&&...>)
            ->               std::invoke_result_t<F&&, Args&&...> {
                static_assert(std::is_invocable_v<F&&, Args&&...>);
                return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            }
        };
    public:
        template <class F, class... RestArgs>
        auto unpack(F&& f, RestArgs&&... rest_args) &&
        noexcept(std::is_nothrow_invocable_v<F&&, Payload&&, RestArgs&&...>)
        ->               std::invoke_result_t<F&&, Payload&&, RestArgs&&...> {
            static_assert(std::is_invocable_v<F&&, Payload&&, RestArgs&&...>);
            // invoke the callable with payload as its first argument
            return Guard{*flag}.invoke(std::forward<F>(f), std::move(this->payload), std::forward<RestArgs>(rest_args)...);
        }
        //
        [[nodiscard]] operator Payload() && noexcept(std::is_nothrow_move_constructible_v<Payload>) {
            static_assert(std::is_move_constructible_v<Payload>);
            return Guard{*flag}.invoke([&payload = this->payload]() noexcept(std::is_nothrow_move_constructible_v<Payload>) { return std::move(payload); });
        }
        [[nodiscard]] Payload operator*() && noexcept(std::is_nothrow_move_constructible_v<Payload>) {
            static_assert(std::is_move_constructible_v<Payload>);
            return static_cast<Package&&>(*this);
        }
    };

private:
    //per-Payload message queue
    //
    template <class Payload>
    class Queue {
        std::map<long, std::queue<Package<Payload>>> map{};
        std::atomic_flag flag = ATOMIC_FLAG_INIT;
    private:
        class Guard {
            std::atomic_flag& flag;
        public:
            Guard(std::atomic_flag& f) noexcept : flag{f} { // acquire access
                do {} while (flag.test_and_set(std::memory_order_acquire));
            }
            ~Guard() noexcept { flag.clear(std::memory_order_release); } // relinquish access
            template <class F, class... Args>
            auto invoke(F&& f, Args&&... args) const // invoke the callable synchronously
            noexcept(std::is_nothrow_invocable_v<F&&, Args&&...>)
            ->               std::invoke_result_t<F&&, Args&&...> {
                static_assert(std::is_invocable_v<F&&, Args&&...>);
                return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            }
        };
    public:
        [[nodiscard]] Ticket operator()(long const key, Payload&& payload) & {
            return map[key].emplace(Package<Payload>{std::move(payload)});
        }
        [[nodiscard]] Ticket enqueue(long const key, Payload payload) & {
            return Guard{flag}.invoke(*this, key, std::move(payload));
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
            do {
                if (auto opt = Guard{flag}.invoke(*this, key)) {
                    return *std::move(opt);
                }
                std::this_thread::yield();
            } while (true);
        }
    };

public:
    // PO box identifier
    //
    union [[nodiscard]] Envelope {
    private:
        std::pair<int, int> int_pair;
        std::pair<unsigned, unsigned> uint_pair;
        long id;
    public:
        constexpr Envelope(int addresser, int addressee) noexcept : int_pair{addresser, addressee} {}
        constexpr Envelope(unsigned addresser, unsigned addressee) noexcept : uint_pair{addresser, addressee} {}
        [[nodiscard]] constexpr operator long() const noexcept { return id; }
    };
    static_assert(sizeof(Envelope) == sizeof(long));

    // send
    //
    template <long I, class Payload> [[nodiscard]]
    auto send(Payload&& payload, Envelope const envelope) {
        return std::get<I>(pool).enqueue(envelope, std::forward<Payload>(payload));
    }
    template <class Payload> [[nodiscard]]
    auto send(Payload&& payload, Envelope const envelope) {
        using T = Queue<std::decay_t<Payload>>;
        return std::get<T>(pool).enqueue(envelope, std::forward<Payload>(payload));
    }

    // receive
    //
    template <long I> [[nodiscard]]
    auto recv(Envelope const envelope) {
        return std::get<I>(pool).dequeue(envelope);
    }
    template <class Payload> [[nodiscard]]
    auto recv(Envelope const envelope) {
        using T = Queue<std::decay_t<Payload>>;
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
/// the MessageDispatch object that created this must outlive
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
    int rank() const noexcept { return static_cast<int>(address); }

    // send
    //
    template <long I, class Payload, class To> [[nodiscard]]
    auto send(Payload&& payload, To const to) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->template send<I>(std::forward<Payload>(payload), {static_cast<To>(address), to});
    }
    template <class Payload, class To> [[nodiscard]]
    auto send(Payload&& payload, To const to) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->send(std::forward<Payload>(payload), {static_cast<To>(address), to});
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
}

// not for public use
//
void test_message_queue();
void test_inter_thread_comm();
PIC1D_END_NAMESPACE

#endif /* MessageDispatch_variant_h */
