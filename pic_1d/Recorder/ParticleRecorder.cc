//
//  ParticleRecorder.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "ParticleRecorder.h"
#include "../Utility/println.h"
#include "../InputWrapper.h"

std::string P1D::ParticleRecorder::filepath(long const step_count, unsigned const sp_id)
{
    constexpr char prefix[] = "particle";
    std::string const filename = std::string{prefix} + "-sp_" + std::to_string(sp_id) + "-" + std::to_string(step_count) + ".csv";
    return std::string{Input::working_directory} + "/" + filename;
}

P1D::ParticleRecorder::ParticleRecorder()
: Recorder{Input::particle_recording_frequency} {
    // setup output stream
    //
    os.setf(os.scientific);
    os.precision(15);
}

void P1D::ParticleRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;
    //
    for (unsigned s = 0; s < domain.part_species.size(); ++s) {
        os.open(filepath(step_count, s + 1), os.trunc);
        {
            // header lines
            //
            print(os, "step = ", step_count, "; ");
            print(os, "time = ", step_count*Input::dt, "; ");
            print(os, "Dx = ", Input::Dx, "; ");
            print(os, "Nx = ", Input::Nx, "; ");
            print(os, "species = ", s, '\n');
            println(os, "v1, v2, v3, x");

            // contents
            //
            record(os, domain.part_species[s], Input::Ndumps.at(s));
        }
        os.close();
    }
}
void P1D::ParticleRecorder::record(std::ostream &os, PartSpecies const &sp, long max_count)
{
    for (Particle const &ptl : sp.bucket) {
        if (max_count-- <= 0) break;
        //
        Vector const vel = cart2fac(ptl.vel);
        println(os, vel.x, ", ", vel.y, ", ", vel.z, ", ", ptl.pos_x);
    }
}
