//
//  Current.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Current.h"
#include "./Species.h"

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

P1D::Current::Current(ParamSet const &params)
: GridQ{}, tmp{}, params{params}, geomtr{params} {
}

// current collector
//
P1D::Current &P1D::Current::operator+=(Species const &sp) noexcept
{
    ::accumulate(this->dead_begin(), sp.moment<1>().dead_begin(), sp.moment<1>().dead_end(), sp.current_density_conversion_factor());
    return *this;
}
