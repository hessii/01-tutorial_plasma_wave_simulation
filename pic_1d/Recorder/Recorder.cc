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

P1D::Recorder::message_dispatch_t P1D::Recorder::dispatch;
P1D::Recorder::Recorder(unsigned const recording_frequency, unsigned const rank, unsigned const size)
: recording_frequency{recording_frequency ? recording_frequency*Input::inner_Nt : large_int}
, comm{dispatch.comm(rank)}, size{size}, all_but_master{} {
    for (unsigned rank = 0; is_master() && rank < size; ++rank) {
        all_but_master.emplace_hint(all_but_master.end(), rank);
    }
    all_but_master.erase(master);
}
