//
//  Snapshot.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/27/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Snapshot.h"

#include <type_traits>
#include <functional> // std::hash, std::mem_fn
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <fstream>

namespace {
    template <class Tuple> struct Hash;
    template <class T> Hash(T const &t) -> Hash<T>;
    //
    template <class... Ts> struct Hash<std::tuple<Ts...>> {
        std::tuple<Ts...> const t;
        [[nodiscard]] constexpr operator std::size_t() const noexcept {
            return operator()();
        }
        [[nodiscard]] constexpr std::size_t operator()() const noexcept {
            return hash(std::index_sequence_for<Ts...>{});
        }
    private:
        template <std::size_t... Is>
        [[nodiscard]] constexpr std::size_t hash(std::index_sequence<Is...>) const noexcept {
            std::size_t hash = 0;
            return (..., ((hash <<= 1) ^= this->hash(std::get<Is>(t)))), hash;
        }
        template <class T>
        [[nodiscard]] static constexpr std::size_t hash(T const &x) noexcept {
            return std::hash<T>{}(x);
        }
    };
}

P1D::Snapshot::message_dispatch_t P1D::Snapshot::dispatch{P1D::ParamSet::number_of_subdomains};
P1D::Snapshot::Snapshot(unsigned const rank, unsigned const size, ParamSet const &params, long const step_count)
: comm{dispatch.comm(rank)}, size{size}, step_count{step_count}, signature{Hash{serialize(params)}}, all_ranks{}
{
    if (size > ParamSet::number_of_subdomains) {
        throw std::invalid_argument{__PRETTY_FUNCTION__};
    }

    // method dispatch
    //
    if (is_master()) {
        save = &Snapshot::save_master;
        load = &Snapshot::load_master;
    } else {
        save = &Snapshot::save_worker;
        load = &Snapshot::load_worker;
    }

    // participants
    //
    for (unsigned rank = 0; is_master() && rank < size; ++rank) {
        all_ranks.emplace_back(rank);
    }
}

