//
//  FullDomainDelegate.c
//  pic_1d
//
//  Created by KYUNGGUK MIN on 12/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "FullDomainDelegate.h"
#include "../Module/Domain.h"
#include "../InputWrapper.h"

#include <algorithm>
#include <stdexcept>

P1D::FullDomainDelegate::FullDomainDelegate()
{
    if (Input::number_of_worker_threads != 0) {
        throw std::domain_error{std::string{__FUNCTION__} + " used in multi-domain environment"};
    }
}

// MARK: Interface
//
void P1D::FullDomainDelegate::pass(Domain const &domain, PartBucket &L_bucket, PartBucket &R_bucket)
{
    // note that particle position is already normalized by the grid size

    // simulation domain size
    //
    Real const Lx = domain.params.domain_extent.len;

    for (Particle &ptl : L_bucket) { // crossed left boundary; wrap around to the rightmost cell
        ptl.pos_x += Lx;
    }
    for (Particle &ptl : R_bucket) { // crossed right boundary; wrap around to the leftmost cell
        ptl.pos_x -= Lx;
    }

    using std::swap;
    swap(L_bucket, R_bucket);
}
void P1D::FullDomainDelegate::pass(Domain const&, BField &bfield)
{
    if constexpr (Debug::zero_out_electromagnetic_field) {
        bfield.fill(bfield.geomtr.B0);
    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
        for (Vector &v : bfield) {
            v.y = bfield.geomtr.B0.y;
            v.z = bfield.geomtr.B0.z;
        }
    }
    _pass(bfield);
}
void P1D::FullDomainDelegate::pass(Domain const&, EField &efield)
{
    if constexpr (Debug::zero_out_electromagnetic_field) {
        efield.fill(Vector{});
    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
        for (Vector &v : efield) {
            v.y = v.z = 0;
        }
    }
    _pass(efield);
}
void P1D::FullDomainDelegate::pass(Domain const&, Current &current)
{
    _pass(current);
}
void P1D::FullDomainDelegate::gather(Domain const&, Current &current)
{
    _gather(current);
}
void P1D::FullDomainDelegate::gather(Domain const&, PartSpecies &sp)
{
    _gather(sp.moment<0>());
    _gather(sp.moment<1>());
    _gather(sp.moment<2>());
}

// MARK: Implementation
//
template <class T, long N>
void P1D::FullDomainDelegate::_pass(GridQ<T, N> &A)
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
template <class T, long N>
void P1D::FullDomainDelegate::_gather(GridQ<T, N> &A)
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
