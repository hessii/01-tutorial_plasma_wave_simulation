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

    // PO box identifier
    //
    class [[nodiscard]] Envelope {
        long key;
    public:
        template <class T>
        static constexpr bool is_int_v = std::is_integral_v<T> && (sizeof(T)*2 <= sizeof(key));
        //
        constexpr explicit Envelope(long const key) noexcept : key{key} {}
        template <class T, class U, std::enable_if_t<is_int_v<T> && is_int_v<U>, int> = 0>
        constexpr Envelope(T const most, U const least) noexcept : key{most} {
            key = key << (sizeof(key)/2);
            key |= least;
        }
        [[nodiscard]] constexpr operator long() const noexcept { return key; }
        [[nodiscard]] friend
        constexpr bool operator<(Envelope const &lhs, Envelope const &rhs) noexcept {
            return lhs.key < rhs.key;
        }
    };

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
            // invoke the callable with payload as its first argument
            return Guard{std::move(flag)}.invoke(std::forward<F>(f), std::move(this->payload), std::forward<RestArgs>(rest_args)...);
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
        using queue_t = std::queue<Package<Payload>>;
        std::map<Envelope, queue_t> map{};
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
        [[nodiscard]] auto push_back(Envelope const key, Payload&& payload) & -> Ticket {
            return map[key].emplace(std::move(payload));
        }
        [[nodiscard]] auto bulk_push(std::map<Envelope, Payload>&& payloads) & {
            std::vector<Ticket> tks;
            tks.reserve(payloads.size());
            for (auto &[key, payload] : payloads) {
                tks.emplace_back(push_back(key, std::move(payload)));
            }
            return tks; // NRVO
        }
        [[nodiscard]] static auto pop_front(queue_t &q) -> std::optional<Package<Payload>> {
            if (!q.empty()) {
                auto payload = std::move(q.front()); // must take the ownership of payload
                q.pop();
                return payload; // NRVO
            }
            return std::nullopt;
        }
        [[nodiscard]] auto bulk_pops(std::set<Envelope> const &keys) & {
            std::map<Envelope, Package<Payload>> pkgs;
            for (auto const &key : keys) {
                if (auto opt = pop_front(map[key])) {
                    pkgs.try_emplace(key, *std::move(opt));
                }
            };
            return pkgs; // NRVO
        }
    public:
        [[nodiscard]] auto enqueue(Envelope const key, Payload payload) & -> Ticket {
            return Guard{flag}.invoke(&Queue::push_back, this, key, std::move(payload));
        }
        [[nodiscard]] auto enqueue(std::map<Envelope, Payload> payloads) & -> std::vector<Ticket> {
            return Guard{flag}.invoke(&Queue::bulk_push, this, std::move(payloads));
        }
        [[nodiscard]] auto dequeue(Envelope const key) & -> Package<Payload> {
            queue_t *q = nullptr;
            if (auto opt = Guard{flag}.invoke([this, key, &q]{ return pop_front(*(q = &map[key])); })) {
                return *std::move(opt);
            }
            while (true) {
                std::this_thread::yield();
                if (auto opt = Guard{flag}.invoke(&pop_front, *q)) {
                    return *std::move(opt);
                }
            }
        }
        [[nodiscard]] auto dequeue(std::set<Envelope> keys) & -> std::vector<Package<Payload>> {
            if (keys.empty()) return {};
            std::map<Envelope, Package<Payload>> pkgs;
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

public: // communication methods
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
        if constexpr (std::is_same_v<Payload, std::tuple_element_t<I, payload_tuple>>) {
            return std::get<I>(pool).enqueue(std::move(payloads));
        } else {
            return std::get<I>(pool).enqueue({
                std::make_move_iterator(payloads.begin()),
                std::make_move_iterator(payloads.end())
            });
        }
    }
    template <class Payload> [[nodiscard]]
    auto scatter(std::map<Envelope, Payload> payloads) {
        using T = Queue<std::decay_t<Payload>>;
        return std::get<T>(pool).enqueue(std::move(payloads));
    }

    // gather
    //
    template <long I> [[nodiscard]]
    auto gather(std::set<Envelope> const &srcs) {
        return std::get<I>(pool).dequeue(srcs);
    }
    template <class Payload> [[nodiscard]]
    auto gather(std::set<Envelope> const &srcs) {
        using T = Queue<std::decay_t<Payload>>;
        return std::get<T>(pool).dequeue(srcs);
    }

    // broadcast
    //
    template <long I, class Payload> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<Envelope> const &dests) {
        std::map<Envelope, std::tuple_element_t<I, payload_tuple>> payloads;
        for (Envelope const &key : dests) {
            payloads.try_emplace(payloads.end(), key, payload);
        }
        return std::get<I>(pool).enqueue(std::move(payloads));
    }
    template <class Payload> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<Envelope> const &dests) {
        std::map<Envelope, Payload> payloads;
        for (Envelope const &key : dests) {
            payloads.try_emplace(payloads.end(), key, payload);
        }
        using T = Queue<std::decay_t<Payload>>;
        return std::get<T>(pool).enqueue(std::move(payloads));
    }

    // communicator
    //
    class Communicator;

    template <class Int> [[nodiscard]]
    Communicator comm(Int const rank) & noexcept { return {this, rank}; }
};

