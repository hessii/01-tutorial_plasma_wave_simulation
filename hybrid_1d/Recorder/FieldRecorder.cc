//
//  FieldRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "FieldRecorder.h"
#include "../Utility/println.h"
#include "../Inputs.h"

std::string H1D::FieldRecorder::filepath()
{
    constexpr char filename[] = "field.m";
    return std::string{Input::working_directory} + "/" + filename;
}

H1D::FieldRecorder::FieldRecorder()
: Recorder{Input::field_recording_frequency} {
    // open output stream
    //
    {
        os.open(filepath(), os.trunc);
        os.setf(os.scientific);
        os.precision(15);
    }

    // insert preambles
    //
    println(os, "Dx = ", Input::Dx);
    println(os, "Nx = ", Input::Nx);

    println(os, "step = {}"); // integral step count
    println(os, "time = {}"); // simulation time
    println(os, "dB = {}"); // without background B
    println(os, "dE = {}");
    (os << std::endl).flush();
}

void H1D::FieldRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;

    println(os, "step = step ~ Append ~ ", step_count);
    println(os, "time = time ~ Append ~ ", step_count*Input::dt);

    println(os, "dB = dB ~ Append ~ ", dump(domain.bfield));
    println(os, "dE = dE ~ Append ~ ", dump(domain.efield));

    (os << std::endl).flush();
}

auto H1D::FieldRecorder::dump(BField const &bfield) noexcept
-> GridQ<Vector> const &{
    auto lhs_first = ws.begin();
    auto rhs_first = bfield.begin(), rhs_last = bfield.end();
    while (rhs_first != rhs_last) {
        *lhs_first++ = cart2fac(*rhs_first++ - bfield.B0);
    }
    return ws;
}
auto H1D::FieldRecorder::dump(EField const &efield) noexcept
-> GridQ<Vector> const &{
    auto lhs_first = ws.begin();
    auto rhs_first = efield.begin(), rhs_last = efield.end();
    while (rhs_first != rhs_last) {
        *lhs_first++ = cart2fac(*rhs_first++);
    }
    return ws;
}
