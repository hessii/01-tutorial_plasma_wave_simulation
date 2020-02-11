//
//  MessageDispatch-variant.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/10/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MessageDispatch_variant_h
#define MessageDispatch_variant_h

#include "../Macros.h"

#include <type_traits>
#include <functional>
#include <optional>
#include <utility>
#include <variant>
#include <memory>
#include <thread>
#include <atomic>
#include <queue>
#include <map>

PIC1D_BEGIN_NAMESPACE
namespace _ver_variant {
/// MPI-like message (payload) passing machinary
///
/// @discussion The payload type is NOT part of the message signature.
/// Mixed ordering of send-recv pairs for different payload types will throw an exception.
///
template <class... Payloads>
class MessageDispatch {
    static_assert((... && std::is_move_constructible_v<Payloads>), "Payloads should be move-constructible");
public:
    MessageDispatch() = default;
    MessageDispatch(MessageDispatch const&) = delete;
    MessageDispatch &operator=(MessageDispatch const&) = delete;

private:
    class Queue;
    class [[nodiscard]] Tracker;
public:
    class Communicator;
    using payload_t = std::variant<Payloads...>;

    // payload tracker
    //
    class [[nodiscard]] Ticket {
        friend Tracker;
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

private:
    // payload tracker
    //
    class [[nodiscard]] Tracker {
        friend Queue;
        operator Ticket() & { return std::unique_ptr<std::atomic_flag>{flag = new std::atomic_flag}; }
    protected:
        payload_t payload;
        std::atomic_flag* flag;
    public:
        Tracker &operator=(Tracker&&) = delete;
        Tracker(Tracker&&) noexcept(std::is_nothrow_move_constructible_v<payload_t>) = default;
        Tracker(payload_t&& payload) noexcept(std::is_nothrow_move_constructible_v<payload_t>) : payload{std::move(payload)} {}
    };
public:
    // payload wrapper
    //
    template <class Payload>
    class [[nodiscard]] Package : private Tracker {
        static_assert(std::is_constructible_v<payload_t, Payload&&>, "no alternative for the given payload type");
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
        Package(Tracker&& t) noexcept(std::is_nothrow_move_constructible_v<Tracker>) : Tracker{std::move(t)} {}
        //
        template <class F, class... RestArgs>
        auto unpack(F&& f, RestArgs&&... rest_args) && // std::get may throw exception
        ->               std::invoke_result_t<F&&, Payload&&, RestArgs&&...> {
            static_assert(std::is_invocable_v<F&&, Payload&&, RestArgs&&...>);
            // invoke the callable with payload as its first argument
            return Guard{*this->flag}.invoke(std::forward<F>(f), std::get<Payload>(std::move(this->payload)), std::forward<RestArgs>(rest_args)...);
        }
        //
        [[nodiscard]] operator Payload() && { // std::get may throw exception
            static_assert(std::is_move_constructible_v<Payload>);
            return Guard{*this->flag}.invoke([&payload = this->payload]() noexcept(std::is_nothrow_move_constructible_v<Payload>) { return std::get<Payload>(std::move(payload)); });
        }
        [[nodiscard]] Payload operator*() && {
            static_assert(std::is_move_constructible_v<Payload>);
            return static_cast<Package&&>(*this);
        }
    };

private:
    //per-Payload message queue
    //
    class Queue {
        std::map<long, std::queue<Tracker>> map{};
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
        template <class Payload>
        [[nodiscard]] Ticket operator()(long const key, Payload&& payload) & {
            return map[key].emplace(std::move(payload));
        }
        template <class Payload>
        [[nodiscard]] Ticket enqueue(long const key, Payload payload) & {
            return Guard{flag}.invoke(*this, key, std::move(payload));
        }
        //
        [[nodiscard]] std::optional<Tracker> operator()(long const key) & {
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
        [[nodiscard]] Tracker dequeue(long const key) & {
            do {
                if (auto opt = Guard{flag}.invoke(*this, key)) {
                    return *std::move(opt);
                }
                std::this_thread::yield();
            } while (true);
        }
    } queue;

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
    auto send(Payload&& payload, Envelope const envelope) -> Ticket {
        static_assert(std::is_constructible_v<payload_t, std::variant_alternative_t<I, payload_t>>, "no alternative for the given payload index");
        return queue.enqueue(envelope, std::forward<Payload>(payload));
    }
    template <class Payload> [[nodiscard]]
    auto send(Payload&& payload, Envelope const envelope) -> Ticket {
        static_assert(std::is_constructible_v<payload_t, Payload&&>, "no alternative for the given payload type");
        return queue.enqueue(envelope, std::forward<Payload>(payload));
    }

    // receive
    //
    template <long I> [[nodiscard]]
    auto recv(Envelope const envelope) -> Package<std::variant_alternative_t<I, payload_t>> {
        return queue.dequeue(envelope);
    }
    template <class Payload> [[nodiscard]]
    auto recv(Envelope const envelope) -> Package<Payload> {
        return queue.dequeue(envelope);
    }

    // communicator
    //
    template <class Int>
    [[nodiscard]] Communicator comm(Int const address) & noexcept {
        static_assert(std::is_same_v<Int, int> || std::is_same_v<Int, unsigned>);
        return {this, address};
    }
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
PIC1D_END_NAMESPACE

#endif /* MessageDispatch_variant_h */
