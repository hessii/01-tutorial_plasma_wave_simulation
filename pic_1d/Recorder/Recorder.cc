//
//  Recorder.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Recorder.h"
#include "../InputWrapper.h"

#include <limits>
#include <stdexcept>

namespace {
    constexpr long large_int = std::numeric_limits<unsigned>::max();
}

P1D::Recorder::message_dispatch_t P1D::Recorder::dispatch{P1D::ParamSet::number_of_subdomains};
P1D::Recorder::Recorder(unsigned const recording_frequency, unsigned const rank, unsigned const size)
: recording_frequency{recording_frequency ? recording_frequency*Input::inner_Nt : large_int}
, comm{dispatch.comm(rank)}, size{size}, all_ranks{}, all_but_master{} {
    if (size > ParamSet::number_of_subdomains) {
        throw std::invalid_argument{__PRETTY_FUNCTION__};
    }

    for (unsigned rank = 0; is_master() && rank < size; ++rank) {
        all_ranks.emplace_back(rank);
        if (master != rank) all_but_master.emplace_back(rank);
    }
}
