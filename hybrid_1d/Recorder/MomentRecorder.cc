//
//  MomentRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "MomentRecorder.h"
#include "../Utility/println.h"
#include "../InputWrapper.h"

std::string H1D::MomentRecorder::filepath(long const step_count)
{
    constexpr char prefix[] = "moment";
    std::string const filename = std::string{prefix} + "-" + std::to_string(step_count) + ".csv";
    return std::string{Input::working_directory} + "/" + filename;
}

H1D::MomentRecorder::MomentRecorder()
: Recorder{Input::field_recording_frequency} {
    // open output stream
    //
    os.setf(os.scientific);
    os.precision(15);
}

void H1D::MomentRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;
    //
    os.open(filepath(step_count), os.trunc);
    {
        // header lines
        //
        print(os, "step = ", step_count, "; ");
        print(os, "time = ", step_count*Input::dt, "; ");
        print(os, "Dx = ", Input::Dx, "; ");
        print(os, "Nx = ", Input::Nx, "; ");
        print(os, "Ns = ", Input::iKinetic::Ns, '\n');
        //
        for (long i = 1; i <= Input::iKinetic::Ns; ++i) {
            if (i - 1) print(os, ", ");
            //
            print(os, "species(", i, ") <1>");
            print(os, ", species(", i, ") <v1>", ", species(", i, ") <v2>", ", species(", i, ") <v3>");
            print(os, ", species(", i, ") <v1v1>", ", species(", i, ") <v2v2>", ", species(", i, ") <v3v3>");
        }
        print(os, '\n');

        // contents
        //
        auto printer = [&os = this->os](Vector const &v)->std::ostream &{
            return print(os, v.x, ", ", v.y, ", ", v.z);
        };
        //
        for (long i = 0; i < Input::Nx; ++i) {
            for (unsigned s = 0; s < domain.species.size(); ++s) {
                if (s) print(os, ", ");
                //
                Species const &sp = domain.species[s];
                print(os, Real{sp.moment<0>()[i]}, ", ");
                printer(cart2fac(sp.moment<1>()[i])) << ", ";
                printer(cart2fac(sp.moment<2>()[i]));
            }
            print(os, '\n');
        }
    }
    os.close();
}