/// MPI-like inter-thread communicator
///
/// the MessageDispatch object that created this must outlive
///
template <class... Payloads>
class MessageDispatch<Payloads...>::Communicator {
    friend MessageDispatch<Payloads...>;
    MessageDispatch<Payloads...> *dispatch;
    int address{};
    //
    template <class T>
    static constexpr bool is_int_v = Envelope::template is_int_v<T>;
    template <class Int>
    Communicator(MessageDispatch<Payloads...> *dispatch, Int const rank) noexcept : dispatch{dispatch} {
        static_assert(is_int_v<Int>, "rank should be an integral type of size 4 or less");
        address |= rank;
    }
public:
    Communicator() noexcept = default;

    // rank
    //
    [[nodiscard]] int rank() const noexcept { return address; }

    // send
    //
    template <long I, class Payload, class To> [[nodiscard]]
    auto send(Payload&& payload, To const to) const {
        static_assert(is_int_v<To>);
        return dispatch->template send<I>(std::forward<Payload>(payload), {rank(), to});
    }
    template <class Payload, class To> [[nodiscard]]
    auto send(Payload&& payload, To const to) const {
        static_assert(is_int_v<To>);
        return dispatch->send(std::forward<Payload>(payload), {rank(), to});
    }

    // receive
    //
    template <long I, class From> [[nodiscard]]
    auto recv(From const from) const {
        static_assert(is_int_v<From>);
        return dispatch->template recv<I>({from, rank()});
    }
    template <class Payload, class From> [[nodiscard]]
    auto recv(From const from) const {
        static_assert(is_int_v<From>);
        return dispatch->template recv<Payload>({from, rank()});
    }

    // scatter
    //
    template <long I, class To, class Payload> [[nodiscard]]
    auto scatter(std::map<To, Payload> payloads) const {
        static_assert(is_int_v<To>);
        return dispatch->template scatter<I>([](auto const rank, decltype(payloads) transient) {
            std::map<Envelope, Payload> payloads;
            for (auto &[to, payload] : transient) {
                payloads.try_emplace(payloads.end(), {rank, to}, std::move(payload));
            }
            return payloads;
        }(rank(), std::move(payloads)));
    }
    template <class To, class Payload> [[nodiscard]]
    auto scatter(std::map<To, Payload> payloads) const {
        static_assert(is_int_v<To>);
        return dispatch->scatter([](auto const rank, decltype(payloads) transient) {
            std::map<Envelope, Payload> payloads;
            for (auto &[to, payload] : transient) {
                payloads.try_emplace(payloads.end(), {rank, to}, std::move(payload));
            }
            return payloads;
        }(rank(), std::move(payloads)));
    }

    // gather
    //
    template <long I, class From> [[nodiscard]]
    auto gather(std::set<From> const &froms) const {
        static_assert(is_int_v<From>);
        std::set<Envelope> srcs;
        for (From const &from : froms) {
            srcs.emplace_hint(srcs.end(), from, rank()); // order is important
        }
        return dispatch->template gather<I>(srcs);
    }
    template <class Payload, class From> [[nodiscard]]
    auto gather(std::set<From> const &froms) const {
        static_assert(is_int_v<From>);
        std::set<Envelope> srcs;
        for (From const &from : froms) {
            srcs.emplace_hint(srcs.end(), from, rank()); // order is important
        }
        return dispatch->template gather<Payload>(srcs);
    }

    // broadcast
    //
    template <long I, class Payload, class To> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<To> const &tos) const {
        static_assert(is_int_v<To>);
        std::set<Envelope> dests;
        for (To const &to : tos) {
            dests.emplace_hint(dests.end(), rank(), to); // order is important
        }
        return dispatch->template bcast<I>(payload, dests);
    }
    template <class Payload, class To> [[nodiscard]]
    auto bcast(Payload const &payload, std::set<To> const &tos) const {
        static_assert(is_int_v<To>);
        std::set<Envelope> dests;
        for (To const &to : tos) {
            dests.emplace_hint(dests.end(), rank(), to); // order is important
        }
        return dispatch->bcast(payload, dests);
    }

    // reduce
    // reduction operation
    // first argument is the payload, second argument is init
    //
    template <long I, class Participant, class Payload, class BinaryOp> [[nodiscard]]
    auto reduce(std::set<Participant> const &participants, Payload init, BinaryOp&& op) const {
        static_assert(is_int_v<Participant>);
        auto pkgs = this->template gather<I>(participants);
        for (auto&& pkg : pkgs) {
            init = std::move(pkg).unpack(op, std::move(init));
        }
        return init;
    }
    template <class Participant, class Payload, class BinaryOp> [[nodiscard]]
    auto reduce(std::set<Participant> const &participants, Payload init, BinaryOp&& op) const {
        static_assert(is_int_v<Participant>);
        auto pkgs = this->template gather<Payload>(participants);
        for (auto&& pkg : pkgs) {
            init = std::move(pkg).unpack(op, std::move(init));
        }
        return init;
    }

    // for_each
    // apply sequentially a unary function to the payloads gathered from all participants
    //
    template <long I, class Participant, class UnaryOp>
    void for_each(std::set<Participant> const &participants, UnaryOp&& op) const {
        static_assert(is_int_v<Participant>);
        auto pkgs = this->template gather<I>(participants);
        for (auto&& pkg : pkgs) {
            std::move(pkg).unpack(op);
        }
    }
    template <class Payload, class Participant, class UnaryOp>
    void for_each(std::set<Participant> const &participants, UnaryOp&& op) const {
        static_assert(is_int_v<Participant>);
        auto pkgs = this->template gather<Payload>(participants);
        for (auto&& pkg : pkgs) {
            std::move(pkg).unpack(op);
        }
    }
};
}
PIC1D_END_NAMESPACE

#endif /* MessageDispatch_tuple_h */
