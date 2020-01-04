//
//  WorkerDelegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef WorkerDelegate_h
#define WorkerDelegate_h

#include "Delegate.h"
#include "MessageDispatch.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../Utility/GridQ.h"
#include "../InputWrapper.h"

#include <utility>

PIC1D_BEGIN_NAMESPACE
class MasterDelegate;

class WorkerDelegate final : public Delegate {
public:
    using message_dispatch_t = MessageDispatch<
        std::pair<PartBucket*, PartBucket*>,
        ScalarGrid      *, VectorGrid      *, TensorGrid      *,
        ScalarGrid const*, VectorGrid const*, TensorGrid const*
    >;
    using interthread_comm_t = message_dispatch_t::Communicator;
    //
    MasterDelegate *master{};
    interthread_comm_t comm{};

private:
    void once(Domain &) override;
#if defined(PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
    void pass(Domain const&, PartSpecies &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Current &) override;
#endif
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, PartSpecies &) override;

private: // helpers
    template <class T, long N>
    void recv_from_master(GridQ<T, N> &buffer);
    template <class T, long N>
    void reduce_to_master(GridQ<T, N> &payload);
    template <class T, long N>
    void reduce_divide_and_conquer(GridQ<T, N> &payload);
    template <class T, long N>
    void accumulate_by_worker(GridQ<T, N> const &payload);
};
PIC1D_END_NAMESPACE

#endif /* WorkerDelegate_h */
