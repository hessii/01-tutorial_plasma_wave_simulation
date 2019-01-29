//
//  ParticleRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "ParticleRecorder.h"
#include "../Inputs.h"

H1D::ParticleRecorder::ParticleRecorder()
: Recorder(Input::particle_recording_frequency) {
    // setup output stream
    //
    os.setf(os.scientific | os.showpos);
    os.precision(15);
}

void H1D::ParticleRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;

    os.open(filename(step_count), os.trunc);
    {
        os << "step = " << step_count << std::endl;
        os << "time = " << step_count*Input::dt << std::endl;

        os << "particle = {Sequence[]";
        for (Species const &sp : domain.species) {
            os << ",\n" << sp.bucket;
        }
        os << "\n}" << std::endl;

        (os << std::endl).flush();
    }
    os.close();
}

std::string H1D::ParticleRecorder::filename(long const step_count) const
{
    constexpr char prefix[] = "particle";
    return std::string(prefix) + "_" + std::to_string(step_count) + ".m";
}
