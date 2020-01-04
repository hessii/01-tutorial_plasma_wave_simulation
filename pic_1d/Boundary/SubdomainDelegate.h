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
#include "../Utility/InterThreadComm.h"
#include "../Utility/Particle.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"
#include "../Utility/GridQ.h"

PIC1D_BEGIN_NAMESPACE
class SubdomainDelegate : public Delegate {
    struct Rx { constexpr explicit Rx() noexcept = default; };
    struct Tx { constexpr explicit Tx() noexcept = default; };
public:
    using ITComm = InterThreadComm<Tx, Rx, Scalar, Vector, Tensor, PartBucket>;

private:
    ITComm *const L_comm;
    ITComm *const R_comm;
public:
    explicit SubdomainDelegate(ITComm *left, ITComm *right) noexcept
    : L_comm{left}, R_comm{right} {}

public:
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
