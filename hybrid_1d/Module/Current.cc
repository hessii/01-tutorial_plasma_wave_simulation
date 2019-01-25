//
//  Current.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./Current.h"
#include "./Species.h"
#include "./EField.h"
#include "./BField.h"
#include "./Charge.h"

// helper
//
namespace {
    template <class LIt, class RIt, class U>
    void accumulate(LIt lhs_first, RIt rhs_first, RIt const rhs_last, U const &weight) noexcept {
        while (rhs_first != rhs_last) {
            *lhs_first++ += *rhs_first++ * weight;
        }
    }
}

// current collector
//
H1D::Current &H1D::Current::operator+=(Species const &sp) noexcept
{
    ::accumulate(this->dead_begin(), sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), sp.current_density_conversion_factor());
    return *this;
}

H1D::Gamma &H1D::Gamma::operator+=(Species const &sp) noexcept
{
    ::accumulate(this->dead_begin(), sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), sp.current_density_conversion_factor()*sp.Oc/Input::O0);
    return *this;
}

// current advance
//
void H1D::Current::advance(Lambda const &lambda, Gamma const &gamma, BField const &bfield, EField const &efield, Real const dt) noexcept
{
    _advance(*this, lambda, gamma, bfield, efield, dt);
}
void H1D::Current::_advance(Current &J, Lambda const &L, Gamma const &G, BField const &B, EField const &E, Real const dt) noexcept
{
    for (long i = 0; i < J.size(); ++i) {
        Vector const &Ei = (E[i-0] + E[i-1])*0.5;
        J[i] += (Ei*Real{L[i]} + cross(G[i], B[i])) *= dt;
    }
}
