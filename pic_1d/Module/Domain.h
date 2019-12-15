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
#include "./Species.h"
#include "./PartSpecies.h"
#include "./ColdSpecies.h"
#include "../InputWrapper.h"

#include <array>
#include <utility>

PIC1D_BEGIN_NAMESPACE
class Delegate;

class Domain {
    bool is_recurring_pass{false};
    BField bfield_1; // temporary B at half time step
    Current J;
public:
    Delegate *const delegate;
    std::array<PartSpecies, Input::PartDesc::Ns> part_species;
    std::array<ColdSpecies, Input::ColdDesc::Ns> cold_species;
    BField bfield;
    EField efield;
    Current current;

    ~Domain();
    explicit Domain(Delegate *delegate);

    void advance_by(unsigned const n_steps);
private:
    void cycle(Domain const &domain);
    template <class Species>
    Current& collect(Current &J, Species const &sp) const;

    template <class Int, Int... Is> [[deprecated]]
    void init_part_species(std::integer_sequence<Int, Is...>);
    template <class Int, Int... Is> [[deprecated]]
    void init_cold_species(std::integer_sequence<Int, Is...>);
};
PIC1D_END_NAMESPACE

#endif /* Domain_h */
