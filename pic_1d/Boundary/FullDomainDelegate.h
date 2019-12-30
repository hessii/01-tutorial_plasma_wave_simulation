//
//  FullDomainDelegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 12/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef FullDomainDelegate_h
#define FullDomainDelegate_h

#include "Delegate.h"
#include "../Utility/GridQ.h"
#include "../Utility/Particle.h"

#include <deque>

PIC1D_BEGIN_NAMESPACE
class FullDomainDelegate : public Delegate {
public:
    using Delegate::Delegate;

public:
    // default implementation is periodic boundary condition
    //
    void pass(Domain const&, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Current &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, PartSpecies &) override;

private: // helpers
    template <class T, long N>
    static void _pass(GridQ<T, N> &);
    template <class T, long N>
    static void _gather(GridQ<T, N> &);
};
PIC1D_END_NAMESPACE

#endif /* FullDomainDelegate_h */
