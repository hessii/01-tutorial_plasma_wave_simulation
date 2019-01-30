//
//  Domain_CAMCL.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "Domain_CAMCL.h"
#include "Delegate.h"

#include <stdexcept>

namespace {
    template <class T>
    auto &operator+=(H1D::GridQ<T> &lhs, H1D::GridQ<T> const &rhs) noexcept {
        auto rhs_first = rhs.dead_begin(), rhs_last = rhs.dead_end();
        auto lhs_first = lhs.dead_begin();
        while (rhs_first != rhs_last) {
            *lhs_first++ += *rhs_first++;
        }
        return lhs;
    }
    //
    template <class T>
    auto &operator*=(H1D::GridQ<T> &lhs, T const rhs) noexcept {
        auto first = lhs.dead_begin(), last = lhs.dead_end();
        while (first != last) {
            *first++ *= rhs;
        }
        return lhs;
    }
}

void H1D::Domain_CAMCL::advance_by(unsigned const n_steps)
{
    Domain const &domain = *this;

    // pre-process
    //
    if (!is_recurring_pass) { // execute only once
        is_recurring_pass = true;
        //
        if (nullptr == delegate) {
            throw std::runtime_error(std::string(__FUNCTION__) + " - delegate is not set");
        }
    }

    // cycle
    //
    for (long i = 1; i <= n_steps; ++i) {
        cycle(domain);
    }

    // post-process; collect all moments
    //
    for (Species &sp : species) {
        sp.collect_all();
        delegate->gather(domain, sp);
    }
}
void H1D::Domain_CAMCL::cycle(Domain const &domain)
{
    Current &current_0 = this->current;
    Charge &charge_0 = this->charge;
    using Input::dt;
    //
    // 1 & 2. update velocities and positions by full step and collect charge and current densities
    //
    current_0.reset();
    current_1.reset();
    charge_0.reset();
    charge_1.reset();
    lambda.reset();
    gamma.reset();
    for (Species &sp : species) {
        sp.update_vel(bfield, efield, dt); // v^N-1/2 -> v^N+1/2
        sp.collect_part();
        current_0 += sp; // J^-
        charge_0 += sp; // rho^N
        sp.update_pos(dt, 1), delegate->pass(domain, sp); // x^N -> x^N+1
        sp.collect_part();
        current_1 += sp; // J^+
        charge_1 += sp; // rho^N+1
        lambda += sp; // L^N+1
        gamma += sp; // G^+
    }
    //
    // 3. gather, smooth and average charge and current densities
    //
    delegate->gather(domain, current_0), delegate->pass(domain, current_0);
    delegate->gather(domain, charge_0), delegate->pass(domain, charge_0);
    delegate->gather(domain, current_1), delegate->pass(domain, current_1);
    delegate->gather(domain, charge_1), delegate->pass(domain, charge_1);
    delegate->gather(domain, lambda);
    delegate->gather(domain, gamma);
    //
    (charge_0 += charge_1) *= Scalar{.5}; // rho^N+1/2
    (current_0 += current_1) *= Vector{.5}; // J^N+1/2
    //
    for (long i = 0; i < Input::Nsmooths; ++i) {
        charge_0.smooth(), delegate->pass(domain, charge_0);
        current_0.smooth(), delegate->pass(domain, current_0);
    }
    //
    // 4. subcycle magnetic field by full step
    //
    subcycle(domain, charge_0, current_0, dt);
    //
    // 5. calculate electric field* and advance current density
    //
    efield.update(bfield, charge_1, current_0), delegate->pass(domain, efield);
    current_1.advance(lambda, gamma, bfield, efield, dt/2.0), delegate->pass(domain, current_1);
    for (long i = 0; i < Input::Nsmooths; ++i) {
        charge_1.smooth(), delegate->pass(domain, charge_1);
        current_1.smooth(), delegate->pass(domain, current_1);
    }
    //
    // 6. calculate electric field
    //
    efield.update(bfield, charge_1, current_1), delegate->pass(domain, efield);
}
void H1D::Domain_CAMCL::subcycle(Domain const &domain, Charge const &charge, Current const &current, Real const _Dt)
{
    BField &bfield_0 = this->bfield;
    constexpr long m = Input::Nsubcycles;
    static_assert(m >= 2, "invalid Nsubcycles");
    Real const dt = _Dt/m, dt_x_2 = dt*2.0;
    //
    // prologue
    //
    bfield_1 = bfield_0;
    efield.update(bfield_0, charge, current), delegate->pass(domain, efield);
    bfield_1.update(efield, dt), delegate->pass(domain, bfield_1);
    //
    // loop
    //
    for (long i = 1; i < m; ++i) {
        efield.update(bfield_1, charge, current), delegate->pass(domain, efield);
        bfield_0.update(efield, dt_x_2), delegate->pass(domain, bfield_0);
        bfield_0.swap(bfield_1);
    }
    //
    // epilogue
    //
    efield.update(bfield_1, charge, current), delegate->pass(domain, efield);
    bfield_0.update(efield, dt), delegate->pass(domain, bfield_0);
    //
    // average
    //
    (bfield_0 += bfield_1) *= Vector{.5};
}
