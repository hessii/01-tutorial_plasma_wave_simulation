//
//  Delegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Delegate_h
#define Delegate_h

#include "../Utility/Particle.h"

#include <deque>

PIC1D_BEGIN_NAMESPACE
class Domain;
class BField;
class EField;
class Current;
class PartSpecies;

class Delegate {
    Delegate(Delegate const&) = delete;
    Delegate &operator=(Delegate const&) = delete;

protected:
    explicit Delegate() noexcept = default;
public:
    virtual ~Delegate() = default;

    // called once after initialization but right before entering loop
    //
    virtual void once(Domain &);

    virtual void prologue(Domain const&, [[maybe_unused]] long const i) {}
    virtual void epilogue(Domain const&, [[maybe_unused]] long const i) {}

    // boundary value communication
    //
    virtual void partition(PartSpecies &, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket);
    virtual void pass(Domain const&, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket) = 0;
    virtual void pass(Domain const&, PartSpecies &);
    virtual void pass(Domain const&, BField &) = 0;
    virtual void pass(Domain const&, EField &) = 0;
    virtual void pass(Domain const&, Current &) = 0;
    virtual void gather(Domain const&, Current &) = 0;
    virtual void gather(Domain const&, PartSpecies &) = 0;
};
PIC1D_END_NAMESPACE

#endif /* Delegate_h */
