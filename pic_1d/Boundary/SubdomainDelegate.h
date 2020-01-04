//
//  SubdomainDelegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 12/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef SubdomainDelegate_h
#define SubdomainDelegate_h

#include "Delegate.h"
#include "../Utility/MessageDispatch.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../Utility/GridQ.h"

PIC1D_BEGIN_NAMESPACE
class SubdomainDelegate : public Delegate {
public:
    using message_dispatch_t = MessageDispatch<Scalar, Vector, Tensor, PartBucket>;
    using interthread_comm_t = message_dispatch_t::Communicator;

public:
    static message_dispatch_t dispatch;
    interthread_comm_t const comm;
    unsigned const size;
    unsigned const left_;
    unsigned const right;

public:
    SubdomainDelegate(unsigned const rank, unsigned const size) noexcept;
    bool is_master() const noexcept { return 0 == comm.rank(); }

private:
    // default implementation is periodic boundary condition
    //
    void pass(Domain const&, PartBucket &L_bucket, PartBucket &R_bucket) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Current &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, PartSpecies &) override;

private: // helpers
    template <class T, long N>
    void pass(GridQ<T, N> &) const;
    template <class T, long N>
    void gather(GridQ<T, N> &) const;
};
PIC1D_END_NAMESPACE

#endif /* SubdomainDelegate_h */
