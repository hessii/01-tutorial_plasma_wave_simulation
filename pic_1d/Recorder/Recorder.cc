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

namespace {
    constexpr long large_int = std::numeric_limits<unsigned>::max();
}

P1D::Recorder::Recorder(unsigned const recording_frequency) noexcept
: recording_frequency{recording_frequency ? recording_frequency*Input::inner_Nt : large_int} {
}
