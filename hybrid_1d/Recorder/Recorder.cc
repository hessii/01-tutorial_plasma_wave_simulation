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
#include <cmath>

namespace {
    constexpr long large_int = std::numeric_limits<int>::max();
}

H1D::Recorder::Recorder(unsigned const recording_period) noexcept
: recording_period(recording_period ? recording_period*Input::inner_Nt : large_int) {
}

namespace {
    constexpr H1D::Real theta = H1D::Input::theta*M_PI/180;
}

H1D::Vector const H1D::Recorder::e1 = {std::cos(theta), std::sin(theta), 0};
H1D::Vector const H1D::Recorder::e2 = {-std::sin(theta), std::cos(theta), 0};
H1D::Vector const H1D::Recorder::e3 = {0, 0, 1};
