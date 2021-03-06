//
//  SubdomainDelegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 12/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef SubdomainDelegate_h
#define SubdomainDelegate_h

#include "./Delegate.h"
#include "../Utility/MessageDispatch.h"

PIC1D_BEGIN_NAMESPACE
class SubdomainDelegate : public Delegate {
public:
    using message_dispatch_t = MessageDispatch<Scalar const*, Vector const*, Tensor const*, PartBucket>;
    using interthread_comm_t = message_dispatch_t::Communicator;

    static message_dispatch_t dispatch;
    interthread_comm_t const comm;
    unsigned const size;
    unsigned const left_;
    unsigned const right;
    static constexpr unsigned master = 0;
    [[nodiscard]] bool is_master() const noexcept { return master == comm.rank(); }

public:
    SubdomainDelegate(unsigned const rank, unsigned const size);

private:
    void once(Domain &) const override;
    void prologue(Domain const&, long const) const override {}
    void epilogue(Domain const&, long const) const override {}

    // default implementation is periodic boundary condition
    //
    void pass(Domain const&, PartBucket &L_bucket, PartBucket &R_bucket) const override;
    void pass(Domain const&, ColdSpecies &) const override;
    void pass(Domain const&, BField &) const override;
    void pass(Domain const&, EField &) const override;
    void pass(Domain const&, Current &) const override;
    void gather(Domain const&, Current &) const override;
    void gather(Domain const&, PartSpecies &) const override;

private: // helpers
    template <class T, long N>
    void pass(GridQ<T, N> &) const;
    template <class T, long N>
    void gather(GridQ<T, N> &) const;
};
PIC1D_END_NAMESPACE

#endif /* SubdomainDelegate_h */
