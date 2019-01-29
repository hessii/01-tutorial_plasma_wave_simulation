//
//  FieldRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "FieldRecorder.h"
#include "../Inputs.h"

H1D::FieldRecorder::FieldRecorder()
: Recorder(Input::field_recording_frequency) {
    // open output stream
    //
    {
        constexpr char filename[] = "field.m";
        os.open(filename, os.trunc);
        os.setf(os.scientific | os.showpos);
        os.precision(15);
    }

    // insert preambles
    //
    os << "step = {}" << std::endl; // integral step count
    os << "time = {}" << std::endl; // simulation time
    os << "dB = {}" << std::endl; // without background B
    os << "dE = {}" << std::endl;
    (os << std::endl).flush();
}

void H1D::FieldRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;

    os << "step = step ~ Append ~ " << step_count << std::endl;
    os << "time = time ~ Append ~ " << step_count*Input::dt << std::endl;

    os << "dB = dB ~ Append ~ " << dump(domain.bfield) << std::endl;
    os << "dE = dE ~ Append ~ " << dump(domain.efield) << std::endl;

    (os << std::endl).flush();
}

auto H1D::FieldRecorder::dump(BField const &bfield) noexcept
-> GridQ<Vector> const &{
    auto lhs_first = ws.begin();
    auto rhs_first = bfield.begin(), rhs_last = bfield.end();
    while (rhs_first != rhs_last) {
        *lhs_first++ = fac(*rhs_first++) - Vector{Input::O0, 0, 0};
    }
    return ws;
}
auto H1D::FieldRecorder::dump(EField const &efield) noexcept
-> GridQ<Vector> const &{
    auto lhs_first = ws.begin();
    auto rhs_first = efield.begin(), rhs_last = efield.end();
    while (rhs_first != rhs_last) {
        *lhs_first++ = fac(*rhs_first++);
    }
    return ws;
}
