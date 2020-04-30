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
#include <iterator>
#include <optional>
#include <utility>
#include <variant>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <map>
#include <set>

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

private:
    // payload tracker
    //
    class [[nodiscard]] Tracker {
        friend Queue;
        [[nodiscard]] operator Ticket() & { return flag; }
    protected:
        payload_t payload;
        std::weak_ptr<std::atomic_flag> flag;
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
        Package(Tracker&& t) noexcept(std::is_nothrow_move_constructible_v<Tracker>) : Tracker{std::move(t)} {}
        //
        template <class F, class... RestArgs>
        auto unpack(F&& f, RestArgs&&... rest_args) && // std::get may throw exception
        ->               std::invoke_result_t<F&&, Payload&&, RestArgs&&...> {
            static_assert(std::is_invocable_v<F&&, Payload&&, RestArgs&&...>);
            Guard const g = std::move(this->flag); // notify of delivery upon destruction
            // invoke the callable with payload as its first argument
            return g.invoke(std::forward<F>(f), std::get<Payload>(std::move(this->payload)), std::forward<RestArgs>(rest_args)...);
        }
        //
        [[nodiscard]] operator Payload() && {
            Guard const g = std::move(this->flag); // notify of delivery upon destruction
             // std::get may throw exception
            return std::get<Payload>(std::move(this->payload));
        }
        [[nodiscard]] Payload operator*() && {
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
        template <class Payload>
        [[nodiscard]] auto push_back(long const key, Payload&& payload) & -> Ticket {
            return map[key].emplace(std::move(payload));
        }
        template <class Payload>
        [[nodiscard]] auto bulk_push(std::map<long, Payload>&& payloads) & {
            std::vector<Ticket> tks;
            tks.reserve(payloads.size());
            for (auto &[key, payload] : payloads) {
                tks.emplace_back(push_back(key, std::move(payload)));
            }
            return tks; // NRVO
        }
        [[nodiscard]] auto pop_front(long const key) & -> std::optional<Tracker> {
            if (auto &q = map[key]; !q.empty()) {
                auto payload = std::move(q.front()); // must take the ownership of payload
                q.pop();
                return payload; // NRVO
            }
            return std::nullopt;
        }
        [[nodiscard]] auto bulk_pops(std::set<long> const &keys) & {
            std::map<long, Tracker> pkgs;
            for (auto const &key : keys) {
                if (auto opt = pop_front(key)) {
                    pkgs.try_emplace(key, *std::move(opt));
                }
            };
            return pkgs; // NRVO
        }
    public:
        template <class Payload>
        [[nodiscard]] auto enqueue(long const key, Payload payload) & -> Ticket {
            return Guard{flag}.invoke(&Queue::push_back<Payload>, this, key, std::move(payload));
        }
        template <class Payload>
        [[nodiscard]] auto enqueue(std::map<long, Payload> payloads) & -> std::vector<Ticket> {
            return Guard{flag}.invoke(&Queue::bulk_push<Payload>, this, std::move(payloads));
        }
        [[nodiscard]] auto dequeue(long const key) & -> Tracker {
            do {
                if (auto opt = Guard{flag}.invoke(&Queue::pop_front, this, key)) {
                    return *std::move(opt);
                }
                std::this_thread::yield();
            } while (true);
        }
        [[nodiscard]] auto dequeue(std::set<long> keys) & -> std::vector<Tracker> {
            if (keys.empty()) return {};
            std::map<long, Tracker> pkgs;
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
                std::vector<Tracker> pkgs;
                pkgs.reserve(ordered_pkgs.size());
                for (auto &[_, pkg] : ordered_pkgs) {
                    pkgs.emplace_back(std::move(pkg));
                }
                return pkgs; // NRVO
            }(std::move(pkgs));
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
        [[nodiscard]] friend constexpr
        bool operator<(Envelope const &lhs, Envelope const &rhs) noexcept { return lhs.id < rhs.id; }
    };
    static_assert(sizeof(Envelope) == sizeof(long));

    // send
    //
    template <long I> [[nodiscard]]
    auto send(std::variant_alternative_t<I, payload_t> const& payload, Envelope const envelope) -> Ticket {
        static_assert(std::is_constructible_v<payload_t, decltype(payload)>, "no alternative for the given payload index");
        return queue.enqueue(envelope, payload);
    }
    template <long I> [[nodiscard]]
    auto send(std::variant_alternative_t<I, payload_t>&& payload, Envelope const envelope) -> Ticket {
        static_assert(std::is_constructible_v<payload_t, decltype(payload)>, "no alternative for the given payload index");
        return queue.enqueue(envelope, std::move(payload));
    }
    template <class Payload> [[nodiscard]]
    auto send(Payload&& payload, Envelope const envelope) -> Ticket {
        static_assert(std::is_constructible_v<payload_t, decltype(payload)>, "no alternative for the given payload type");
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

    // scatter
    //
    template <long I> [[nodiscard]]
    auto scatter(std::map<Envelope, std::variant_alternative_t<I, payload_t>> payloads) {
        static_assert(std::is_constructible_v<payload_t, std::variant_alternative_t<I, payload_t>>, "no alternative for the given payload index");
        return queue.template enqueue<std::variant_alternative_t<I, payload_t>>({
            std::make_move_iterator(payloads.begin()),
            std::make_move_iterator(payloads.end())
        });
    }
    template <class Payload> [[nodiscard]]
    auto scatter(std::map<Envelope, Payload> payloads) {
        static_assert(std::is_constructible_v<payload_t, Payload>, "no alternative for the given payload type");
        return queue.template enqueue<Payload>({
            std::make_move_iterator(payloads.begin()),
            std::make_move_iterator(payloads.end())
        });
    }

    // gather
    //
    template <long I> [[nodiscard]]
    auto gather(std::set<Envelope> const &dests) -> std::vector<Package<std::variant_alternative_t<I, payload_t>>> {
        auto pkgs = queue.dequeue({begin(dests), end(dests)});
        return {std::make_move_iterator(pkgs.begin()), std::make_move_iterator(pkgs.end())};
    }
    template <class Payload> [[nodiscard]]
    auto gather(std::set<Envelope> const &dests) -> std::vector<Package<Payload>> {
        auto pkgs = queue.dequeue({begin(dests), end(dests)});
        return {std::make_move_iterator(pkgs.begin()), std::make_move_iterator(pkgs.end())};
    }

    // broadcast
    //
    template <long I> [[nodiscard]]
    auto bcast(std::variant_alternative_t<I, payload_t> const &payload, std::set<Envelope> const &dests) {
        static_assert(std::is_constructible_v<payload_t, decltype(payload)>, "no alternative for the given payload index");
        std::map<long, std::variant_alternative_t<I, payload_t>> payloads;
        for (auto const &key : dests) {
            payloads.try_emplace(payloads.end(), key, payload);
        }
        return queue.enqueue(std::move(payloads));
    }
    template <class Payload> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<Envelope> const &dests) {
        static_assert(std::is_constructible_v<payload_t, decltype(payload)>, "no alternative for the given payload type");
        std::map<long, Payload> payloads;
        for (auto const &key : dests) {
            payloads.try_emplace(payloads.end(), key, payload);
        }
        return queue.enqueue(std::move(payloads));
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

#endif /* MessageDispatch_variant_h */
