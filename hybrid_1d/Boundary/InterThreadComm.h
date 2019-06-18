//
//  InterThreadComm.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef InterThreadComm_h
#define InterThreadComm_h

#include "../InputWrapper.h"

#include <type_traits>
#include <utility>
#include <atomic>
#include <tuple>

HYBRID1D_BEGIN_NAMESPACE
namespace experimental {
/// one-way inter-thread communicator
///
template <class Tx, class Rx, class... Payloads>
class InterThreadComm {
    static_assert(sizeof...(Payloads) > 0, "invalid number of channels");
    InterThreadComm(InterThreadComm const&) = delete;
    InterThreadComm &operator=(InterThreadComm const&) = delete;

public:
    static constexpr unsigned number_of_channels() noexcept { return sizeof...(Payloads); }

    // one-way communication coordinator
    //
    struct Coordinator {
    private:
        std::atomic_flag flag1{};
        std::atomic_flag flag2{};
        //
        static void _notify(std::atomic_flag &flag) noexcept {
            flag.clear(std::memory_order_release);
        }
        static void _wait_for(std::atomic_flag &flag) noexcept {
            while (flag.test_and_set(std::memory_order_acquire));
        }
    public:
        Coordinator() noexcept {
            flag1.test_and_set();
            flag2.test_and_set();
        }
        //
        void notify_tx() noexcept { _notify(flag1); }
        void notify_rx() noexcept { _notify(flag2); }
        //
        void wait_for_transmit() noexcept { _wait_for(flag2); }
        void wait_for_receipt() noexcept { _wait_for(flag1); }
    };
private:
    std::tuple<std::pair<Coordinator, Payloads>...> channels{};

public:
    InterThreadComm() noexcept {}

public:
    class Ticket {
        friend InterThreadComm;
        Coordinator *coord;
        void (Coordinator::*done)(void) noexcept;
        Ticket(Coordinator *coord, void (Coordinator::*done)(void) noexcept) noexcept : coord{coord}, done{done} {}
    public:
        Ticket() noexcept : Ticket{nullptr, nullptr} {}
        Ticket(Ticket &&o) noexcept : Ticket{} { std::swap(coord, o.coord), std::swap(done, o.done); }
        Ticket &operator=(Ticket &&o) noexcept { std::swap(coord, o.coord), std::swap(done, o.done); return *this; }
        //
        ~Ticket() { this->operator()(); }
        void operator()() noexcept {
            if (coord) {
                (coord->*done)();
                coord = nullptr;
            }
        }
    };

    // tx thread
    //
    template <long i, class Payload> [[nodiscard]]
    Ticket send([[maybe_unused]] Tx const& tx_tag, [[maybe_unused]] std::integral_constant<long, i> ch_tag, Payload&& payload)
    {
        auto &[coord, pkt] = std::get<i>(channels);

        // 1. send data
        //
        pkt = std::forward<Payload>(payload);
        coord.notify_rx();

        // 2. return ticket
        //
        return {&coord, &Coordinator::wait_for_receipt};
    }

    // rx thread
    //
    template <long i> [[nodiscard]]
    auto recv([[maybe_unused]] Rx const& rx_tag, [[maybe_unused]] std::integral_constant<long, i> ch_tag)
    -> std::pair<Ticket, std::tuple_element_t<i, std::tuple<Payloads...>>>
    {
        auto &[coord, pkt] = std::get<i>(channels);

        // 1. wait for data
        //
        coord.wait_for_transmit();

        // 2. return packet
        //
        return std::make_pair(Ticket{&coord, &Coordinator::notify_tx}, std::move(pkt));
    }
};
}
HYBRID1D_END_NAMESPACE

#include <type_traits>
#include <utility>
#include <atomic>
#include <array>
#include <any>

#include <cstdlib> // std::abort

