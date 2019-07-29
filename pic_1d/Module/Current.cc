//
//  Current.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./Current.h"
#include "./Species.h"
#include "./EField.h"
#include "./BField.h"
#include "./Charge.h"
#include "../InputWrapper.h"

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
P1D::Current &P1D::Current::operator+=(Species const &sp) noexcept
{
    ::accumulate(this->dead_begin(), sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), sp.current_density_conversion_factor());
    return *this;
}

P1D::Gamma &P1D::Gamma::operator+=(Species const &sp) noexcept
{
    ::accumulate(this->dead_begin(), sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), sp.current_density_conversion_factor()*sp.Oc/Input::O0);
    return *this;
}

// current advance
//
void P1D::Current::advance(Lambda const &lambda, Gamma const &gamma, BField const &bfield, EField const &efield, Real const dt) noexcept
{
    _advance(*this, lambda, gamma, bfield, efield, dt);
}
void P1D::Current::_advance(Current &J, Lambda const &L, Gamma const &G, BField const &B, EField const &E, Real const dt) noexcept
{
    for (long i = 0; i < J.size(); ++i) {
        Vector const Bi = (B[i+1] + B[i+0])*0.5;
        J[i] += (E[i]*Real{L[i]} + cross(G[i], Bi)) *= dt;
    }
}
