//
//  LossconeVDF.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/23/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "LossconeVDF.h"

#include <stdexcept>
#include <utility>
#include <cmath>

auto P1D::VDF::make(const LossconePlasmaDesc &desc) -> std::unique_ptr<VDF>
{
    return std::make_unique<LossconeVDF>(desc);
}

P1D::LossconeVDF::LossconeVDF(LossconePlasmaDesc const &desc)
: VDF{} { // parameter check is assumed to be done already
    Real const Delta = desc.Delta;
    Real const beta = [beta = desc.beta]() noexcept { // avoid beta == 1
        constexpr Real eps = 1e-5;
        Real const diff = beta - 1;
        return beta + (std::abs(diff) < eps ? std::copysign(eps, diff) : 0);
    }();
    rs = RejectionSampler{Delta, beta};
    vth1 = std::sqrt(desc.beta1)*Input::c * std::abs(desc.Oc)/desc.op;
    T2OT1 = desc.T2_T1;
    xd = desc.Vd/vth1;
    //
    xth2_squared = T2OT1/(1 + (1 - Delta)*beta);
    vth1_cubed = vth1*vth1*vth1;
}

auto P1D::LossconeVDF::f0(Vector const &v) const noexcept
-> Real {
    // note that vel = {v1, v2, v3}/vth1
    //
    // f0(x1, x2, x3) = exp(-(x1 - xd)^2)/√π *
    // ((1 - Δ*β)*exp(-(x2^2 + x3^2)/xth2^2) - (1 - Δ)*exp(-(x2^2 + x3^2)/(β*xth2^2)))
    // -------------------------------------------------------------------------------
    //                           (π * xth2^2 * (1 - β))
    //
    using std::exp;
    Real const f1 = [t = v.x - xd]() noexcept {
        return exp(-t*t)*M_2_SQRTPI*.5;
    }();
    Real const f2 = [ D = rs.Delta, b = rs.beta,
                     t2 = (v.y*v.y + v.z*v.z)/xth2_squared,
                     de = M_PI*xth2_squared*(1 - rs.beta)]() noexcept {
        return ((1 - D*b)*exp(-t2) - (1 - D)*exp(-t2/b))/de;
    }();
    return f1*f2;
}

auto P1D::LossconeVDF::variate() const
-> Particle {
    Particle ptl = load();

    // rescaling
    //
    ptl.vel *= vth1;
    ptl.pos_x *= Input::Nx; // [0, Nx)

    // delta-f parameters
    //
    ptl.f = f0(ptl);
    //ptl.fOg = ptl.f/ptl.g0(ptl);
    static_assert(Particle::fOg == 1.0, "f and g should be identical");

    return ptl;
}
auto P1D::LossconeVDF::load() const
-> Particle {
    // position
    //
    Real const pos_x = bit_reversed<2>(); // [0, 1]

    // velocity in field-aligned frame (Hu et al., 2010, doi:10.1029/2009JA015158)
    //
    Real const phi1 = bit_reversed<3>()*2*M_PI; // [0, 2pi]
    Real const v1 = std::sqrt(-std::log(uniform_real<100>()))*std::sin(phi1); // v_para
    //
    auto const [v2, v3] = [v2 = rs.sample()*std::sqrt(xth2_squared)]() noexcept {
        Real const phi2 = bit_reversed<5>()*2*M_PI; // [0, 2pi]
        // {in-plane v_perp, out-of-plane v_perp}
        return std::make_pair(std::cos(phi2)*v2, std::sin(phi2)*v2);
    }();

    // velocity in Cartesian frame
    //
    Vector const vel = geomtr.fac2cart({v1 + xd, v2, v3});

    return Particle{vel, pos_x};
}

// MARK: - RejectionSampler
//
P1D::LossconeVDF::RejectionSampler::RejectionSampler(Real const Delta, Real const beta/*must not be 1*/)
: Delta{Delta}, beta{beta} {
    constexpr Real eps = 1e-5;
    if (std::abs(1 - Delta) < eps) { // Δ == 1
        alpha = 1;
        M = 1;
    } else { // Δ != 1
        alpha = (beta < 1 ? 1 : beta) + aoffset;
        auto const eval_xpk = [D = Delta, b = beta, a = alpha]{
            Real const det = -b/(1 - b) * std::log(((a - 1)*(1 - D*b)*b)/((a - b)*(1 - D)));
            return std::isfinite(det) && det > 0 ? std::sqrt(det) : 0;
        };
        Real const xpk = std::abs(1 - Delta*beta) < eps ? 0 : eval_xpk();
        M = fOg(xpk);
    }
    if (!std::isfinite(M)) {
        throw std::runtime_error{__FUNCTION__};
    }
}
auto P1D::LossconeVDF::RejectionSampler::fOg(const Real x) const noexcept
-> Real {
    using std::exp;
    Real const x2 = x*x;
    Real const f = ((1 - Delta*beta)*exp(-x2) - (1 - Delta)*exp(-x2/beta))/(1 - beta);
    Real const g = exp(-x2/alpha)/alpha;
    return f/g; // ratio of the target distribution to proposed distribution
}
auto P1D::LossconeVDF::RejectionSampler::sample() const noexcept
-> Real {
    auto const vote = [this](Real const proposal) noexcept {
        Real const jury = VDF::uniform_real<300>()*M;
        return jury <= fOg(proposal);
    };
    auto const proposed = [a = this->alpha]() noexcept {
        return std::sqrt(-std::log(uniform_real<200>())*a);
    };
    //
    Real sample;
    while (!vote(sample = proposed()) );
    return sample;
}

