//
//  MessageDispatch-tuple.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/3/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MessageDispatch_tuple_h
#define MessageDispatch_tuple_h

#include "../Macros.h"

#include <type_traits>
#include <functional>
#include <iterator>
#include <optional>
#include <utility>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <tuple>
#include <queue>
#include <map>
#include <set>

PIC1D_BEGIN_NAMESPACE
inline namespace _ver_tuple {
/// MPI-like message (payload) passing machinary
///
/// @discussion The payload type is part of the message signature.
/// Hence, the ordering of send-recv pairs for different payload types can be mixed.
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
        std::shared_ptr<std::atomic_flag> flag;
        Ticket(std::weak_ptr<std::atomic_flag> &weak) : flag{std::make_shared<std::atomic_flag>()} {
            flag->test_and_set();
            weak = flag;
        }
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
        std::weak_ptr<std::atomic_flag> flag;
    public:
        Package &operator=(Package&&) = delete;
        Package(Package&&) noexcept(std::is_nothrow_move_constructible_v<Payload>) = default;
        Package(Payload&& payload) noexcept(std::is_nothrow_move_constructible_v<Payload>) : payload{std::move(payload)} {}
    private:
        [[nodiscard]] operator Ticket() & { return flag; }
        class [[nodiscard]] Guard {
            std::shared_ptr<std::atomic_flag> flag;
        public:
            Guard(std::weak_ptr<std::atomic_flag> &&weak) noexcept(noexcept(weak.lock())) : flag{weak.lock()} {}
            ~Guard() noexcept { if (flag) flag->clear(std::memory_order_release); } // notify of delivery
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
            Guard const g = std::move(flag); // notify of delivery upon destruction
            // invoke the callable with payload as its first argument
            return g.invoke(std::forward<F>(f), std::move(this->payload), std::forward<RestArgs>(rest_args)...);
        }
        //
        [[nodiscard]] operator Payload() && noexcept(std::is_nothrow_move_constructible_v<Payload>) {
            Guard const g = std::move(flag); // notify of delivery upon destruction
            return std::move(payload);
        }
        [[nodiscard]] Payload operator*() && noexcept(std::is_nothrow_move_constructible_v<Payload>) {
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
        class [[nodiscard]] Guard { // guarded invocation
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
        //
        [[nodiscard]] auto push_back(long const key, Payload&& payload) & -> Ticket {
            return map[key].emplace(std::move(payload));
        }
        [[nodiscard]] auto bulk_push(std::map<long, Payload>&& payloads) & {
            std::vector<Ticket> tks;
            tks.reserve(payloads.size());
            for (auto &[key, payload] : payloads) {
                tks.emplace_back(push_back(key, std::move(payload)));
            }
            return tks; // NRVO
        }
        [[nodiscard]] auto pop_front(long const key) & -> std::optional<Package<Payload>> {
            if (auto &q = map[key]; !q.empty()) {
                auto payload = std::move(q.front()); // must take the ownership of payload
                q.pop();
                return payload; // NRVO
            }
            return std::nullopt;
        }
        [[nodiscard]] auto bulk_pops(std::set<long> const &keys) & {
            std::map<long, Package<Payload>> pkgs;
            for (auto const &key : keys) {
                if (auto opt = pop_front(key)) {
                    pkgs.try_emplace(key, *std::move(opt));
                }
            };
            return pkgs; // NRVO
        }
    public:
        [[nodiscard]] auto enqueue(long const key, Payload payload) & -> Ticket {
            return Guard{flag}.invoke(&Queue::push_back, this, key, std::move(payload));
        }
        [[nodiscard]] auto enqueue(std::map<long, Payload> payloads) & -> std::vector<Ticket> {
            return Guard{flag}.invoke(&Queue::bulk_push, this, std::move(payloads));
        }
        [[nodiscard]] auto dequeue(long const key) & -> Package<Payload> {
            do {
                if (auto opt = Guard{flag}.invoke(&Queue::pop_front, this, key)) {
                    return *std::move(opt);
                }
                std::this_thread::yield();
            } while (true);
        }
        [[nodiscard]] auto dequeue(std::set<long> keys) & -> std::vector<Package<Payload>> {
            if (keys.empty()) return {};
            std::map<long, Package<Payload>> pkgs;
            do {
                pkgs.merge(Guard{flag}.invoke(&Queue::bulk_pops, this, keys));
                for (auto const &[key, _] : pkgs) { // remove used keys
                    keys.erase(key);
                }
                if (keys.empty()) {
                    break;
                }
                std::this_thread::yield();
            } while (true);
            //
            return [](auto ordered_pkgs) {
                std::vector<Package<Payload>> pkgs;
                pkgs.reserve(ordered_pkgs.size());
                for (auto &[_, pkg] : ordered_pkgs) {
                    pkgs.emplace_back(std::move(pkg));
                }
                return pkgs; // NRVO
            }(std::move(pkgs));
        }
    };
    std::tuple<Queue<Payloads>...> pool{};

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
        [[nodiscard]] friend constexpr
        bool operator<(Envelope const &lhs, Envelope const &rhs) noexcept { return lhs.id < rhs.id; }
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

    // scatter
    //
    template <long I, class Payload> [[nodiscard]]
    auto scatter(std::map<Envelope, Payload> payloads) {
        return std::get<I>(pool).enqueue({
            std::make_move_iterator(payloads.begin()),
            std::make_move_iterator(payloads.end())
        });
    }
    template <class Payload> [[nodiscard]]
    auto scatter(std::map<Envelope, Payload> payloads) {
        using T = Queue<std::decay_t<Payload>>;
        return std::get<T>(pool).enqueue({
            std::make_move_iterator(payloads.begin()),
            std::make_move_iterator(payloads.end())
        });
    }

    // gather
    //
    template <long I> [[nodiscard]]
    auto gather(std::set<Envelope> const &dests) {
        return std::get<I>(pool).dequeue({begin(dests), end(dests)});
    }
    template <class Payload> [[nodiscard]]
    auto gather(std::set<Envelope> const &dests) {
        using T = Queue<std::decay_t<Payload>>;
        return std::get<T>(pool).dequeue({begin(dests), end(dests)});
    }

    // broadcast
    //
    template <long I, class Payload> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<Envelope> const &dests) {
        std::map<long, Payload> payloads;
        for (auto const &key : dests) {
            payloads.try_emplace(payloads.end(), key, payload);
        }
        return std::get<I>(pool).enqueue(std::move(payloads));
    }
    template <class Payload> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<Envelope> const &dests) {
        std::map<long, Payload> payloads;
        for (auto const &key : dests) {
            payloads.try_emplace(payloads.end(), key, payload);
        }
        using T = Queue<std::decay_t<Payload>>;
        return std::get<T>(pool).enqueue(std::move(payloads));
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

    // scatter
    //
    template <long I, class To, class Payload> [[nodiscard]]
    auto scatter(std::map<To, Payload> payloads) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->template scatter<I>([](auto const address, auto transient) {
            std::map<Envelope, Payload> payloads;
            for (auto &[to, payload] : transient) {
                payloads.try_emplace(payloads.end(), {address, to}, std::move(payload));
            }
            return payloads;
        }(static_cast<To>(address), std::move(payloads)));
    }
    template <class To, class Payload> [[nodiscard]]
    auto scatter(std::map<To, Payload> payloads) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->scatter([](auto const address, auto transient) {
            std::map<Envelope, Payload> payloads;
            for (auto &[to, payload] : transient) {
                payloads.try_emplace(payloads.end(), {address, to}, std::move(payload));
            }
            return payloads;
        }(static_cast<To>(address), std::move(payloads)));
    }

    // gather
    //
    template <long I, class From> [[nodiscard]]
    auto gather(std::set<From> const &froms) const {
        static_assert(std::is_same_v<From, int> || std::is_same_v<From, unsigned>);
        return dispatch->template gather<I>([](auto const address, auto const &froms) {
            std::set<Envelope> dests;
            for (From const &from : froms) {
                dests.emplace_hint(dests.end(), from, address); // order is important
            }
            return dests;
        }(static_cast<From>(address), froms));
    }
    template <class Payload, class From> [[nodiscard]]
    auto gather(std::set<From> const &froms) const {
        static_assert(std::is_same_v<From, int> || std::is_same_v<From, unsigned>);
        return dispatch->template gather<Payload>([](auto const address, auto const &froms) {
            std::set<Envelope> dests;
            for (From const &from : froms) {
                dests.emplace_hint(dests.end(), from, address); // order is important
            }
            return dests;
        }(static_cast<From>(address), froms));
    }

    // broadcast
    //
    template <long I, class Payload, class To> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<To> const tos) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->template bcast<I>(payload, [](auto const address, auto const &tos) {
            std::set<Envelope> dests;
            for (To const &to : tos) {
                dests.emplace_hint(dests.end(), address, to); // order is important
            }
            return dests;
        }(static_cast<To>(address), tos));
    }
    template <class Payload, class To> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<To> const tos) const {
        static_assert(std::is_same_v<To, int> || std::is_same_v<To, unsigned>);
        return dispatch->bcast(payload, [](auto const address, auto const &tos) {
            std::set<Envelope> dests;
            for (To const &to : tos) {
                dests.emplace_hint(dests.end(), address, to); // order is important
            }
            return dests;
        }(static_cast<To>(address), tos));
    }
};
}
PIC1D_END_NAMESPACE

#endif /* MessageDispatch_tuple_h */
