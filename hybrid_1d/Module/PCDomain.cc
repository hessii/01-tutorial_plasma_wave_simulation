//
//  PCDomain.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "PCDomain.h"
#include "Delegate.h"

#include <algorithm>

namespace {
    inline void assign(H1D::GridQ<H1D::Vector> &lhs, H1D::GridQ<H1D::Vector> const &rhs) noexcept {
        std::copy(rhs.dead_begin(), rhs.dead_end(), lhs.dead_begin());
    }
}

void H1D::PCDomain::cycle()
{
    predictor_step();
    corrector_step();
}
void H1D::PCDomain::predictor_step()
{
    //
    // 1. Faraday's law; predict 1
    //
    _assign(*bfield<1>(), *bfield<0>());
    bfield<1>().update(efield<0>(), dt()), delegate()->pass(domain, bfield<1>());
    //
    // 2. Ohm's law; predict 1
    //
    efield<1>().update(bfield<1>(), charge(), current()), delegate()->pass(domain, efield<1>());
    //
    // 3. Average fields: paddings should be valid
    //
    _mean_assign(*bfield<1>(), *bfield<0>());
    _mean_assign(*efield<1>(), *efield<0>()), _mean_assign(efield<1>()[1], efield<0>()[1]);
    //
    // 4 & 5. Particle push and deposit charge and current densities; predict
    //
    charge().reset();
    current().reset();
    for (Species<ND> &_sp0 : species()) {
        Species<ND> &sp = *_species_proxy = _sp0;
        sp.update(dt()/2.0, 0.5);
        sp.update(bfield<1>(), efield<1>(), dt());
        sp.update(dt()/2.0, 0.5), delegate()->pass(domain, sp);
        sp.collect_scalar_vector_moments();
        charge().collect(sp);
        current().collect(sp);
    }
    delegate()->gather(domain, current()), delegate()->pass(domain, current());
    delegate()->gather(domain, charge()), delegate()->pass(domain, charge());
    for (unsigned i_smooth = 0; i_smooth < _number_of_smoothing_passes; ++i_smooth) {
        current().smooth(), delegate()->pass(domain, current());
        charge().smooth(), delegate()->pass(domain, charge());
    }
    //
    // 6. Faraday's law; predict 2
    //
    _assign(*bfield<1>(), *bfield<0>());
    bfield<1>().update(efield<1>(), dt()), delegate()->pass(domain, bfield<1>());
    //
    // 7. Ohm's law; predict 2
    //
    efield<1>().update(bfield<1>(), charge(), current()), delegate()->pass(domain, efield<1>());
    //
    // 8. Average fields: paddings should be valid
    //
    _mean_assign(*bfield<1>(), *bfield<0>());
    _mean_assign(*efield<1>(), *efield<0>()), _mean_assign(efield<1>()[1], efield<0>()[1]);
}
void H1D::PCDomain::corrector_step()
{
    //
    // 9 & 10. Particle push and deposit charge and current densities; correct
    //
    charge().reset();
    current().reset();
    for (Species<ND> &sp : species()) {
        sp.update(dt()/2.0, 0.5);
        sp.update(bfield<1>(), efield<1>(), dt());
        sp.update(dt()/2.0, 0.5), delegate()->pass(domain, sp);
        sp.collect_scalar_vector_moments();
        charge().collect(sp);
        current().collect(sp);
    }
    delegate()->gather(domain, current()), delegate()->pass(domain, current());
    delegate()->gather(domain, charge()), delegate()->pass(domain, charge());
    for (unsigned i_smooth = 0; i_smooth < _number_of_smoothing_passes; ++i_smooth) {
        current().smooth(), delegate()->pass(domain, current());
        charge().smooth(), delegate()->pass(domain, charge());
    }
    //
    // 11. Faraday's law; correct
    //
    bfield<0>().update(efield<1>(), dt()), delegate()->pass(domain, bfield<0>());
    //
    // 12. Ohm's law; correct
    //
    efield<0>().update(bfield<0>(), charge(), current()), delegate()->pass(domain, efield<0>());
}
