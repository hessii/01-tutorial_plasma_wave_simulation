//
//  DomainBase.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef DomainBase_h
#define DomainBase_h

#include "./BField.h"
#include "./EField.h"
#include "./Charge.h"
#include "./Current.h"
#include "./Species.h"
#include "../Inputs.h"

HYBRID1D_BEGIN_NAMESPACE
class _Domain {
public:
    // member variables
    //
    BField bfield;
    EField efield;
    Charge charge;
    Lambda lambda;
    Current current;
    Gamma gamma;
};
HYBRID1D_END_NAMESPACE

#endif /* DomainBase_h */
