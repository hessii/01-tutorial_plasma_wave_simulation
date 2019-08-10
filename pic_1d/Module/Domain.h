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
#include "./Current.h"
#include "./PartSpecies.h"
#include "../InputWrapper.h"

#include <array>

PIC1D_BEGIN_NAMESPACE
class Delegate;

class Domain {
    bool is_recurring_pass{false};
    BField bfield_1; // temporary B at half time step
public:
    Delegate *const delegate;
    std::array<PartSpecies, Input::PartDesc::Ns> part_species;
    BField bfield;
    EField efield;
    Current current;

    ~Domain();
    explicit Domain(Delegate *delegate);

    void advance_by(unsigned const n_steps);
private:
    void cycle(Domain const &domain);
};
PIC1D_END_NAMESPACE

#endif /* Domain_h */
