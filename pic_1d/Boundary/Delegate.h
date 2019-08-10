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
class PartSpecies;
class BField;
class EField;
class Current;

class Delegate {
    Delegate(Delegate const&) = delete;
    Delegate &operator=(Delegate const&) = delete;

public:
    virtual ~Delegate() = default;
    Delegate() noexcept = default;

    // boundary value communication
    // default implementation is periodic boundary condition
    //
    virtual void partition(PartSpecies &, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket);
    virtual void pass(Domain const&, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket);
    virtual void pass(Domain const&, PartSpecies &);
    virtual void pass(Domain const&, BField &);
    virtual void pass(Domain const&, EField &);
    virtual void pass(Domain const&, Current &);
    virtual void gather(Domain const&, Current &);
    virtual void gather(Domain const&, PartSpecies &);

private: // helpers
    template <class T>
    static void _pass(GridQ<T> &);
    template <class T>
    static void _gather(GridQ<T> &);
};
PIC1D_END_NAMESPACE

#endif /* Delegate_h */
