//
//  SpeciesBase.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/17/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "SpeciesBase.h"
#include "../Inputs.h"

H1D::_Species::_Species(Real const Oc, Real const op, long const Nc)
: Nc(Nc), Oc(Oc), op(op), bucket(), _mom() {
    bucket.resize(static_cast<unsigned long>(Nc*Input::Nx));
}
