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
#include <stdexcept>

std::string P1D::ParticleRecorder::filepath(std::string const &wd, long const step_count, unsigned const sp_id) const
{
    constexpr char prefix[] = "particle";
    std::string const filename = std::string{prefix} + "-sp_" + std::to_string(sp_id) + "-" + std::to_string(step_count) + ".csv";
    return is_master() ? wd + "/" + filename : null_dev;
}

P1D::ParticleRecorder::ParticleRecorder(unsigned const rank, unsigned const size)
: Recorder{Input::particle_recording_frequency, rank, size}, urbg{123 + rank} {
    // configure output stream
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
        std::string const path = filepath(domain.params.working_directory, step_count, s + 1);
        if (os.open(path, os.trunc); !os) {
            throw std::invalid_argument{std::string{__FUNCTION__} + " - open failed: " + path};
        } else {
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
        ptl.pos_x += sp.params.domain_extent.min(); // coordinates relative to the whole simulation domain
    }
    //
    auto printer = [&os = this->os, &geomtr = sp.geomtr](Particle const &ptl)->std::ostream &{
        Vector const vel = geomtr.cart2fac(ptl.vel);
        return println(os, vel.x, ", ", vel.y, ", ", vel.z, ", ", ptl.pos_x, ", ", ptl.w);
    };
    //
    auto tk = comm.send(std::move(samples), master);
    if (is_master()) {
        auto const writer = [printer](PartBucket samples) {
            for (Particle const &ptl : samples) {
                printer(ptl);
            }
        };
        auto all = all_but_master;
        all.insert(master);
        comm.for_each<PartBucket>(all, writer);
    }
    std::move(tk).wait();
}
