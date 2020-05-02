//
//  VHistogramRecorder.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 4/15/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "VHistogramRecorder.h"
#include "../Utility/println.h"
#include "../InputWrapper.h"

#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <limits>
#include <cmath>

std::string P1D::VHistogramRecorder::filepath(std::string const &wd, long const step_count, unsigned const sp_id) const
{
    constexpr char prefix[] = "vhist2d";
    std::string const filename = std::string{prefix} + "-sp_" + std::to_string(sp_id) + "-" + std::to_string(step_count) + ".csv";
    return is_master() ? wd + "/" + filename : null_dev;
}

P1D::VHistogramRecorder::VHistogramRecorder(unsigned const rank, unsigned const size)
: Recorder{Input::vhistogram_recording_frequency, rank, size} {
    // configure output stream
    //
    os.setf(os.scientific);
    os.precision(15);
}

class P1D::VHistogramRecorder::Indexer {
    // preconditions:
    // 1. length of span is positive
    // 2. dim is positive
    //
    P1D::Range v1span;
    P1D::Range v2span;
    unsigned v1dim;
    unsigned v2dim;
public:
    constexpr Indexer(P1D::Range const &v1span, unsigned const &v1dim, P1D::Range const &v2span, unsigned const &v2dim) noexcept
    : v1span{v1span}, v2span{v2span}, v1dim{v1dim}, v2dim{v2dim} {
    }
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return (v1dim > 0) & (v2dim > 0);
    }
    //
    using index_pair_t = vhist_t::key_type;
    [[nodiscard]] auto operator()(Real const v1, Real const v2) const noexcept {
        // zero-based indexing
        //
        index_pair_t const idx = {
            static_cast<index_pair_t:: first_type>(std::floor((v1 - v1span.min())*v1dim/v1span.len)),
            static_cast<index_pair_t::second_type>(std::floor((v2 - v2span.min())*v2dim/v2span.len))
        };
        if (within(idx, std::make_pair(0, 0), std::make_pair(v1dim, v2dim),
                   std::make_index_sequence<std::tuple_size_v<index_pair_t>>{})) {
            return idx;
        } else {
            constexpr long max = std::numeric_limits<long>::max();
            return std::make_pair(max, max);
        }
    }
private:
    template <std::size_t... I> [[nodiscard]] static
    bool within(index_pair_t const &idx, index_pair_t const &min, index_pair_t const &max,
                std::index_sequence<I...>) noexcept {
        return (... & (std::get<I>(idx) >= std::get<I>(min)))
        &      (... & (std::get<I>(idx) <  std::get<I>(max)));
    }
};
void P1D::VHistogramRecorder::record(const Domain &domain, const long step_count)
{
    if (step_count%recording_frequency) return;
    //
    for (unsigned s = 0; s < domain.part_species.size(); ++s) {
        auto const [v1span, v1divs] = Input::v1hist_specs.at(s);
        auto const [v2span, v2divs] = Input::v2hist_specs.at(s);
        Indexer const idxer{v1span, v1divs, v2span, v2divs};
        if (!idxer) continue;
        if (v1span.len <= 0 || v2span.len <= 0) {
            throw std::invalid_argument{std::string{__FUNCTION__} + " - invalid vspan extent: " + std::to_string(s) + "th species"};
        }
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
            print(os, "v1lim = ", v1span, "; ");
            print(os, "v2lim = ", v2span, "; ");
            print(os, "vdims = {", v1divs, ", ", v2divs, "}; ");
            print(os, "species = ", s, '\n');
            println(os, R"("{full-f, delta-f} velocity histogram normalized by the number of samples; sparse array format")");

            // contents
            //
            auto const vhist = histogram(domain.part_species[s], idxer);
            for (auto const &kv : vhist) {
                std::pair<long, long> const &idx = kv.first;
                std::pair<Real, Real> const &val = kv.second;
                //
                print(os, '{', idx.first, ", ", idx.second, '}') << " -> ";
                print(os, '{', val.first, ", ", val.second, '}') << '\n';
            }
        }
        os.close();
    }
}

namespace {
template <class T1, class T2, class U1, class U2>
constexpr decltype(auto) operator+=(std::pair<T1, T2> &lhs, std::pair<U1, U2> const &rhs)
noexcept(noexcept(std::declval<T1&>() += std::declval<U1>())) {
    std::get<0>(lhs) += std::get<0>(rhs);
    std::get<1>(lhs) += std::get<1>(rhs);
    return lhs;
}
template <class T, class U> [[nodiscard]]
constexpr auto operator+(std::pair<T, T> a, U const &b)
noexcept(noexcept(std::declval<T&>() += std::declval<U>())) {
    return a += std::make_pair(b, b);
}
template <class T, class U>
constexpr decltype(auto) operator/=(std::pair<T, T> &lhs, U const &rhs)
noexcept(noexcept(std::declval<T&>() /= std::declval<U>())) {
    std::get<0>(lhs) /= rhs;
    std::get<1>(lhs) /= rhs;
    return lhs;
}
}
auto P1D::VHistogramRecorder::histogram(PartSpecies const &sp, Indexer const &idxer) const
-> vhist_t {
    // local counting
    //
    vhist_payload_t local_vhist{};
    for (Particle const &ptl : sp.bucket) {
        auto const &vel = sp.geomtr.cart2fac(ptl.vel);
        local_vhist[idxer(vel.x, std::sqrt(vel.y*vel.y + vel.z*vel.z))
                    ] += std::make_pair(1, ptl.w);
    }

    // global counting
    //
    auto tk = comm.send(std::make_pair(sp.bucket.size(), std::move(local_vhist)), master);
    vhist_t global_vhist = histogram(idxer);
    std::move(tk).wait();
    //
    return global_vhist; // NRVO
}
auto P1D::VHistogramRecorder::histogram([[maybe_unused]] Indexer const &idxer) const
-> vhist_t {
    if (!is_master()) return {};

    // consolidation
    //
    Real denom{};
    vhist_t vhist{}; // one-based index
    for (unsigned rank = 0; rank < size; ++rank) {
        auto const [count, lwhist] = *comm.recv<std::pair<unsigned long, vhist_payload_t>>(rank);
        //
        denom += count;
        std::for_each(std::next(lwhist.rbegin()), lwhist.rend(),
                      [&vhist](auto const &kv) {
            std::pair<long, long> const &key = kv.first;
            std::pair<long, Real> const &val = kv.second;
            vhist[key + 1] += val;
        });
    }

    // normalization
    //
    for (auto &kv : vhist) {
        std::pair<Real, Real> &val = kv.second;
        val /= denom;
    }

    return vhist; // NRVO
}