HYBRID1D_BEGIN_NAMESPACE
/// one-way inter-thread communicator
///
template <class Tx, class Rx, long N_Chs>
class InterThreadComm {
    static_assert(N_Chs > 0, "invalid number of channels");
    InterThreadComm(InterThreadComm const&) = delete;
    InterThreadComm &operator=(InterThreadComm const&) = delete;

public:
    static constexpr long number_of_channels() noexcept { return N_Chs; }

    // one-way communication channel
    //
    struct Channel {
        std::any payload{};
    private:
        std::atomic_flag flag1{};
        std::atomic_flag flag2{};
        //
        static void _notify(std::atomic_flag &flag) noexcept {
            flag.clear(std::memory_order_release);
        }
        static void _wait_for(std::atomic_flag &flag) noexcept {
            while (flag.test_and_set(std::memory_order_acquire));
        }
    public:
        Channel() noexcept {
            flag1.test_and_set();
            flag2.test_and_set();
        }
        //
        void notify_tx() noexcept { _notify(flag1); }
        void notify_rx() noexcept { _notify(flag2); }
        //
        void wait_for_transmit() noexcept { _wait_for(flag2); }
        void wait_for_receipt() noexcept { _wait_for(flag1); }
    };
private:
    std::array<Channel, N_Chs> chs{};

public:
    InterThreadComm() noexcept {}

public:
    // tx thread
    //
    class Ticket {
        friend InterThreadComm;
        Channel *ch;
        Ticket(Channel *ch) noexcept : ch{ch} {}
    public:
        Ticket() noexcept : Ticket{nullptr} {}
        Ticket(Ticket &&o) noexcept : ch{o.ch} { o.ch = nullptr; }
        Ticket &operator=(Ticket &&o) noexcept { std::swap(ch, o.ch); return *this; }
        //
        ~Ticket() { wait(); }
        void wait() {
            if (ch) {
                ch->wait_for_receipt();
                ch->payload = {};
                ch = nullptr;
            }
        }
    };

    template <long i, class Payload> [[nodiscard]]
    Ticket send([[maybe_unused]] Tx const& tx_tag, [[maybe_unused]] std::integral_constant<long, i> channel, Payload *payload)
    {
        Channel &ch = std::get<i>(chs);

        // 1. send data
        //
        ch.payload = payload;
        ch.notify_rx();

        // 2. return ticket
        //
        return {&ch};
    }

public:
    // rx thread
    //
    class Packet {
        friend InterThreadComm;
        Channel *ch;
        Packet(Channel *ch) noexcept : ch{ch} {}
    public:
        Packet() noexcept : Packet{nullptr} {}
        Packet(Packet &&o) noexcept : ch{o.ch} { o.ch = nullptr; }
        Packet &operator=(Packet &&o) noexcept { std::swap(ch, o.ch); return *this; }
        //
        ~Packet() { notify(); }
        void notify() {
            if (ch) {
                ch->notify_tx();
                ch = nullptr;
            }
        }
        //
        template <class Payload> [[nodiscard]]
        auto payload() const -> typename std::remove_reference<Payload>::type *{
            if constexpr (true) {
                using RetT = typename std::remove_reference<Payload>::type *;
                RetT *ret = std::any_cast<typename std::remove_reference<Payload>::type *>(&ch->payload);
                if (!ret) {
                    std::abort();
                }
                return *ret;
            } else {
                // this version is not available
                //
                //return std::any_cast<typename std::remove_reference<Payload>::type *>(ch->payload);
            }
        }
    };
    //
    template <long i> [[nodiscard]]
    Packet recv([[maybe_unused]] Rx const& rx_tag, [[maybe_unused]] std::integral_constant<long, i> channel) noexcept
    {
        Channel &ch = std::get<i>(chs);

        // 1. wait for data
        //
        ch.wait_for_transmit();

        // 2. return packet
        //
        return {&ch};
    }
};
HYBRID1D_END_NAMESPACE

#endif /* InterThreadComm_h */
