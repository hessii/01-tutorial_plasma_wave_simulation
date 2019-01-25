//
//  PCDomain.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "PCDomain.h"
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

H1D::PCDomain::PCDomain()
{
}

void H1D::PCDomain::advance_by(unsigned const n_steps)
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
        //
        // 0.1 Deposit charge and current densities
        //
        charge.reset();
        current.reset();
        for (Species &sp : species) {
            sp.collect_part();
            charge += sp;
            current += sp;
        }
        delegate->gather(domain, current), delegate->pass(domain, current);
        delegate->gather(domain, charge), delegate->pass(domain, charge);
        for (long i = 0; i < Input::Nsmooths; ++i) {
            current.smooth(), delegate->pass(domain, current);
            charge.smooth(), delegate->pass(domain, charge);
        }
        //
        // 0.2 Ohm's law
        // NOTE: check whether efield<1> properties are same as those of efield<0>
        //
        efield.update(bfield, charge, current), delegate->pass(domain, efield);
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
void H1D::PCDomain::cycle(Domain const &domain)
{
    predictor_step(domain);
    corrector_step(domain);
}
void H1D::PCDomain::predictor_step(Domain const &domain)
{
    BField &bfield_0 = this->bfield;
    EField &efield_0 = this->efield;
    using Input::dt;
    //
    // 1. Faraday's law; predict 1
    //
    bfield_1 = bfield_0;
    bfield_1.update(efield_0, dt), delegate->pass(domain, bfield_1);
    //
    // 2. Ohm's law; predict 1
    //
    efield_1.update(bfield_1, charge, current), delegate->pass(domain, efield_1);
    //
    // 3. Average fields
    //
    (bfield_1 += bfield_0) *= Vector{.5};
    (efield_1 += efield_0) *= Vector{.5};
    //
    // 4 & 5. Particle push and deposit charge and current densities; predict
    //
    charge.reset();
    current.reset();
    for (Species &_sp : species) {
        predictor = _sp;
        predictor.update_pos(dt/2.0, 0.5);
        predictor.update_vel(bfield_1, efield_1, dt);
        predictor.update_pos(dt/2.0, 0.5), delegate->pass(domain, predictor);
        predictor.collect_part();
        charge += predictor;
        current += predictor;
    }
    delegate->gather(domain, current), delegate->pass(domain, current);
    delegate->gather(domain, charge), delegate->pass(domain, charge);
    for (long i = 0; i < Input::Nsmooths; ++i) {
        current.smooth(), delegate->pass(domain, current);
        charge.smooth(), delegate->pass(domain, charge);
    }
    //
    // 6. Faraday's law; predict 2
    //
    bfield_1 = bfield_0;
    bfield_1.update(efield_1, dt), delegate->pass(domain, bfield_1);
    //
    // 7. Ohm's law; predict 2
    //
    efield_1.update(bfield_1, charge, current), delegate->pass(domain, efield_1);
    //
    // 8. Average fields
    //
    (bfield_1 += bfield_0) *= Vector{.5};
    (efield_1 += efield_0) *= Vector{.5};
}
void H1D::PCDomain::corrector_step(Domain const &domain)
{
    BField &bfield_0 = this->bfield;
    EField &efield_0 = this->efield;
    using Input::dt;
    //
    // 9 & 10. Particle push and deposit charge and current densities; correct
    //
    charge.reset();
    current.reset();
    for (Species &sp : species) {
        sp.update_pos(dt/2.0, 0.5);
        sp.update_vel(bfield_1, efield_1, dt);
        sp.update_pos(dt/2.0, 0.5), delegate->pass(domain, sp);
        sp.collect_part();
        charge += sp;
        current += sp;
    }
    delegate->gather(domain, current), delegate->pass(domain, current);
    delegate->gather(domain, charge), delegate->pass(domain, charge);
    for (long i = 0; i < Input::Nsmooths; ++i) {
        current.smooth(), delegate->pass(domain, current);
        charge.smooth(), delegate->pass(domain, charge);
    }
    //
    // 11. Faraday's law; correct
    //
    bfield_0.update(efield_1, dt), delegate->pass(domain, bfield_0);
    //
    // 12. Ohm's law; correct
    //
    efield_0.update(bfield_0, charge, current), delegate->pass(domain, efield_0);
}
