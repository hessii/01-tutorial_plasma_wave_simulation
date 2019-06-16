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

    MasterWrapper *master{};
    InterThreadComm<MasterWrapper, WorkerWrapper, NChs::value> master_to_worker{};

private:
#if defined(HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS) && HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS
    void pass(Domain const&, Species &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Charge &) override;
    void pass(Domain const&, Current &) override;
#endif
    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;

    template <long i, class Payload>
    void recv_from_master(std::integral_constant<long, i> tag, Payload &buffer, bool const is_boundary_only = false);
    template <long i, class Payload>
    void reduce_to_master(std::integral_constant<long, i> tag, Payload const &payload);
};
HYBRID1D_END_NAMESPACE

#endif /* WorkerWrapper_h */
