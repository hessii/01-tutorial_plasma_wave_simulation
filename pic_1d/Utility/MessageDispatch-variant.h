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
#include <stdexcept>
#include <algorithm>
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

private:
    class Queue;
    class [[nodiscard]] Tracker;
public:
    using payload_t = std::variant<Payloads...>;

    // PO box identifier
    //
    class [[nodiscard]] Envelope {
        long most, least;
    public:
        template <class T>
        static constexpr bool is_int_v = std::is_integral_v<T> && (sizeof(T) <= sizeof(int));
        //
        template <class T, class U, std::enable_if_t<is_int_v<T> && is_int_v<U>, int> = 0>
        constexpr Envelope(T const most, U const least) noexcept : most{most}, least{least} {
            static_assert(is_int_v<T> && is_int_v<U>, "T or U not an integral type of size 4");
        }
        [[nodiscard]] friend
        constexpr bool operator<(Envelope const &lhs, Envelope const &rhs) noexcept {
            if (lhs.most == rhs.most) return lhs.least < rhs.least;
            else                      return lhs.most  < rhs.most;
        }
    };

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
            // invoke the callable with payload as its first argument
            return Guard{std::move(this->flag)}.invoke(std::forward<F>(f), std::get<Payload>(std::move(this->payload)), std::forward<RestArgs>(rest_args)...);
        }
        //
        [[nodiscard]] operator Payload() && { // std::get may throw exception
            Guard const g = std::move(this->flag); // notify of delivery upon destruction
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
        using queue_t = std::queue<Tracker>;
        using mapped_t = std::pair<std::unique_ptr<std::atomic_flag>, queue_t>;
        using map_t = std::map<Envelope, mapped_t>;
        map_t map{};
    public: // constructor
        Queue() noexcept(std::is_nothrow_default_constructible_v<map_t>) = default;
        Queue(Queue&&) noexcept(std::is_nothrow_move_constructible_v<map_t>) = default;
        Queue &operator=(Queue&&) noexcept(std::is_nothrow_move_assignable_v<map_t>) = default;
        explicit Queue(std::vector<Envelope> const &addresses) {
            for (auto const &address : addresses) {
                mapped_t &v = map.try_emplace(map.end(), address, std::make_unique<std::atomic_flag>(), queue_t{})->second;
                v.first->clear();
            }
        }
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
        [[nodiscard]] static auto push_back(queue_t &q, payload_t&& payload) -> Ticket {
            return q.emplace(std::move(payload));
        }
        [[nodiscard]] static auto pop_front(queue_t &q) -> std::optional<Tracker> {
            if (!q.empty()) {
                auto payload = std::move(q.front()); // must take the ownership of payload
                q.pop();
                return payload; // NRVO
            }
            return std::nullopt;
        }
        [[nodiscard]] decltype(auto) at(Envelope const key) & {
            try {
                return map.at(key);
            } catch (std::out_of_range const&) {
                throw std::out_of_range{__PRETTY_FUNCTION__};
            }
        }
    public:
        [[nodiscard]] auto enqueue(Envelope const key, payload_t payload) & -> Ticket {
            auto &[flag, q] = at(key);
            return Guard{*flag}.invoke(&push_back, q, std::move(payload));
        }
        [[nodiscard]] auto try_dequeue(Envelope const key) & -> std::optional<Tracker> {
            auto &[flag, q] = at(key);
            return Guard{*flag}.invoke(&pop_front, q);
        }
        [[nodiscard]] auto dequeue(Envelope const key) & -> Tracker {
            auto &[flag, q] = at(key);
            do {
                if (auto opt = Guard{*flag}.invoke(&pop_front, q)) {
                    return *std::move(opt);
                }
                std::this_thread::yield();
            } while (true);
        }
    } queue{};

