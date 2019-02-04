//
//  ParticleRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "ParticleRecorder.h"
#include "../Utility/println.h"
#include "../Inputs.h"

H1D::ParticleRecorder::ParticleRecorder()
: Recorder{Input::particle_recording_frequency} {
    // setup output stream
    //
    os.setf(os.scientific);
    os.precision(15);
}

void H1D::ParticleRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;

    os.open(filepath(step_count), os.trunc);
    {
        println(os, "step = ", step_count);
        println(os, "time = ", step_count*Input::dt);

        print(os, "particle = {Sequence[]");
        for (Species const &sp : domain.species) {
            print(os, ",\n", sp.bucket);
        }
        println(os, "\n}");

        (os << std::endl).flush();
    }
    os.close();
}

std::string H1D::ParticleRecorder::filepath(long const step_count) const
{
    constexpr char prefix[] = "particle";
    std::string const filename = std::string(prefix) + "_" + std::to_string(step_count) + ".m";
    return std::string(Input::working_directory) + "/" + filename;
}
