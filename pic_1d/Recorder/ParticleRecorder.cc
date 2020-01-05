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

#include <iterator>
#include <algorithm>

std::string P1D::ParticleRecorder::filepath(long const step_count, unsigned const sp_id) const
{
    constexpr char prefix[] = "particle";
    std::string const filename = std::string{prefix} + "-sp_" + std::to_string(sp_id) + "-" + std::to_string(step_count) + ".csv";
    return is_master() ? std::string{Input::working_directory} + "/" + filename : null_dev;
}

P1D::ParticleRecorder::ParticleRecorder(unsigned const rank, unsigned const size)
: Recorder{Input::particle_recording_frequency, rank, size}, urbg{123 + rank} {
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
        if (!Input::Ndumps.at(s)) continue;
        //
        os.open(filepath(step_count, s + 1), os.trunc);
        {
            // header lines
            //
            print(os, "step = ", step_count, "; ");
            print(os, "time = ", step_count*domain.params.dt, "; ");
            print(os, "Dx = ", domain.params.Dx, "; ");
            print(os, "Nx = ", domain.params.Nx, "; ");
            print(os, "species = ", s, '\n');
            println(os, "v1, v2, v3, x, w");

            // contents
            //
            record(domain.part_species[s], Input::Ndumps.at(s));
        }
        os.close();
    }
}
void P1D::ParticleRecorder::record(PartSpecies const &sp, unsigned const max_count)
{
    PartBucket samples;
    std::sample(sp.bucket.cbegin(), sp.bucket.cend(), std::back_inserter(samples), max_count/size, urbg);
    for (Particle &ptl : samples) {
        ptl.pos_x += sp.params.domain_extent.min();
    }
    //
    auto tk = comm.send(std::move(samples), master);
    for (unsigned rank = 0; is_master() && rank < size; ++rank) {
        comm.recv<PartBucket>(rank).unpack([&os = this->os, &geomtr = sp.geomtr](PartBucket samples) {
            for (Particle const &ptl : samples) {
                Vector const vel = geomtr.cart2fac(ptl.vel);
                println(os, vel.x, ", ", vel.y, ", ", vel.z, ", ", ptl.pos_x, ", ", ptl.w);
            }
        });
    }
    tk.wait();
}
