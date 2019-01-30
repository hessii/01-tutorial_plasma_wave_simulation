//
//  MomentRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MomentRecorder.h"
#include "../Utility/println.h"
#include "../Inputs.h"

std::string H1D::MomentRecorder::filepath()
{
    constexpr char filename[] = "moment.m";
    return std::string(Input::working_directory) + "/" + filename;
}

H1D::MomentRecorder::MomentRecorder()
: Recorder(Input::moment_recording_frequency) {
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
    println(os, "mom0 = {}"); // number density; n
    println(os, "mom1 = {}"); // 1st moment; nV
    println(os, "mom2 = {}"); // 2nd moment; diagonal components
    (os << std::endl).flush();
}

void H1D::MomentRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;

    println(os, "step = step ~ Append ~ ", step_count);
    println(os, "time = time ~ Append ~ ", step_count*Input::dt);

    print(os, "mom0 = mom0 ~ Append ~ {Sequence[]");
    for (Species const &sp : domain.species) {
        print(os, ",\n", dump(sp.moment<0>()));
    }
    println(os, "\n}");

    print(os, "mom1 = mom1 ~ Append ~ {Sequence[]");
    for (Species const &sp : domain.species) {
        print(os, ",\n", dump(sp.moment<1>()));
    }
    println(os, "\n}");

    print(os, "mom2 = mom2 ~ Append ~ {Sequence[]");
    for (Species const &sp : domain.species) {
        print(os, ",\n", dump(sp.moment<2>()));
    }
    println(os, "\n}");

    (os << std::endl).flush();
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