public: // constructor
    MessageDispatch() noexcept(std::is_nothrow_default_constructible_v<Queue>) = default;
    MessageDispatch(MessageDispatch&&) noexcept(std::is_nothrow_move_constructible_v<Queue>) = default;
    MessageDispatch &operator=(MessageDispatch&&) noexcept(std::is_nothrow_move_assignable_v<Queue>) = default;
    explicit MessageDispatch(std::vector<Envelope> const &addresses) : queue{addresses} {
    }
    explicit MessageDispatch(unsigned const world_size) {
        std::vector<Envelope> addresses;
        addresses.reserve(world_size*world_size);
        for (unsigned i = 0; i < world_size; ++i) {
            for (unsigned j = 0; j < world_size; ++j) {
                addresses.emplace_back(i, j);
            }
        }
        *this = MessageDispatch{addresses};
    }

public: // communication methods
    // send
    //
    template <long I> [[nodiscard]]
    auto send(std::variant_alternative_t<I, payload_t> const& payload, Envelope const envelope) -> Ticket {
        return queue.enqueue(envelope, payload);
    }
    template <long I> [[nodiscard]]
    auto send(std::variant_alternative_t<I, payload_t>&& payload, Envelope const envelope) -> Ticket {
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

    // try receive
    //
    template <long I> [[nodiscard]]
    auto try_recv(Envelope const envelope) -> std::optional<Package<std::variant_alternative_t<I, payload_t>>> {
        return queue.try_dequeue(envelope);
    }
    template <class Payload> [[nodiscard]]
    auto try_recv(Envelope const envelope) -> std::optional<Package<Payload>> {
        return queue.try_dequeue(envelope);
    }

    // scatter
    //
    template <long I> [[nodiscard]]
    auto scatter(std::vector<std::variant_alternative_t<I, payload_t>> payloads, std::vector<Envelope> const &dests) {
        if (payloads.size() != dests.size()) {
            throw std::invalid_argument{__PRETTY_FUNCTION__};
        }
        using namespace std::placeholders;
        std::vector<Ticket> tks(payloads.size());
        std::transform(std::make_move_iterator(begin(payloads)), std::make_move_iterator(end(payloads)),
                       begin(dests), begin(tks), std::bind<Ticket>(&MessageDispatch::send<I>, this, _1, _2));
        return tks; // NRVO
    }
    template <class Payload> [[nodiscard]]
    auto scatter(std::vector<Payload> payloads, std::vector<Envelope> const &dests) {
        static_assert(std::is_constructible_v<payload_t, Payload>, "no alternative for the given payload type");
        if (payloads.size() != dests.size()) {
            throw std::invalid_argument{__PRETTY_FUNCTION__};
        }
        using namespace std::placeholders;
        std::vector<Ticket> tks(payloads.size());
        std::transform(std::make_move_iterator(begin(payloads)), std::make_move_iterator(end(payloads)),
                       begin(dests), begin(tks), std::bind<Ticket>(&MessageDispatch::send<Payload>, this, _1, _2));
        return tks; // NRVO
    }

    // broadcast
    //
    template <long I> [[nodiscard]]
    auto bcast(std::variant_alternative_t<I, payload_t> const &payload, std::vector<Envelope> const &dests) {
        std::vector<Ticket> tks(dests.size());
        std::transform(begin(dests), end(dests), begin(tks),
                       [this, &payload](Envelope const &dest) { return send<I>(payload, dest); });
        return tks; // NRVO
    }
    template <class Payload> [[nodiscard]]
    auto bcast(Payload const &payload, std::vector<Envelope> const &dests) {
        static_assert(std::is_constructible_v<payload_t, decltype(payload)>, "no alternative for the given payload type");
        std::vector<Ticket> tks(dests.size());
        std::transform(begin(dests), end(dests), begin(tks),
                       [this, &payload](Envelope const &dest) { return send(payload, dest); });
        return tks; // NRVO
    }

    // gather
    //
    template <long I> [[nodiscard]]
    auto gather(std::vector<Envelope> const &srcs) {
        std::vector<Package<std::variant_alternative_t<I, payload_t>>> pkgs;
        pkgs.reserve(srcs.size());
        for (auto const &src : srcs) {
            pkgs.emplace_back(recv<I>(src));
        }
        return pkgs; // NRVO
    }
    template <class Payload> [[nodiscard]]
    auto gather(std::vector<Envelope> const &srcs) {
        std::vector<Package<Payload>> pkgs;
        pkgs.reserve(srcs.size());
        for (auto const &src : srcs) {
            pkgs.emplace_back(recv<Payload>(src));
        }
        return pkgs; // NRVO
    }

    // reduce
    // reduction operation
    // first argument is the payload, second argument is init
    //
    template <long I, class Payload, class BinaryOp> [[nodiscard]]
    auto reduce(std::vector<Envelope> const &participants, Payload init, BinaryOp&& op) {
        static_assert(std::is_invocable_r_v<Payload, BinaryOp, std::variant_alternative_t<I, payload_t>&&, Payload&&>);
        for (auto const &participant : participants) {
            init = recv<I>(participant).unpack(op, std::move(init));
        }
        return init;
    }
    template <class Payload, class Ret, class BinaryOp> [[nodiscard]]
    auto reduce(std::vector<Envelope> const &participants, Ret init, BinaryOp&& op) {
        static_assert(std::is_invocable_r_v<Ret, BinaryOp, Payload&&, Ret&&>);
        for (auto const &participant : participants) {
            init = recv<Payload>(participant).unpack(op, std::move(init));
        }
        return init;
    }

    // for_each
    // apply sequentially a unary function to the payloads gathered from all participants
    //
    template <long I, class Fn, class... RestArgs>
    void for_each(std::vector<Envelope> const &participants, Fn&& f, RestArgs&&... rest) {
        static_assert(std::is_invocable_v<Fn, std::variant_alternative_t<I, payload_t>&&, RestArgs&...>);
        for (auto const &participant : participants) {
            recv<I>(participant).unpack(f, rest...);
        }
    }
    template <class Payload, class Fn, class... RestArgs>
    void for_each(std::vector<Envelope> const &participants, Fn&& f, RestArgs&&... rest) {
        static_assert(std::is_invocable_v<Fn, Payload&&, RestArgs&...>);
        for (auto const &participant : participants) {
            recv<Payload>(participant).unpack(f, rest...);
        }
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
    int address;
    //
    template <class T>
    static constexpr bool is_int_v = Envelope::template is_int_v<T>;
    template <class Int>
    Communicator(MessageDispatch<Payloads...> *dispatch, Int const rank) noexcept : dispatch{dispatch}, address{} {
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
    template <long I, class To> [[nodiscard]]
    auto send(std::variant_alternative_t<I, payload_t> const& payload, To const to) const {
        static_assert(is_int_v<To>);
        return dispatch->template send<I>(payload, {rank(), to});
    }
    template <long I, class To> [[nodiscard]]
    auto send(std::variant_alternative_t<I, payload_t>&& payload, To const to) const {
        static_assert(is_int_v<To>);
        return dispatch->template send<I>(std::move(payload), {rank(), to});
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

    // try receive
    //
    template <long I, class From> [[nodiscard]]
    auto try_recv(From const from) const {
        static_assert(is_int_v<From>);
        return dispatch->template try_recv<I>({from, rank()});
    }
    template <class Payload, class From> [[nodiscard]]
    auto try_recv(From const from) const {
        static_assert(is_int_v<From>);
        return dispatch->template try_recv<Payload>({from, rank()});
    }

    // scatter
    //
    template <long I, class To> [[nodiscard]]
    auto scatter(std::vector<std::variant_alternative_t<I, payload_t>> payloads, std::vector<To> const &dests) const {
        static_assert(is_int_v<To>);
        if (payloads.size() != dests.size()) {
            throw std::invalid_argument{__PRETTY_FUNCTION__};
        }
        using namespace std::placeholders;
        std::vector<Ticket> tks(payloads.size());
        std::transform(std::make_move_iterator(begin(payloads)), std::make_move_iterator(end(payloads)),
                       begin(dests), begin(tks), std::bind<Ticket>(&Communicator::send<I, To>, this, _1, _2));
        return tks; // NRVO
    }
    template <class Payload, class To> [[nodiscard]]
    auto scatter(std::vector<Payload> payloads, std::vector<To> const &dests) const {
        static_assert(is_int_v<To>);
        if (payloads.size() != dests.size()) {
            throw std::invalid_argument{__PRETTY_FUNCTION__};
        }
        using namespace std::placeholders;
        std::vector<Ticket> tks(payloads.size());
        std::transform(std::make_move_iterator(begin(payloads)), std::make_move_iterator(end(payloads)),
                       begin(dests), begin(tks), std::bind<Ticket>(&Communicator::send<Payload, To>, this, _1, _2));
        return tks; // NRVO
    }

    // broadcast
    //
    template <long I, class To> [[nodiscard]]
    auto bcast(std::variant_alternative_t<I, payload_t> const &payload, std::vector<To> const &dests) const {
        static_assert(is_int_v<To>);
        std::vector<Ticket> tks(dests.size());
        std::transform(begin(dests), end(dests), begin(tks),
                       [this, &payload](To const &dest) { return send<I>(payload, dest); });
        return tks; // NRVO
    }
    template <class Payload, class To> [[nodiscard]]
    auto bcast(Payload const &payload, std::vector<To> const &dests) const {
        static_assert(is_int_v<To>);
        std::vector<Ticket> tks(dests.size());
        std::transform(begin(dests), end(dests), begin(tks),
                       [this, &payload](To const &dest) { return send(payload, dest); });
        return tks; // NRVO
    }

    // gather
    //
    template <long I, class From> [[nodiscard]]
    auto gather(std::vector<From> const &srcs) const {
        static_assert(is_int_v<From>);
        std::vector<Package<std::variant_alternative_t<I, payload_t>>> pkgs;
        pkgs.reserve(srcs.size());
        for (auto const &src : srcs) {
            pkgs.emplace_back(recv<I>(src));
        }
        return pkgs; // NRVO
    }
    template <class Payload, class From> [[nodiscard]]
    auto gather(std::vector<From> const &srcs) const {
        static_assert(is_int_v<From>);
        std::vector<Package<Payload>> pkgs;
        pkgs.reserve(srcs.size());
        for (auto const &src : srcs) {
            pkgs.emplace_back(recv<Payload>(src));
        }
        return pkgs; // NRVO
    }

    // reduce
    // reduction operation
    // first argument is the payload, second argument is init
    //
    template <long I, class Participant, class Payload, class BinaryOp> [[nodiscard]]
    auto reduce(std::vector<Participant> const &participants, Payload init, BinaryOp&& op) const {
        static_assert(is_int_v<Participant>);
        static_assert(std::is_invocable_r_v<Payload, BinaryOp, std::variant_alternative_t<I, payload_t>&&, Payload&&>);
        for (auto const &participant : participants) {
            init = recv<I>(participant).unpack(op, std::move(init));
        }
        return init;
    }
    template <class Payload, class Participant, class Ret, class BinaryOp> [[nodiscard]]
    auto reduce(std::vector<Participant> const &participants, Ret init, BinaryOp&& op) const {
        static_assert(is_int_v<Participant>);
        static_assert(std::is_invocable_r_v<Ret, BinaryOp, Payload&&, Ret&&>);
        for (auto const &participant : participants) {
            init = recv<Payload>(participant).unpack(op, std::move(init));
        }
        return init;
    }

    // for_each
    // apply sequentially a unary function to the payloads gathered from all participants
    //
    template <long I, class Participant, class Fn, class... RestArgs>
    void for_each(std::vector<Participant> const &participants, Fn&& f, RestArgs&&... rest) {
        static_assert(is_int_v<Participant>);
        static_assert(std::is_invocable_v<Fn, std::variant_alternative_t<I, payload_t>&&, RestArgs&...>);
        for (auto const &participant : participants) {
            recv<I>(participant).unpack(f, rest...);
        }
    }
    template <class Payload, class Participant, class Fn, class... RestArgs>
    void for_each(std::vector<Participant> const &participants, Fn&& f, RestArgs&&... rest) const {
        static_assert(is_int_v<Participant>);
        static_assert(std::is_invocable_v<Fn, Payload&&, RestArgs&...>);
        for (auto const &participant : participants) {
            recv<Payload>(participant).unpack(f, rest...);
        }
    }
};
}
PIC1D_END_NAMESPACE

#endif /* MessageDispatch_variant_h */
