//
//  Snapshot.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/27/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Snapshot.h"

#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <memory>

P1D::Snapshot::message_dispatch_t P1D::Snapshot::dispatch;
P1D::Snapshot::Snapshot(unsigned const rank, unsigned const size, ParamSet const &params, long const step_count)
: comm{dispatch.comm(rank)}, size{size}, step_count{step_count}
{
    // method dispatch
    //
    save = is_master() ? &Snapshot::save_master : &Snapshot::save_worker;
    load = is_master() ? &Snapshot::load_master : &Snapshot::load_worker;

    // calculate signature
    //
    (void)params;
}

std::string P1D::Snapshot::filepath(std::string_view const basename) const
{
    std::string const filename = std::string{basename} + ".snapshot";
    return std::string{Input::working_directory} + "/" + filename;
}

namespace {
    template <class T, long N>
    std::vector<T> pack(P1D::GridQ<T, N> const &payload) {
        return {payload.begin(), payload.end()};
    }
    auto pack(P1D::PartSpecies const &sp) {
        auto payload = sp.bucket;
        for (P1D::Particle &ptl : payload) {
            ptl.pos_x += sp.params.domain_extent.min(); // coordinates relative to whole domain
        }
        return payload;
    }
    //
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ofstream &os, T const &payload) {
        return os.write(static_cast<char const*>(static_cast<void const*>(std::addressof(payload))), sizeof(T));
    }
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ofstream &os, std::vector<T> const &payload) {
        return os.write(static_cast<char const*>(static_cast<void const*>(payload.data())), static_cast<long>(payload.size()*sizeof(T)));
    }
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ofstream &os, std::deque<T> const &payload) {
        for (auto const &x : payload) {
            if (!write(os, x)) {
                break;
            }
        }
        return os;
    }
}
void P1D::Snapshot::save_master(Domain const &domain) const&
{
    auto save = [this, pretty_function = __PRETTY_FUNCTION__](auto const &payload, std::string_view basename) {
        if (std::ofstream os{filepath(basename)}; os) {
            if (!write(os, signature)) {
                throw std::runtime_error{pretty_function};
            }
            if (!write(os, step_count)) {
                throw std::runtime_error{pretty_function};
            }
            //
            auto tk = comm.send(pack(payload), master);
            for (unsigned rank = 0; rank < size; ++rank) {
                if (!write(os, *comm.recv<decltype(pack(payload))>(rank))) {
                    throw std::runtime_error{pretty_function};
                }
            }
            std::move(tk).wait();
        } else {
            throw std::runtime_error{pretty_function};
        }
    };

    // B & E
    save(domain.bfield, "bfield");
    save(domain.efield, "efield");

    // particles
    for (unsigned i = 0; i < domain.part_species.size(); ++i) {
        PartSpecies const &sp = domain.part_species.at(i);
        constexpr char prefix[] = "part_species_";
        save(sp, std::string{prefix} + std::to_string(i) + "-particles");
    }

    // cold fluid
    for (unsigned i = 0; i < domain.cold_species.size(); ++i) {
        ColdSpecies const &sp = domain.cold_species.at(i);
        constexpr char prefix[] = "cold_species_";
        save(sp.moment<0>(), std::string{prefix} + std::to_string(i) + "-moment_0");
        save(sp.moment<1>(), std::string{prefix} + std::to_string(i) + "-moment_1");
        save(sp.moment<2>(), std::string{prefix} + std::to_string(i) + "-moment_2");
    }
}
void P1D::Snapshot::save_worker(Domain const &domain) const&
{
    // B & E
    comm.send(pack(domain.bfield), master).wait();
    comm.send(pack(domain.efield), master).wait();

    // particles
    for (PartSpecies const &sp : domain.part_species) {
        comm.send(pack(sp), master).wait();
    }

    // cold fluid
    for (ColdSpecies const &sp : domain.cold_species) {
        comm.send(pack(sp.moment<0>()), master).wait();
        comm.send(pack(sp.moment<1>()), master).wait();
        comm.send(pack(sp.moment<2>()), master).wait();
    }
}

