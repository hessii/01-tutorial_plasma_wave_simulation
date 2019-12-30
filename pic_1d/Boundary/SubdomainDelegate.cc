//
//  SubdomainDelegate.c
//  pic_1d
//
//  Created by KYUNGGUK MIN on 12/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "SubdomainDelegate.h"
#include "../Module/Domain.h"

#include <utility>

// MARK: Interface
//
void P1D::SubdomainDelegate::pass(Domain const &domain, PartBucket &L_bucket, PartBucket &R_bucket)
{
    // note that particle position is already normalized by the grid size

    // pass across boundaries
    //
    {
        auto ticket = L_comm->send(Tx{}, std::move(L_bucket));
        L_bucket = std::move(R_comm->recv<PartBucket>(Rx{}).second);
    }
    {
        auto ticket = R_comm->send(Tx{}, std::move(R_bucket));
        R_bucket = std::move(L_comm->recv<PartBucket>(Rx{}).second);
    }

    // adjust coordinates
    //
    {
        Real const Lx = domain.params.domain_extent.len;
        //
        for (Particle &ptl : L_bucket) { // crossed left boundary; wrap around to the rightmost cell
            ptl.pos_x += Lx;
        }
        for (Particle &ptl : R_bucket) { // crossed right boundary; wrap around to the leftmost cell
            ptl.pos_x -= Lx;
        }
    }

    using std::swap;
    swap(L_bucket, R_bucket);
}
void P1D::SubdomainDelegate::pass(Domain const&, BField &bfield)
{
    if constexpr (Debug::zero_out_electromagnetic_field) {
        bfield.fill(bfield.geomtr.B0);
    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
        for (Vector &v : bfield) {
            v.y = bfield.geomtr.B0.y;
            v.z = bfield.geomtr.B0.z;
        }
    }
    //
    // pass across boundaries
    //
    _pass(bfield);
}
void P1D::SubdomainDelegate::pass(Domain const&, EField &efield)
{
    if constexpr (Debug::zero_out_electromagnetic_field) {
        efield.fill(Vector{});
    } else if constexpr (Input::is_electrostatic) { // zero-out transverse components
        for (Vector &v : efield) {
            v.y = v.z = 0;
        }
    }
    //
    // pass across boundaries
    //
    _pass(efield);
}
void P1D::SubdomainDelegate::pass(Domain const&, Current &current)
{
    _pass(current);
}
void P1D::SubdomainDelegate::gather(Domain const&, Current &current)
{
    _gather(current);
}
void P1D::SubdomainDelegate::gather(Domain const&, PartSpecies &sp)
{
    _gather(sp.moment<0>());
    _gather(sp.moment<1>());
    _gather(sp.moment<2>());
}

template <class T, long N>
void P1D::SubdomainDelegate::_pass(GridQ<T, N> &grid) const
{
    // from inside out
    //
    for (long i = 0; i < Pad; ++i) {
        auto ticket = L_comm->send(Tx{}, grid[i]);
        grid.end()[i] = R_comm->recv<T>(Rx{}).second;
    }
    for (long i = -1; i >= -Pad; --i) {
        auto ticket = R_comm->send(Tx{}, grid.end()[i]);
        grid[i] = L_comm->recv<T>(Rx{}).second;
    }
}
template <class T, long N>
void P1D::SubdomainDelegate::_gather(GridQ<T, N> &grid) const
{
    // from outside in
    //
    for (long i = -Pad; i < 0; ++i) {
        auto ticket = L_comm->send(Tx{}, grid[i]);
        grid.end()[i] += R_comm->recv<T>(Rx{}).second;
    }
    for (long i = Pad - 1; i >= 0; --i) {
        auto ticket = R_comm->send(Tx{}, grid.end()[i]);
        grid[i] += L_comm->recv<T>(Rx{}).second;
    }
}
