//
//  EnergyRecorder.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "EnergyRecorder.h"
#include "../Utility/println.h"
#include "../InputWrapper.h"

std::string P1D::EnergyRecorder::filepath() const
{
    constexpr char filename[] = "energy.csv";
    return is_master() ? std::string{Input::working_directory} + "/" + filename : null_dev;
}

P1D::EnergyRecorder::EnergyRecorder(unsigned const rank, unsigned const size)
: Recorder{Input::energy_recording_frequency, rank, size} {
    // open output stream
    //
    {
        os.open(filepath(), os.trunc);
        os.setf(os.scientific);
        os.precision(15);
    }

    // header lines
    //
    print(os, "step"); // integral step count
    print(os, ", time"); // simulation time
    //
    print(os, ", dB1^2/2, dB2^2/2, dB3^2/2"); // spatial average of fluctuating (without background) magnetic field energy density
    print(os, ", dE1^2/2, dE2^2/2, dE3^2/2"); // spatial average of fluctuating (without background) electric field energy density
    //
    for (unsigned i = 1; i <= ParamSet::part_indices::size(); ++i) {
        // spatial average of i'th species kinetic energy density
        print(os, ", part_species(", i, ") mv1^2/2", ", part_species(", i, ") mv2^2/2", ", part_species(", i, ") mv3^2/2");
        // spatial average of i'th species bulk flow energy density
        print(os, ", part_species(", i, ") mU1^2/2", ", part_species(", i, ") mU2^2/2", ", part_species(", i, ") mU3^2/2");
    }
    //
    for (unsigned i = 1; i <= ParamSet::cold_indices::size(); ++i) {
        // spatial average of i'th species kinetic energy density
        print(os, ", cold_species(", i, ") mv1^2/2", ", cold_species(", i, ") mv2^2/2", ", cold_species(", i, ") mv3^2/2");
        // spatial average of i'th species bulk flow energy density
        print(os, ", cold_species(", i, ") mU1^2/2", ", cold_species(", i, ") mU2^2/2", ", cold_species(", i, ") mU3^2/2");
    }
    //
    os << std::endl;
}

void P1D::EnergyRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;
    //
    print(os, step_count, ", ", step_count*domain.params.dt);
    //
    auto printer = [&os = this->os](Vector const &v) {
        print(os, ", ", v.x, ", ", v.y, ", ", v.z);
    };
    //
    printer(reduce(dump(domain.bfield), std::plus{}));
    printer(reduce(dump(domain.efield), std::plus{}));
    //
    for (Species const &sp : domain.part_species) {
        Tensor const t = reduce(dump(sp), std::plus{});
        printer(t.lo()); // kinetic
        printer(t.hi()); // bulk flow
    }
    //
    for (Species const &sp : domain.cold_species) {
        Tensor const t = reduce(dump(sp), std::plus{});
        printer(t.lo()); // kinetic
        printer(t.hi()); // bulk flow
    }
    //
    os << std::endl;
}

P1D::Vector P1D::EnergyRecorder::dump(BField const &bfield) noexcept
{
    Vector dB2O2{};
    for (Vector const &B : bfield) {
        Vector const dB = bfield.geomtr.cart2fac(B - bfield.geomtr.B0);
        dB2O2 += dB*dB;
    }
    dB2O2 /= 2*Input::Nx;
    return dB2O2;
}
P1D::Vector P1D::EnergyRecorder::dump(EField const &efield) noexcept
{
    Vector dE2O2{};
    for (Vector const &_E : efield) {
        Vector const dE = efield.geomtr.cart2fac(_E);
        dE2O2 += dE*dE;
    }
    dE2O2 /= 2*Input::Nx;
    return dE2O2;
}
P1D::Tensor P1D::EnergyRecorder::dump(Species const &sp) noexcept
{
    Tensor KE{};
    Vector &mv2O2 = KE.lo(), &mU2O2 = KE.hi();
    for (long i = 0; i < sp.moment<0>().size(); ++i) {
        Real const n{sp.moment<0>()[i]};
        Vector const nV = sp.geomtr.cart2fac(sp.moment<1>()[i]);
        Vector const nvv = sp.geomtr.cart2fac(sp.moment<2>()[i]);
        constexpr Real zero = 1e-15;
        mU2O2 += nV*nV/(n > zero ? n : 1);
        mv2O2 += nvv;
    }
    KE *= sp.energy_density_conversion_factor()/(2*Input::Nx);
    return KE;
}