namespace {
    template <class T, long N>
    void unpack(std::vector<T> const &payload, P1D::GridQ<T, N> &to) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::move(begin(payload), end(payload), to.begin());
    }
    template <class T>
    void unpack(std::deque<T> const *payload, P1D::PartSpecies &sp) {
        sp.bucket.clear();
        for (auto const &ptl : *payload) {
            if (sp.params.domain_extent.is_member(ptl.pos_x)) {
                sp.bucket.emplace_back(ptl).pos_x -= sp.params.domain_extent.min(); // coordinates relative to this subdomain
            }
        }
    }
    //
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) read(std::ifstream &is, T &payload) {
        return is.read(static_cast<char*>(static_cast<void*>(std::addressof(payload))), sizeof(T));
    }
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) read(std::ifstream &is, std::vector<T> &payload) {
        return is.read(static_cast<char*>(static_cast<void*>(payload.data())), static_cast<long>(payload.size()*sizeof(T)));
    }
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) read(std::ifstream &is, std::deque<T> &payload) {
        payload.clear();
        while (!is.eof()) {
            if (!read(is, payload.emplace_back())) {
                break;
            }
        }
        return is;
    }
}
long P1D::Snapshot::load_master(Domain &domain) const&
{
    long step_count;
    auto load_grid = [this, &step_count, pretty_function = __PRETTY_FUNCTION__](auto &to, std::string_view basename) {
        long signature;
        //
        if (std::ifstream is{filepath(basename)}; is) {
            if (!read(is, signature)) {
                throw std::runtime_error{pretty_function};
            } else if (this->signature != signature) {
                throw std::runtime_error{"snapshot loading failed - incompatible signature"};
            }
            if (!read(is, step_count)) {
                throw std::runtime_error{pretty_function};
            }
            //
            std::vector<message_dispatch_t::Ticket> tks;
            for (unsigned rank = 0; rank < size; ++rank) {
                decltype(pack(to)) payload(to.size());
                if (!read(is, payload)) {
                    throw std::runtime_error{pretty_function};
                }
                tks.push_back(comm.send(std::move(payload), rank));
            }
            unpack(*comm.recv<decltype(pack(to))>(master), to);
            // assumes tk.wait() is called on destruction
        } else {
            throw std::runtime_error{pretty_function};
        }
    };
    auto load_ptls = [this, &step_count, pretty_function = __PRETTY_FUNCTION__](PartSpecies &sp, std::string_view basename) {
        long signature;
        //
        if (std::ifstream is{filepath(basename)}; is) {
            if (!read(is, signature)) {
                throw std::runtime_error{pretty_function};
            } else if (this->signature != signature) {
                throw std::runtime_error{"snapshot loading failed - incompatible signature"};
            }
            if (!read(is, step_count)) {
                throw std::runtime_error{pretty_function};
            }
            decltype(pack(sp)) payload;
            if (!read(is, payload)) {
                throw std::runtime_error{pretty_function};
            }
            //
            std::vector<message_dispatch_t::Ticket> tks;
            for (unsigned rank = 0; rank < size; ++rank) {
                tks.push_back(comm.send<4>(&payload, rank));
            }
            unpack(*comm.recv<4>(master), sp);
            // assumes tk.wait() is called on destruction
        } else {
            throw std::runtime_error{pretty_function};
        }
    };

    // B & E
    load_grid(domain.bfield, "bfield");
    load_grid(domain.efield, "efield");

    // particles
    for (unsigned i = 0; i < domain.part_species.size(); ++i) {
        PartSpecies &sp = domain.part_species.at(i);
        constexpr char prefix[] = "part_species_";
        load_ptls(sp, std::string{prefix} + std::to_string(i) + "-particles");
    }

    // cold fluid
    for (unsigned i = 0; i < domain.cold_species.size(); ++i) {
        ColdSpecies &sp = domain.cold_species.at(i);
        constexpr char prefix[] = "cold_species_";
        load_grid(sp.moment<0>(), std::string{prefix} + std::to_string(i) + "-moment_0");
        load_grid(sp.moment<1>(), std::string{prefix} + std::to_string(i) + "-moment_1");
        load_grid(sp.moment<2>(), std::string{prefix} + std::to_string(i) + "-moment_2");
    }

    // step count
    return step_count;
}
long P1D::Snapshot::load_worker(Domain &domain) const&
{
    // B & E
    unpack(*comm.recv<1>(master), domain.bfield);
    unpack(*comm.recv<1>(master), domain.efield);

    // particles
    for (PartSpecies &sp : domain.part_species) {
        unpack(*comm.recv<4>(master), sp);
    }

    // cold fluid
    for (ColdSpecies &sp : domain.cold_species) {
        unpack(*comm.recv<0>(master), sp.moment<0>());
        unpack(*comm.recv<1>(master), sp.moment<1>());
        unpack(*comm.recv<2>(master), sp.moment<2>());
    }

    // step count
    return comm.recv<long>(master);
}
