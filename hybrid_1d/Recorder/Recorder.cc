//
//  Recorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "Recorder.h"
#include <Inputs.h>

#include <limits>
#include <cmath>

namespace {
    constexpr long large_int = std::numeric_limits<unsigned>::max();
}

H1D::Recorder::Recorder(unsigned const recording_frequency) noexcept
: recording_frequency{recording_frequency ? recording_frequency*Input::inner_Nt : large_int} {
}

H1D::Vector const H1D::Recorder::e3 = {0, 0, 1};
H1D::Vector const H1D::Recorder::e1 = []{
    constexpr Real theta = Input::theta*M_PI/180;
    return Vector{std::cos(theta), std::sin(theta), 0};
}();
H1D::Vector const H1D::Recorder::e2 = []{
    constexpr Real theta = Input::theta*M_PI/180;
    return Vector{-std::sin(theta), std::cos(theta), 0};
}();
