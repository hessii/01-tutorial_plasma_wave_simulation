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
#include "../ParamSet.h"
#include "../Geometry.h"

#include <array>
#include <tuple>
#include <utility>

PIC1D_BEGIN_NAMESPACE
class Delegate;

class Domain {
public:
    ParamSet const params;
    Geometry const geomtr;
    Delegate *const delegate;
    std::array<PartSpecies, ParamSet::part_indices::size()> part_species;
    std::array<ColdSpecies, ParamSet::cold_indices::size()> cold_species;
    BField bfield;
    EField efield;
    Current current;
private:
    BField bfield_1; // temporary B at half time step
    Current J;
    bool is_recurring_pass{false};

public:
    ~Domain();
    explicit Domain(ParamSet const& params, Delegate *delegate);

    void advance_by(unsigned const n_steps);
private:
    void cycle(Domain const &domain);
    template <class Species>
    Current const& collect_smooth(Current &J, Species const &sp) const;

    template <class... Ts, class Int, Int... Is>
    static auto make_part_species(ParamSet const& params, std::tuple<Ts...> const& descs, std::integer_sequence<Int, Is...>);
    template <class... Ts, class Int, Int... Is>
    static auto make_cold_species(ParamSet const& params, std::tuple<Ts...> const& descs, std::integer_sequence<Int, Is...>);
};
PIC1D_END_NAMESPACE

#endif /* Domain_h */
