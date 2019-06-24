//
//  Delegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Delegate_h
#define Delegate_h

#include "../Utility/GridQ.h"
#include "../Utility/Particle.h"

#include <deque>

HYBRID1D_BEGIN_NAMESPACE
class Domain;
class Species;
class BField;
class EField;
class Charge;
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
    virtual void partition(Species &, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket);
    virtual void pass(Domain const&, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket);
    virtual void pass(Domain const&, Species &);
    virtual void pass(Domain const&, BField &);
    virtual void pass(Domain const&, EField &);
    virtual void pass(Domain const&, Charge &);
    virtual void pass(Domain const&, Current &);
    virtual void gather(Domain const&, Charge &);
    virtual void gather(Domain const&, Current &);
    virtual void gather(Domain const&, Species &);

private: // helpers
    template <class T>
    static void _pass(GridQ<T> &);
    template <class T>
    static void _gather(GridQ<T> &);
};
HYBRID1D_END_NAMESPACE

#endif /* Delegate_h */