std::string P1D::Snapshot::filepath(std::string const &wd, std::string_view const basename) const
{
    std::string const filename = std::string{"snapshot"} + "-" + std::string{basename} + ".snapshot";
    return wd + "/" + filename;
}
//
// MARK:- Save
//
namespace {
    template <class T, long N> [[nodiscard]]
    std::vector<T> pack(P1D::GridQ<T, N> const &payload) {
        return {payload.begin(), payload.end()};
    }
    [[nodiscard]] auto pack(P1D::PartSpecies const &sp) {
        return sp.dump_ptls();
    }
    //
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ostream &os, T const &payload) {
        return os.write(static_cast<char const*>(static_cast<void const*>(std::addressof(payload))), sizeof(T));
    }
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ostream &os, std::vector<T> const &payload) {
        return os.write(static_cast<char const*>(static_cast<void const*>(payload.data())), static_cast<long>(payload.size()*sizeof(T)));
    }
}
void P1D::Snapshot::save_master(Domain const &domain) const&
{
    auto save_grid = [this, wd = domain.params.working_directory](auto const &payload, std::string_view const basename) {
        std::string const path = filepath(wd, basename);
        if (std::ofstream os{path}; os) {
            if (!write(os, signature)) {
                throw std::runtime_error{path + " - writing signature failed"};
            }
            if (!write(os, step_count)) {
                throw std::runtime_error{path + " - writing step count failed"};
            }
            //
            auto tk = comm.send(pack(payload), master);
            comm.for_each<decltype(pack(payload))>(all_ranks, [&os, path, basename](auto payload) {
                if (!write(os, std::move(payload))) {
                    throw std::runtime_error{path + " - writing payload failed : " + std::string{basename}};
                }
            });
            std::move(tk).wait();
        } else {
            throw std::runtime_error{path + " - file open failed"};
        }
    };
    auto save_ptls = [this, wd = domain.params.working_directory](PartSpecies const &sp, std::string_view const basename) {
        std::string const path = filepath(wd, basename);
        if (std::ofstream os{path}; os) {
            if (!write(os, signature)) {
                throw std::runtime_error{path + " - writing signature failed"};
            }
            if (!write(os, step_count)) {
                throw std::runtime_error{path + " - writing step count failed"};
            }
            auto payload = pack(sp);
            // particle count
            auto tk1 = comm.send(static_cast<long>(payload.size()), master);
            if (!write(os, comm.reduce<long>(all_ranks, long{}, std::plus{}))) {
                throw std::runtime_error{path + " - writing particle count failed : " + std::string{basename}};
            }
            std::move(tk1).wait();
            // particle dump
            auto tk2 = comm.send(std::move(payload), master);
            comm.for_each<decltype(payload)>(all_ranks, [&os, path, basename](auto payload) {
                if (!write(os, std::move(payload))) {
                    throw std::runtime_error{path + " - writing particles failed : " + std::string{basename}};
                }
            });
            std::move(tk2).wait();
        } else {
            throw std::runtime_error{path + " - file open failed"};
        }
    };

    // B & E
    save_grid(domain.bfield, "bfield");
    save_grid(domain.efield, "efield");

    // particles
    for (unsigned i = 0; i < domain.part_species.size(); ++i) {
        PartSpecies const &sp = domain.part_species.at(i);
        std::string const prefix = std::string{"part_species_"} + std::to_string(i);
        save_ptls(sp, prefix + "-particles");
    }

    // cold fluid
    for (unsigned i = 0; i < domain.cold_species.size(); ++i) {
        ColdSpecies const &sp = domain.cold_species.at(i);
        std::string const prefix = std::string{"cold_species_"} + std::to_string(i);
        save_grid(sp.mom0_full, prefix + "-mom0_full");
        save_grid(sp.mom1_full, prefix + "-mom1_full");
    }
}
void P1D::Snapshot::save_worker(Domain const &domain) const& // just wait because not a performace critical section
{
    // B & E
    comm.send(pack(domain.bfield), master).wait();
    comm.send(pack(domain.efield), master).wait();

    // particles
    for (PartSpecies const &sp : domain.part_species) {
        auto payload = pack(sp);
        comm.send(static_cast<long>(payload.size()), master).wait();
        comm.send(std::move(payload), master).wait(); // potential memory exhaustion if not wait
    }

    // cold fluid
    for (ColdSpecies const &sp : domain.cold_species) {
        comm.send(pack(sp.mom0_full), master).wait();
        comm.send(pack(sp.mom1_full), master).wait();
    }
}
//
// MARK:- Load
//
namespace {
    template <class T, long N>
    void unpack_grid(std::vector<T> payload, P1D::GridQ<T, N> &to) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::move(begin(payload), end(payload), to.begin());
    }
    template <class T>
    void unpack_ptls(std::shared_ptr<T const> payload, P1D::PartSpecies &sp) {
        sp.load_ptls(*payload);
    }
    //
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) read(std::istream &is, T &payload) {
        return is.read(static_cast<char*>(static_cast<void*>(std::addressof(payload))), sizeof(T));
    }
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) read(std::istream &is, std::vector<T> &payload) {
        return is.read(static_cast<char*>(static_cast<void*>(payload.data())), static_cast<long>(payload.size()*sizeof(T)));
    }
}
long P1D::Snapshot::load_master(Domain &domain) const&
{
    long step_count{};
    auto load_grid = [this, wd = domain.params.working_directory, &step_count](auto &to, std::string_view const basename) {
        std::string const path = filepath(wd, basename);
        if (std::ifstream is{path}; is) {
            if (std::size_t signature; !read(is, signature)) {
                throw std::runtime_error{path + " - reading signature failed"};
            } else if (this->signature != signature) {
                throw std::runtime_error{path + " - incompatible signature"};
            }
            if (!read(is, step_count)) {
                throw std::runtime_error{path + " - reading step count failed"};
            }
            //
            std::vector<decltype(pack(to))> payloads;
            payloads.reserve(all_ranks.size());
            for ([[maybe_unused]] unsigned const &rank : all_ranks) {
                if (!read(is, payloads.emplace_back(to.size()))) {
                    throw std::runtime_error{path + " - reading payload failed : " + std::string{basename}};
                }
            }
            if (char dummy; !read(is, dummy).eof()) {
                throw std::runtime_error{path + " - payload not fully read"};
            }
            //
            auto tks = comm.scatter(std::move(payloads), all_ranks);
            unpack_grid(*comm.recv<decltype(pack(to))>(master), to);
            std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)),
                          std::mem_fn(&ticket_t::wait));
        } else {
            throw std::runtime_error{path + " - file open failed"};
        }
    };
    auto load_ptls = [this, wd = domain.params.working_directory, &step_count](PartSpecies &sp, std::string_view const basename) {
        std::string const path = filepath(wd, basename);
        if (std::ifstream is{path}; is) {
            if (std::size_t signature; !read(is, signature)) {
                throw std::runtime_error{path + " - reading signature failed"};
            } else if (this->signature != signature) {
                throw std::runtime_error{path + " - incompatible signature"};
            }
            if (!read(is, step_count)) {
                throw std::runtime_error{path + " - reading step count failed"};
            }
            std::shared_ptr<decltype(pack(sp))> payload;
            // particle count
            if (long size{}; !read(is, size)) {
                throw std::runtime_error{path + " - reading particle count failed"};
            } else if (sp->Nc*sp.params.Nx != size) {
                throw std::runtime_error{path + " - particle count read inconsistent"};
            } else {
                payload = std::make_shared<decltype(pack(sp))>(size);
            }
            // particle load
            if (!payload) {
                throw std::runtime_error{path + " - particle bucket not initialized"};
            } else if (!read(is, *payload)) {
                throw std::runtime_error{path + " - reading particles failed"};
            } else if (char dummy; !read(is, dummy).eof()) {
                throw std::runtime_error{path + " - particles not fully read"};
            }
            // sent payload must be alive until all workers got their particles loaded
            auto tks = comm.bcast<3>(std::move(payload), all_ranks);
            unpack_ptls(*comm.recv<3>(master), sp);
            std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)),
                          std::mem_fn(&ticket_t::wait));
        } else {
            throw std::runtime_error{path + " - file open failed"};
        }
    };

    // B & E
    load_grid(domain.bfield, "bfield");
    load_grid(domain.efield, "efield");

    // particles
    for (unsigned i = 0; i < domain.part_species.size(); ++i) {
        PartSpecies &sp = domain.part_species.at(i);
        std::string const prefix = std::string{"part_species_"} + std::to_string(i);
        load_ptls(sp, prefix + "-particles");
        //
        auto tk = comm.send(static_cast<long>(sp.bucket.size()), master);
        if (sp->Nc*sp.params.Nx != comm.reduce<long>(all_ranks, long{}, std::plus{})) {
            throw std::runtime_error{std::string{__PRETTY_FUNCTION__} + " - particle count inconsistent for species " + std::to_string(i)};
        }
        std::move(tk).wait();
    }

    // cold fluid
    for (unsigned i = 0; i < domain.cold_species.size(); ++i) {
        ColdSpecies &sp = domain.cold_species.at(i);
        std::string const prefix = std::string{"cold_species_"} + std::to_string(i);
        load_grid(sp.mom0_full, prefix + "-mom0_full");
        load_grid(sp.mom1_full, prefix + "-mom1_full");
    }

    // step count
    auto tks = comm.bcast(step_count, all_ranks);
    return comm.recv<long>(master);
    //std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)), std::mem_fn(&ticket_t::wait));
}
long P1D::Snapshot::load_worker(Domain &domain) const&
{
    // B & E
    unpack_grid(*comm.recv<1>(master), domain.bfield);
    unpack_grid(*comm.recv<1>(master), domain.efield);

    // particles
    for (PartSpecies &sp : domain.part_species) {
        // received payload must be alive until all workers got their particles loaded
        unpack_ptls(*comm.recv<3>(master), sp);
        //
        comm.send(static_cast<long>(sp.bucket.size()), master).wait();
    }

    // cold fluid
    for (ColdSpecies &sp : domain.cold_species) {
        unpack_grid(*comm.recv<0>(master), sp.mom0_full);
        unpack_grid(*comm.recv<1>(master), sp.mom1_full);
    }

    // step count
    return comm.recv<long>(master);
}
