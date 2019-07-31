//
//  Domain.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Domain_h
#define Domain_h

#include "./BField.h"
#include "./EField.h"
#include "./Charge.h"
#include "./Current.h"
#include "./Species.h"
#include "../InputWrapper.h"

#include <array>

PIC1D_BEGIN_NAMESPACE
class Delegate;

class Domain {
public:
    Delegate *const delegate;
    std::array<Species, Input::PtlDesc::Ns> species;
    BField bfield;
    EField efield;
    Charge charge;
    Current current;

    virtual ~Domain() = default;
    virtual void advance_by(unsigned const n_steps) = 0;

protected:
    explicit Domain(Delegate *delegate);
};
PIC1D_END_NAMESPACE

#endif /* Domain_h */