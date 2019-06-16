//
//  WorkerWrapper.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef WorkerWrapper_h
#define WorkerWrapper_h

#include "InterThreadComm.h"
#include "Delegate.h"

HYBRID1D_BEGIN_NAMESPACE
class MasterWrapper;

class WorkerWrapper : public Delegate {
public:
    // channel tags
    //
    struct    pass_species_tag : public std::integral_constant<long, 0> {};
    struct     pass_bfield_tag : public std::integral_constant<long, 1> {};
    struct     pass_efield_tag : public std::integral_constant<long, 2> {};
    struct     pass_charge_tag : public std::integral_constant<long, 3> {};
    struct    pass_current_tag : public std::integral_constant<long, 4> {};
    struct   gather_charge_tag : public std::integral_constant<long, 5> {};
    struct  gather_current_tag : public std::integral_constant<long, 6> {};
    struct  gather_species_tag : public std::integral_constant<long, 7> {};
    struct                NChs : public std::integral_constant<long, 8> {};

private:
    InterThreadComm<WorkerWrapper, WorkerWrapper, NChs::value> worker_to_worker{};
public:
    InterThreadComm<MasterWrapper, WorkerWrapper, NChs::value> master_to_worker{};
    MasterWrapper *master{};

private:
#if defined(HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
    void pass(Domain const&, Species &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Charge &) override;
    void pass(Domain const&, Current &) override;
#endif
    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;

    template <long i, class T>
    void recv_from_master(std::integral_constant<long, i> tag, GridQ<T> &buffer);
    template <long i, class T>
    void reduce_to_master(std::integral_constant<long, i> tag, GridQ<T> &payload);
    template <long i, class T>
    void reduce_divide_and_conquer(std::integral_constant<long, i> tag, GridQ<T> &payload);
    template <long i, class T>
    void accumulate_by_worker(std::integral_constant<long, i> tag, GridQ<T> const &payload);
};
HYBRID1D_END_NAMESPACE

#endif /* WorkerWrapper_h */
