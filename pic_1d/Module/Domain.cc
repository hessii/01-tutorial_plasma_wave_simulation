//
//  Domain.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Domain.h"
#include "../VDF/MaxwellianVDF.h"
#include "../Boundary/Delegate.h"

#include <cmath>

// helpers
//
namespace {
    template <class T>
    auto &operator+=(P1D::GridQ<T> &lhs, P1D::GridQ<T> const &rhs) noexcept {
        auto rhs_first = rhs.dead_begin(), rhs_last = rhs.dead_end();
        auto lhs_first = lhs.dead_begin();
        while (rhs_first != rhs_last) {
            *lhs_first++ += *rhs_first++;
        }
        return lhs;
    }
    //
    template <class T>
    auto &operator*=(P1D::GridQ<T> &lhs, T const rhs) noexcept {
        auto first = lhs.dead_begin(), last = lhs.dead_end();
        while (first != last) {
            *first++ *= rhs;
        }
        return lhs;
    }
}

// Domain impl
//
P1D::Domain::~Domain()
{
}
P1D::Domain::Domain(Delegate *delegate)
: delegate{delegate} {
    for (unsigned i = 0; i < part_species.size(); ++i) {
        using namespace Input::PartDesc;
        auto vdf = [](unsigned const i){
            Real const vth1 = std::sqrt(betas.at(i))*Input::c * std::abs(Ocs.at(i))/ops.at(i);
            return MaxwellianVDF{vth1, T2OT1s.at(i), vds.at(i)};
        };
        part_species.at(i) = PartSpecies{Ocs.at(i), ops.at(i), Ncs.at(i), vdf(i)};
    }
}

void P1D::Domain::advance_by(unsigned const n_steps)
{
    Domain const &domain = *this;

    // pre-process
    //
    if (!is_recurring_pass) { // execute only once
        is_recurring_pass = true;
    }

    // cycle
    //
    for (long i = 1; i <= n_steps; ++i) {
        cycle(domain);
    }

    // post-process; collect all moments
    //
    for (PartSpecies &sp : part_species) {
        sp.collect_all();
        delegate->gather(domain, sp);
    }
}
void P1D::Domain::cycle(Domain const &domain)
{
    BField &bfield_0 = bfield;
    using Input::dt;
    //
    // 1. update B<0> from n-1/2 to n+1/2 using E at n
    //    B<1> = (B(n-1/2) + B(n+1/2))/2, so B<1> is at a full time step (n)
    //
    bfield_1 = bfield_0;
    bfield_0.update(efield, dt), delegate->pass(domain, bfield_0);
    (bfield_1 += bfield_0) *= Vector{.5};
    //
    // 2 & 3. update velocities and positions by dt and collect current density
    //
    current.reset();
    for (PartSpecies &sp : part_species) {
        sp.update_vel(bfield_1, efield, dt); // v(n-1/2) -> v(n+1/2)
        sp.update_pos(0.5*dt, 0.5), delegate->pass(domain, sp); // x(n) -> x(n+1/2)
        sp.collect_part(), current += sp; // J(n+1/2)
        sp.update_pos(0.5*dt, 0.5), delegate->pass(domain, sp); // x(n+1/2) -> x(n+1)
    }
    //
    // 4. gather, smooth and average current density
    //
    delegate->gather(domain, current), delegate->pass(domain, current);
    for (long i = 0; i < Input::Nsmooths; ++i) {
        current.smooth(), delegate->pass(domain, current);
    }
    //
    // 5. update E from n to n+1 using B and J at n+1/2
    //
    efield.update(bfield_0, current, dt), delegate->pass(domain, efield);
}
