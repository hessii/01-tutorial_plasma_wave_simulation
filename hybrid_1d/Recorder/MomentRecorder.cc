//
//  MomentRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MomentRecorder.h"
#include "../Inputs.h"

H1D::MomentRecorder::MomentRecorder()
: Recorder(Input::moment_recording_frequency) {
    // open output stream
    //
    {
        constexpr char filename[] = "moment.m";
        os.open(filename, os.trunc);
        os.setf(os.scientific | os.showpos);
        os.precision(15);
    }

    // insert preambles
    //
    os << "step = {}" << std::endl; // integral step count
    os << "time = {}" << std::endl; // simulation time
    os << "mom0 = {}" << std::endl; // number density; n
    os << "mom1 = {}" << std::endl; // 1st moment; nV
    os << "mom2 = {}" << std::endl; // 2nd moment; diagonal components
    os << std::endl;
}

void H1D::MomentRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;

    os << "step = step ~ Append ~ " << step_count << std::endl;
    os << "time = time ~ Append ~ " << step_count*Input::dt << std::endl;

    os << "mom0 = mom0 ~ Append ~ {Sequence[]";
    for (Species const &sp : domain.species) {
        os << ",\n" << dump(sp.moment<0>());
    }
    os << "\n}" << std::endl;

    os << "mom1 = mom1 ~ Append ~ {Sequence[]";
    for (Species const &sp : domain.species) {
        os << ",\n" << dump(sp.moment<1>());
    }
    os << "\n}" << std::endl;

    os << "mom2 = mom2 ~ Append ~ {Sequence[]";
    for (Species const &sp : domain.species) {
        os << ",\n" << dump(sp.moment<2>());
    }
    os << "\n}" << std::endl;

    os << std::endl;
}

template <class T>
auto H1D::MomentRecorder::dump(GridQ<T> const &mom12) noexcept
-> GridQ<Vector> const &{
    auto lhs_first = ws.begin();
    auto rhs_first = mom12.begin(), rhs_last = mom12.end();
    while (rhs_first != rhs_last) {
        *lhs_first++ = fac(*rhs_first++);
    }
    return ws;
}
