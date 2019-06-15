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
#include <atomic>
#include <array>
#include <any>

HYBRID1D_BEGIN_NAMESPACE
class InterThreadComm {
    InterThreadComm(InterThreadComm const&) = delete;
    InterThreadComm &operator=(InterThreadComm const&) = delete;

public:
    // job tags
    //
    struct     pass_species_tag : public std::integral_constant<long, 0> {};
    struct      pass_bfield_tag : public std::integral_constant<long, 1> {};
    struct      pass_efield_tag : public std::integral_constant<long, 2> {};
    struct      pass_charge_tag : public std::integral_constant<long, 3> {};
    struct     pass_current_tag : public std::integral_constant<long, 4> {};
    struct    gather_charge_tag : public std::integral_constant<long, 5> {};
    struct   gather_current_tag : public std::integral_constant<long, 6> {};
    struct   gather_species_tag : public std::integral_constant<long, 7> {};
    struct _total_job_count_tag : public std::integral_constant<long, 8> {};

    // packet
    //
    struct Packet {
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
        Packet() noexcept {
            flag1.test_and_set();
            flag2.test_and_set();
        }
        //
        void notify_worker() noexcept { _notify(flag1); }
        void notify_master() noexcept { _notify(flag2); }
        //
        void wait_for_worker() noexcept { _wait_for(flag2); }
        void wait_for_master() noexcept { _wait_for(flag1); }
    };
private:
    std::array<Packet, _total_job_count_tag::value> packets{};

public:
    InterThreadComm() noexcept {}

public: // worker thread methods
    template <long i, class Payload>
    void worker_thread_request_to_process([[maybe_unused]] std::integral_constant<long, i> job_tag, Payload *payload)
    {
        // 1. submit job
        //
        std::get<i>(packets).payload = payload;
        std::get<i>(packets).notify_master();

        // 2. wait for result
        //
        std::get<i>(packets).wait_for_master();
        std::get<i>(packets).payload = {};
    }

public: // master thread methods
    class Request { // request by worker
        friend InterThreadComm;
        Packet *pkt;
        Request(Packet *pkt) noexcept : pkt{pkt} {} // should not be nullptr
    public:
        Request(Request &&) noexcept = default;
        Request &operator=(Request &&) noexcept = default;
        //
        ~Request() { pkt->notify_worker(); }
        //
        template <class Payload> [[nodiscard]]
        auto payload() const -> typename std::remove_reference<Payload>::type *{
            return std::any_cast<typename std::remove_reference<Payload>::type *>(pkt->payload);
        }
    };
    //
    template <long i> [[nodiscard]]
    Request master_thread_retrieve_job_request([[maybe_unused]] std::integral_constant<long, i> job_tag) noexcept
    {
        Packet &pkt = std::get<i>(packets);

        // 1. wait for request
        //
        pkt.wait_for_worker();

        // 2. return packet
        //
        return {&pkt};
    }
};
HYBRID1D_END_NAMESPACE

#endif /* InterThreadComm_h */