// MARK:- Test
//
#if defined(DEBUG)
#include "../Utility/println.h"
#include <tuple>
#include <vector>
#include <random>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace {
template <unsigned N> [[nodiscard]] auto sample_dist(P1D::VDF const &vdf) {
    std::vector<P1D::Vector> samples(N);
    std::generate(begin(samples), end(samples), [&vdf, gm = P1D::Geometry{{}}]{
        return gm.cart2fac(vdf.variate().vel);
    });
    return samples;
}
//
void test_properties() {
    using namespace P1D;
    constexpr Real b1 = 2, T2OT1 = 2, vd = -1;
    constexpr Real D = .5, b = 1;
    constexpr LossconePlasmaDesc lc({{{1, Input::c}, 1, _1st}, b1, T2OT1, vd}, D, b);
    LossconeVDF const vdf(lc);

    // property checks
    //
    if (vdf.rs.Delta != D) {
        println(std::cout, "invalid Delta: ", D, " != ", vdf.rs.Delta);
    }
    if (vdf.rs.beta != b + 1e-5) {
        println(std::cout, "invalid beta: ", b, " != ", vdf.rs.beta);
    }
    if (vdf.vth1 != std::sqrt(b1)) {
        println(std::cout, "invalid vth1: ", std::sqrt(b1), " != ", vdf.vth1);
    }
    if (vdf.T2OT1 != T2OT1) {
        println(std::cout, "invalid T2OT1: ", T2OT1, " != ", vdf.T2OT1);
    }
    if (vdf.xd != vd/std::sqrt(b1)) {
        println(std::cout, "invalid xd: ", vd/std::sqrt(b1), " != ", vdf.xd);
    }
    if (vdf.xth2_squared != T2OT1/(1 + (1 - D)*vdf.rs.beta)) {
        println(std::cout, "invalid xd: ", T2OT1/(1 + (1 - D)*b), " != ", vdf.xth2_squared);
    }
}
void test_distribution(std::ofstream &os, double const D, double const b,
                       double const b1, double const T2OT1, double const vd) {
    using namespace P1D;
    constexpr unsigned n_samples = 50000;
    //    constexpr Real D = 1, b = .5;
    //    constexpr Real b1 = 1, T2OT1 = 1, vd = -1;
    LossconePlasmaDesc const lc({{{1, Input::c}, 1, _1st}, b1, T2OT1, vd}, D, b);
    LossconeVDF const vdf(lc);
    auto const samples = sample_dist<n_samples>(vdf);
    double vx_mean{}, vx_var{}, vy_mean{}, vy_var{}, vz_mean{}, vz_var{};
    for (Vector const &v : samples) {
        vx_mean += v.x/vdf.vth1 - vdf.xd;
        vy_mean += v.y/vdf.vth1/std::sqrt(vdf.T2OT1);
        vz_mean += v.z/vdf.vth1/std::sqrt(vdf.T2OT1);
        vx_var += v.x*v.x/(vdf.vth1*vdf.vth1) - vdf.xd*vdf.xd;
        vy_var += v.y*v.y/(vdf.vth1*vdf.vth1*vdf.T2OT1);
        vz_var += v.z*v.z/(vdf.vth1*vdf.vth1*vdf.T2OT1);
    }
    vx_mean /= size(samples);
    vy_mean /= size(samples);
    vz_mean /= size(samples);
    vx_var  /= size(samples)/2.0;
    vy_var  /= size(samples)/2.0;
    vz_var  /= size(samples)/2.0;
    print(os, '{', "\"\\[CapitalDelta]\" -> ", D, ", \"\\[Beta]\" -> ", b, ", ",
          "\"\\[Theta]1\" -> ", vdf.vth1, ", \"T2OT1\" -> ", vdf.T2OT1, ", \"vd\" -> ", vdf.xd*vdf.vth1, ", ",
          "\"vxmean\" -> ", vx_mean, ", \"vymean\" -> ", vy_mean, ", \"vzmean\" -> ", vz_mean, ", ",
          "\"vxvar\" -> ", vx_var, ", \"vyvar\" -> ", vy_var, ", \"vzvar\" -> ", vz_var, '}');
}
void test_distribution() {
    constexpr unsigned n_tests = 1000;
    std::uniform_real_distribution<> D{0, 1};
    std::uniform_real_distribution<> b{1e-5, 10};
    std::uniform_real_distribution<> b1{.01, 10};
    std::uniform_real_distribution<> T2OT1{.01, 10};
    std::uniform_real_distribution<> vd{-10, 10};
    std::mt19937 g{3494348};
    {
        std::ofstream os{"/Users/kyungguk/Downloads/losscone.m"};
        os.setf(os.fixed);
        os.precision(15);
        println(os, '{');
        test_distribution(os, D(g), b(g), b1(g), T2OT1(g), vd(g));
        for (long i = 1; os && i < n_tests; ++i) {
            println(os, ",");
            test_distribution(os, D(g), b(g), b1(g), T2OT1(g), vd(g));
        }
        println(os, "\n}");
    }
}
}
void P1D::test_LossconeVDF() {
    test_properties();
    test_distribution();
}
#endif
