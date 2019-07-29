//
//  Domain.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Domain.h"
#include "../VDF/MaxwellianVDF.h"

#include <cmath>

P1D::Domain::Domain(Delegate *delegate)
: delegate{delegate} {
    for (unsigned i = 0; i < species.size(); ++i) {
        using namespace Input::PtlDesc;
        species.at(i) = Species{Ocs.at(i), ops.at(i), Ncs.at(i), [i]{
            Real const vth1 = std::sqrt(betas.at(i))*Input::c * std::abs(Ocs.at(i))/ops.at(i);
            return MaxwellianVDF{vth1, T2OT1s.at(i)};
        }()};
    }
}
