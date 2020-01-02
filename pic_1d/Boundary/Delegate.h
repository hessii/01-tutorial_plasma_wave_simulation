//
//  Delegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Delegate_h
#define Delegate_h

#include "../Utility/GridQ.h"
#include "../Utility/Particle.h"

#include <deque>

PIC1D_BEGIN_NAMESPACE
class Domain;
class BField;
class EField;
class Current;
class PartSpecies;

// single domain delegate
//
class Delegate {
    Delegate(Delegate const&) = delete;
    Delegate &operator=(Delegate const&) = delete;

public:
    virtual ~Delegate() = default;
    explicit Delegate() noexcept = default;

    // called once after initialization but right before entering loop
    //
    virtual void once(Domain &);

    virtual void prologue(Domain const&, [[maybe_unused]] long const i) {}
    virtual void epilogue(Domain const&, [[maybe_unused]] long const i) {}

    // boundary value communication
    //
    using PartBucket = std::deque<Particle>;
    virtual void partition(PartSpecies &, PartBucket &L_bucket, PartBucket &R_bucket);
    virtual void pass(Domain const&, PartBucket &L_bucket, PartBucket &R_bucket);
    virtual void pass(Domain const&, PartSpecies &);
    virtual void pass(Domain const&, BField &);
    virtual void pass(Domain const&, EField &);
    virtual void pass(Domain const&, Current &);
    virtual void gather(Domain const&, Current &);
    virtual void gather(Domain const&, PartSpecies &);

private: // helpers
    template <class T, long N>
    static void pass(GridQ<T, N> &);
    template <class T, long N>
    static void gather(GridQ<T, N> &);
};
PIC1D_END_NAMESPACE

#endif /* Delegate_h */
