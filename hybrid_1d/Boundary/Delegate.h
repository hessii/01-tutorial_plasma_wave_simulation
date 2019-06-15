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

HYBRID1D_BEGIN_NAMESPACE
class Domain;
class Species;
class BField;
class EField;
class Charge;
class Current;

class Delegate {
public:
    virtual ~Delegate() = default;

    // boundary value communication
    // default implementation is periodic boundary condition
    //
    virtual void pass(Domain const&, Species &);
    virtual void pass(Domain const&, BField &);
    virtual void pass(Domain const&, EField &);
    virtual void pass(Domain const&, Charge &);
    virtual void pass(Domain const&, Current &);
    virtual void gather(Domain const&, Charge &);
    virtual void gather(Domain const&, Current &);
    virtual void gather(Domain const&, Species &);

private: // helpers
    inline static void _pass(Species &);
    template <class T>
    inline static void _pass(GridQ<T> &);
    template <class T>
    inline static void _gather(GridQ<T> &);
};
HYBRID1D_END_NAMESPACE

#endif /* Delegate_h */
