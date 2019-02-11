//
//  ParticleRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "ParticleRecorder.h"
#include "../Utility/println.h"
#include "../InputWrapper.h"

std::string H1D::ParticleRecorder::filepath(long const step_count, unsigned const sp_id)
{
    constexpr char prefix[] = "particle";
    std::string const filename = std::string{prefix} + "-sp_" + std::to_string(sp_id) + "-" + std::to_string(step_count) + ".csv";
    return std::string{Input::working_directory} + "/" + filename;
}

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
    //
    for (unsigned s = 0; s < domain.species.size(); ++s) {
        os.open(filepath(step_count, s + 1), os.trunc);
        {
            record(os, domain.species[s], Input::Ndumps.at(s));
        }
        os.close();
    }
}
void H1D::ParticleRecorder::record(std::ostream &os, Species const &sp, long max_count)
{
    // header line
    //
    println(os, "v1, v2, v3, x");

    // contents
    //
    for (Particle const &ptl : sp.bucket) {
        if (max_count-- <= 0) break;
        //
        Vector const vel = cart2fac(ptl.vel);
        println(os, vel.x, ", ", vel.y, ", ", vel.z, ", ", ptl.pos_x);
    }
}
