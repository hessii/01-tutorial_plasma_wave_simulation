//
//  Charge.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./Charge.h"
#include "./Species.h"
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

// density collector
//
P1D::Charge &P1D::Charge::operator+=(Species const &sp) noexcept
{
    ::accumulate(this->dead_begin(), sp.moment<0>().dead_begin(), sp.moment<0>().dead_end(), sp.charge_density_conversion_factor());
    return *this;
}

P1D::Lambda &P1D::Lambda::operator+=(Species const &sp) noexcept
{
    ::accumulate(this->dead_begin(), sp.moment<0>().dead_begin(), sp.moment<0>().dead_end(), sp.charge_density_conversion_factor()*sp.Oc/Input::O0);
    return *this;
}
