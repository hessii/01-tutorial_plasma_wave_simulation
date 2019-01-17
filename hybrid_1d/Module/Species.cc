//
//  Species.c
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "./Species.h"
#include "./EField.h"
#include "./BField.h"
#include "../Utility/ParticlePush.h"
#include "../Inputs.h"

H1D::Species::Species(Real const Oc, Real const op, long const Nc)
: Nc(Nc), Oc(Oc), op(op), bucket(), _mom() {
    bucket.resize(static_cast<unsigned long>(Nc*Input::Nx));
}
