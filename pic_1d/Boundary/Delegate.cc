//
//  Delegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Delegate.h"
#include "../Module/BField.h"
#include "../Module/EField.h"
#include "../Module/Charge.h"
#include "../Module/Current.h"
#include "../Module/Species.h"
#include "../InputWrapper.h"

#include <algorithm>

// MARK: Interface
//
void P1D::Delegate::partition(Species &sp, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket)
{
    constexpr Real Lx = Input::Nx; // simulation size; note that particle position is already normalized by the grid size

    // group particles that have crossed left boundaries
    //
    auto L_it = std::partition(sp.bucket.begin(), sp.bucket.end(), [](Particle const &ptl) noexcept->bool {
        return ptl.pos_x >= 0.;
    });
    L_bucket.insert(L_bucket.cend(), L_it, sp.bucket.end());
    sp.bucket.erase(L_it, sp.bucket.end());

    // group particles that have crossed right boundaries
    //
    auto R_it = std::partition(sp.bucket.begin(), sp.bucket.end(), [](Particle const &ptl) noexcept->bool {
        return ptl.pos_x < Lx;
    });
    R_bucket.insert(R_bucket.cend(), R_it, sp.bucket.end());
    sp.bucket.erase(R_it, sp.bucket.end());
}
void P1D::Delegate::pass(Domain const&, std::deque<Particle> &L_bucket, std::deque<Particle> &R_bucket)
{
    constexpr Real Lx = Input::Nx; // simulation size; note that particle position is already normalized by the grid size

    for (Particle &ptl : L_bucket) { // crossed left boundary
        ptl.pos_x += Lx;
    }
    for (Particle &ptl : R_bucket) { // crossed right boundary
        ptl.pos_x -= Lx;
    }

    std::swap(L_bucket, R_bucket);
}
void P1D::Delegate::pass(Domain const& domain, Species &sp)
{
    std::deque<Particle> L, R;
    partition(sp, L, R);
    pass(domain, L, R);
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}
void P1D::Delegate::pass(Domain const&, BField &bfield)
{
    if (Debug::zero_out_electromagnetic_field) {
        bfield.fill(bfield.B0);
    }
    _pass(bfield);
}
void P1D::Delegate::pass(Domain const&, EField &efield)
{
    if (Debug::zero_out_electromagnetic_field) {
        efield.fill(Vector{});
    }
    _pass(efield);
}
void P1D::Delegate::pass(Domain const&, Charge &charge)
{
    _pass(charge);
}
void P1D::Delegate::pass(Domain const&, Current &current)
{
    _pass(current);
}
void P1D::Delegate::gather(Domain const&, Charge &charge)
{
    _gather(charge);
}
void P1D::Delegate::gather(Domain const&, Current &current)
{
    _gather(current);
}
void P1D::Delegate::gather(Domain const&, Species &sp)
{
    _gather(sp.moment<0>());
    _gather(sp.moment<1>());
    _gather(sp.moment<2>());
}

// MARK: Implementation
//
template <class T>
void P1D::Delegate::_pass(GridQ<T> &A)
{
    // fill ghost cells
    //
    for (long p = 0, m = -1; p < Pad; ++p, --m) {
        // put left boundary value to right ghost cell
        A.end()[p] = A[p];
        // put right boundary value to left ghost cell
        A[m] = A.end()[m];
    }
}
template <class T>
void P1D::Delegate::_gather(GridQ<T> &A)
{
    // gather moments at ghost cells
    //
    for (long p = Pad - 1, m = -Pad; m < 0; --p, ++m) {
        // add right ghost cell value to left boundary
        A[p] += A.end()[p];
        // add left ghost cell value to right boundary
        A.end()[m] += A[m];
    }
}
