//
//  Recorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "Recorder.h"
#include "../Inputs.h"

#include <limits>

namespace {
    constexpr long large_int = std::numeric_limits<int>::max();
}

H1D::Recorder::Recorder(unsigned const recording_period) noexcept
: recording_period(recording_period ? recording_period*Input::inner_Nt : large_int) {
}
