//
//  Delegate.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Delegate.h"
#include "../InputWrapper.h"

#include <algorithm>
#include <random>

// MARK: Interface
//
void P1D::Delegate::once(Domain &domain)
{
    std::mt19937 g{123};
    std::uniform_real_distribution<> d{-1, 1};
    for (Vector &v : domain.efield) {
        v.x += d(g) * Debug::initial_efield_noise_amplitude;
        v.y += d(g) * Debug::initial_efield_noise_amplitude;
        v.z += d(g) * Debug::initial_efield_noise_amplitude;
    }
}

void P1D::Delegate::partition(PartSpecies &sp, PartBucket &L_bucket, PartBucket &R_bucket)
{
    // note that particle position is already normalized by the grid size

    // group particles that have crossed left boundaries
    //
    auto L_it = std::partition(sp.bucket.begin(), sp.bucket.end(), [LB = 0.0](Particle const &ptl) noexcept->bool {
        return ptl.pos_x >= LB;
    });
    L_bucket.insert(L_bucket.cend(), L_it, sp.bucket.end());
    sp.bucket.erase(L_it, sp.bucket.end());

    // group particles that have crossed right boundaries
    //
    auto R_it = std::partition(sp.bucket.begin(), sp.bucket.end(), [RB = sp.params.domain_extent.len](Particle const &ptl) noexcept->bool {
        return ptl.pos_x < RB;
    });
    R_bucket.insert(R_bucket.cend(), R_it, sp.bucket.end());
    sp.bucket.erase(R_it, sp.bucket.end());
}
void P1D::Delegate::pass(Domain const &domain, PartBucket &L_bucket, PartBucket &R_bucket)
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
void P1D::Delegate::pass(Domain const& domain, PartSpecies &sp)
{
    PartSpecies::bucket_type L, R;
    partition(sp, L, R);
    pass(domain, L, R);
    sp.bucket.insert(sp.bucket.cend(), L.cbegin(), L.cend());
    sp.bucket.insert(sp.bucket.cend(), R.cbegin(), R.cend());
}
//void P1D::Delegate::pass(Domain const&, BField &bfield)
//{
//    if constexpr (Debug::zero_out_electromagnetic_field) {
//        bfield.fill(bfield.geomtr.B0);
//    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
//        for (Vector &v : bfield) {
//            v.y = bfield.geomtr.B0.y;
//            v.z = bfield.geomtr.B0.z;
//        }
//    }
//    pass(bfield);
//}
//void P1D::Delegate::pass(Domain const&, EField &efield)
//{
//    if constexpr (Debug::zero_out_electromagnetic_field) {
//        efield.fill(Vector{});
//    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
//        for (Vector &v : efield) {
//            v.y = v.z = 0;
//        }
//    }
//    pass(efield);
//}
//void P1D::Delegate::pass(Domain const&, Current &current)
//{
//    pass(current);
//}
//void P1D::Delegate::gather(Domain const&, Current &current)
//{
//    gather(current);
//}
//void P1D::Delegate::gather(Domain const&, PartSpecies &sp)
//{
//    gather(sp.moment<0>());
//    gather(sp.moment<1>());
//    gather(sp.moment<2>());
//}

// MARK: Implementation
//
template <class T, long N>
void P1D::Delegate::pass(GridQ<T, N> &A)
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
void P1D::Delegate::gather(GridQ<T, N> &A)
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
