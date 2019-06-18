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
/// one-way inter-thread communicator
///
template <class TxTag, class RxTag, class... Payloads>
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
    Ticket send([[maybe_unused]] TxTag const& tx_tag, [[maybe_unused]] std::integral_constant<long, i> ch_tag, Payload&& payload)
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
    auto recv([[maybe_unused]] RxTag const& rx_tag, [[maybe_unused]] std::integral_constant<long, i> ch_tag)
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
HYBRID1D_END_NAMESPACE

#endif /* InterThreadComm_h */
