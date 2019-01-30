//
//  EnergyRecorder.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#include "EnergyRecorder.h"
#include "../Utility/println.h"
#include "../Inputs.h"

std::string H1D::EnergyRecorder::filepath()
{
    constexpr char filename[] = "energy.m";
    return std::string(Input::working_directory) + "/" + filename;
}

H1D::EnergyRecorder::EnergyRecorder()
: Recorder(Input::energy_recording_frequency) {
    // open output stream
    //
    {
        os.open(filepath(), os.trunc);
        os.setf(os.scientific);
        os.precision(15);
    }

    // insert preambles
    //
    println(os, "step = {}"); // integral step count
    println(os, "time = {}"); // simulation time
    println(os, "dB2O2 = {}"); // dB^2/2; without background B
    println(os, "dE2O2 = {}"); // dE^2/2
    println(os, "iKineticE = {}"); // ion kinetic energy & bulk flow energy; {{{mvx^2/2, mvy^2/2, mvz^2/2, mUx^2/2, mUy^2/2, mUz^2/2}*Ns}}, ...}
    (os << std::endl).flush();
}

void H1D::EnergyRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;

    println(os, "step = step ~ Append ~ ", step_count);
    println(os, "time = time ~ Append ~ ", step_count*Input::dt);

    println(os, "dB2O2 = dB2O2 ~ Append ~ ", dump(domain.bfield));
    println(os, "dE2O2 = dE2O2 ~ Append ~ ", dump(domain.efield));

    print(os, "iKineticE = iKineticE ~ Append ~ {Sequence[]"); // `Sequence[args...]' in mathematica means splicing args into the enclosing function; this is just a lazy way to remove the first comma below
    for (Species const &sp : domain.species) {
        print(os, ",\n", dump(sp));
    }
    println(os, "\n}");

    (os << std::endl).flush();
}

H1D::Vector H1D::EnergyRecorder::dump(BField const &bfield) noexcept
{
    Vector dB2O2{};
    for (Vector const &B : bfield) {
        Vector const dB = fac(B - bfield.B0);
        dB2O2 += dB*dB;
    }
    return dB2O2 /= 2*Input::Nx;
}
H1D::Vector H1D::EnergyRecorder::dump(EField const &efield) noexcept
{
    Vector dE2O2{};
    for (Vector const &_E : efield) {
        Vector const dE = fac(_E);
        dE2O2 += dE*dE;
    }
    return dE2O2 /= 2*Input::Nx;
}
H1D::Tensor H1D::EnergyRecorder::dump(Species const &sp) noexcept
{
    Tensor KE{};
    Vector &mv2O2 = KE.lo(), &mU2O2 = KE.hi();
    for (long i = 0; i < sp.moment<0>().size(); ++i) {
        Real const n{sp.moment<0>()[i]};
        Vector const nV = fac(sp.moment<1>()[i]);
        Vector const nvv = fac(sp.moment<2>()[i]);
        mU2O2 += nV*nV/n;
        mv2O2 += nvv;
    }
    return KE *= sp.energy_density_conversion_factor()/(2*Input::Nx);
}
