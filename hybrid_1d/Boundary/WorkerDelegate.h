//
//  WorkerDelegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef WorkerDelegate_h
#define WorkerDelegate_h

#include "InterThreadComm.h"
#include "Delegate.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../Utility/GridQ.h"
#include "../InputWrapper.h"

#include <vector>

HYBRID1D_BEGIN_NAMESPACE
class MasterDelegate;

class WorkerDelegate : public Delegate {
public:
    // channel tags
    //
    struct scalar_grid_tag : public std::integral_constant<long, 0> {};
    struct vector_grid_tag : public std::integral_constant<long, 1> {};
    struct tensor_grid_tag : public std::integral_constant<long, 2> {};
    struct    particle_tag : public std::integral_constant<long, 3> {};

public:
    experimental::InterThreadComm<      Delegate, WorkerDelegate,
        GridQ<Scalar>*, GridQ<Vector>*, GridQ<Tensor>*, std::vector<Particle>*
    > mutable_comm{}; // payload can be modified
    experimental::InterThreadComm<MasterDelegate, WorkerDelegate,
        GridQ<Scalar> const*, GridQ<Vector> const*, GridQ<Tensor> const*
    > constant_comm{}; // payload is immutable
    MasterDelegate *master{};

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

#endif /* WorkerDelegate_h */
