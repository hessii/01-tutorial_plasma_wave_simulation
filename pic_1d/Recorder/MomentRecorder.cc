//
//  MomentRecorder.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "MomentRecorder.h"
#include "../Utility/println.h"
#include "../InputWrapper.h"

#include <stdexcept>

std::string P1D::MomentRecorder::filepath(std::string const &wd, long const step_count) const
{
    constexpr char prefix[] = "moment";
    std::string const filename = std::string{prefix} + "-" + std::to_string(step_count) + ".csv";
    return is_master() ? wd + "/" + filename : null_dev;
}

P1D::MomentRecorder::MomentRecorder(unsigned const rank, unsigned const size)
: Recorder{Input::moment_recording_frequency, rank, size} {
    // configure output stream
    //
    os.setf(os.scientific);
    os.precision(15);
}

void P1D::MomentRecorder::record(const Domain &domain, const long step_count)
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
        print(os, "Nx = ", domain.params.Nx, "; ");
        print(os, "Ns = ", domain.part_species.size() + domain.cold_species.size(), '\n');
        //
        for (unsigned i = 1; i <= domain.part_species.size(); ++i) {
            if (i - 1) print(os, ", ");
            //
            print(os, "part_species(", i, ") <1>");
            print(os, ", part_species(", i, ") <v1>", ", part_species(", i, ") <v2>", ", part_species(", i, ") <v3>");
            print(os, ", part_species(", i, ") <v1v1>", ", part_species(", i, ") <v2v2>", ", part_species(", i, ") <v3v3>");
        }
        if (!domain.part_species.empty() && !domain.cold_species.empty()) print(os, ", ");
        for (unsigned i = 1; i <= domain.cold_species.size(); ++i) {
            if (i - 1) print(os, ", ");
            //
            print(os, "cold_species(", i, ") <1>");
            print(os, ", cold_species(", i, ") <v1>", ", cold_species(", i, ") <v2>", ", cold_species(", i, ") <v3>");
            print(os, ", cold_species(", i, ") <v1v1>", ", cold_species(", i, ") <v2v2>", ", cold_species(", i, ") <v3v3>");
        }
        //
        print(os, '\n');

        // contents
        //
        auto printer = [&os = this->os](Vector const &v)->std::ostream &{
            return print(os, v.x, ", ", v.y, ", ", v.z);
        };
        //
        auto tk = comm.send(std::make_pair(domain.part_species.begin(), domain.cold_species.begin()), master);
        if (is_master()) {
            using Payload = std::pair<PartSpecies const*, ColdSpecies const*>;
            auto const unpack = [&os = this->os, Nx = domain.bfield.size(), Ns_part = domain.part_species.size(), Ns_cold = domain.cold_species.size()](Payload payload, auto printer) {
                auto [part_species, cold_species] = payload;
                for (long i = 0; i < Nx; ++i) {
                    for (unsigned s = 0; s < Ns_part; ++s) {
                        if (s) print(os, ", ");
                        //
                        Species const &sp = part_species[s];
                        print(os, Real{sp.moment<0>()[i]}, ", ");
                        printer(sp.geomtr.cart2fac(sp.moment<1>()[i])) << ", ";
                        printer(sp.geomtr.cart2fac(sp.moment<2>()[i]));
                    }
                    if (Ns_part > 0 && Ns_cold > 0) print(os, ", ");
                    for (unsigned s = 0; s < Ns_cold; ++s) {
                        if (s) print(os, ", ");
                        //
                        Species const &sp = cold_species[s];
                        print(os, Real{sp.moment<0>()[i]}, ", ");
                        printer(sp.geomtr.cart2fac(sp.moment<1>()[i])) << ", ";
                        printer(sp.geomtr.cart2fac(sp.moment<2>()[i]));
                    }
                    //
                    print(os, '\n');
                }
            };
            auto all = all_but_master;
            all.insert(master);
            auto pkgs = comm.gather<Payload>(all);
            for (auto&& pkg : pkgs) {
                std::move(pkg).unpack(unpack, printer);
            }
        }
        std::move(tk).wait();
    }
    os.close();
}
