//
//  FieldRecorder.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "FieldRecorder.h"
#include "../Utility/println.h"
#include "../InputWrapper.h"

#include <stdexcept>

std::string P1D::FieldRecorder::filepath(std::string const &wd, long const step_count) const
{
    constexpr char prefix[] = "field";
    std::string const filename = std::string{prefix} + "-" + std::to_string(step_count) + ".csv";
    return is_master() ? wd + "/" + filename : null_dev;
}

P1D::FieldRecorder::FieldRecorder(unsigned const rank, unsigned const size)
: Recorder{Input::field_recording_frequency, rank, size} {
    // configure output stream
    //
    os.setf(os.scientific);
    os.precision(15);
}

void P1D::FieldRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;
    //
    std::string const path = filepath(domain.params.working_directory, step_count);
    if (os.open(path, os.trunc); !os) {
        throw std::invalid_argument{std::string{__FUNCTION__} + " - open failed: " + path};
    } else {
        // header lines
        //
        print(os, "step = ", step_count, "; ");
        print(os, "time = ", step_count*domain.params.dt, "; ");
        print(os, "Dx = ", domain.params.Dx, "; ");
        print(os, "Nx = ", domain.params.Nx, '\n');
        //
        print(os, "dB1, dB2, dB3") << ", ";
        print(os, "dE1, dE2, dE3") << '\n';

        // contents
        //
        auto printer = [&os = this->os](Vector const &v)->std::ostream &{
            return print(os, v.x, ", ", v.y, ", ", v.z);
        };
        //
        auto tk = comm.send(std::make_pair(domain.bfield.begin(), domain.efield.begin()), master);
        if (is_master()) {
            using Payload = std::pair<Vector const*, Vector const*>;
            auto const writer = [printer, &geomtr = domain.geomtr, Nx = domain.bfield.size()](Payload payload) {
                auto [bfield, efield] = payload;
                for (long i = 0; i < Nx; ++i) {
                    printer(geomtr.cart2fac(bfield[i] - geomtr.B0)) << ", ";
                    printer(geomtr.cart2fac(efield[i])) << '\n';
                }
            };
            auto all = all_but_master;
            all.insert(master);
            comm.for_each<Payload>(all, writer);
        }
        std::move(tk).wait();
    }
    os.close();
}
