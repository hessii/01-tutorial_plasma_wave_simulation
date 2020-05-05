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
#include <memory>

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
    save = is_master() ? &Snapshot::save_master : &Snapshot::save_worker;
    load = is_master() ? &Snapshot::load_master : &Snapshot::load_worker;

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
    template <class T, long N>
    std::vector<T> pack(P1D::GridQ<T, N> const &payload) {
        return {payload.begin(), payload.end()};
    }
    auto pack(P1D::PartSpecies const &sp) {
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
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) write(std::ostream &os, std::deque<T> const &payload) {
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
    auto save = [this, wd = domain.params.working_directory](auto const &payload, std::string_view const basename) {
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

    // B & E
    save(domain.bfield, "bfield");
    save(domain.efield, "efield");

    // particles
    for (unsigned i = 0; i < domain.part_species.size(); ++i) {
        PartSpecies const &sp = domain.part_species.at(i);
        std::string const prefix = std::string{"part_species_"} + std::to_string(i);
        save(sp, prefix + "-particles");
    }

    // cold fluid
    for (unsigned i = 0; i < domain.cold_species.size(); ++i) {
        ColdSpecies const &sp = domain.cold_species.at(i);
        std::string const prefix = std::string{"cold_species_"} + std::to_string(i);
        save(sp.moment<0>(), prefix + "-moment_0");
        save(sp.moment<1>(), prefix + "-moment_1");
        save(sp.moment<2>(), prefix + "-moment_2");
    }
}
void P1D::Snapshot::save_worker(Domain const &domain) const& // just wait because not a performace critical section
{
    // B & E
    comm.send(pack(domain.bfield), master).wait();
    comm.send(pack(domain.efield), master).wait();

    // particles
    for (PartSpecies const &sp : domain.part_species) {
        comm.send(pack(sp), master).wait(); // potentially memory exhaustion if not wait
    }

    // cold fluid
    for (ColdSpecies const &sp : domain.cold_species) {
        comm.send(pack(sp.moment<0>()), master).wait();
        comm.send(pack(sp.moment<1>()), master).wait();
        comm.send(pack(sp.moment<2>()), master).wait();
    }
}
//
// MARK:- Load
//
namespace {
    template <class T, long N>
    void unpack(std::vector<T> payload, P1D::GridQ<T, N> &to) noexcept(std::is_nothrow_move_assignable_v<T>) {
        std::move(begin(payload), end(payload), to.begin());
    }
    template <class T>
    void unpack(std::deque<T> const *payload, P1D::PartSpecies &sp) {
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
    template <class T, std::enable_if_t<std::is_trivially_copyable_v<T>, long> = 0L>
    decltype(auto) read(std::istream &is, std::deque<T> &payload) {
        payload.clear();
        long const cur_pos = is.tellg();
        long const end_pos = is.seekg(0, is.end).tellg();
        if (cur_pos == -1 || end_pos == -1 || (end_pos - cur_pos) % static_cast<long>(sizeof(T)) != 0) {
            is.clear(is.badbit);
        } else {
            is.seekg(cur_pos);
        }
        for (long remaining = (end_pos - cur_pos)/static_cast<long>(sizeof(T));
             is && remaining > 0; --remaining) {
            read(is, payload.emplace_back());
        }
        return is;
    }
}
long P1D::Snapshot::load_master(Domain &domain) const&
{
    long step_count;
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
                decltype(pack(to)) &payload = payloads.emplace_back(to.size());
                if (!read(is, payload)) {
                    throw std::runtime_error{path + " - reading payload failed : " + std::string{basename}};
                }
            }
            if (char dummy; !read(is, dummy).eof()) {
                throw std::runtime_error{path + " - payload not fully read"};
            }
            auto tks = comm.scatter(std::move(payloads), all_ranks);
            unpack(*comm.recv<decltype(pack(to))>(master), to);
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
            decltype(pack(sp)) payload;
            if (!read(is, payload)) {
                throw std::runtime_error{path + " - reading particles failed"};
            } else if (char dummy; !read(is, dummy).eof()) {
                throw std::runtime_error{path + " - payload not fully read"};
            }
            //
            auto tks = comm.bcast<3>(&payload, all_ranks);
            unpack(*comm.recv<3>(master), sp);
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
    }

    // cold fluid
    for (unsigned i = 0; i < domain.cold_species.size(); ++i) {
        ColdSpecies &sp = domain.cold_species.at(i);
        std::string const prefix = std::string{"cold_species_"} + std::to_string(i);
        load_grid(sp.moment<0>(), prefix + "-moment_0");
        load_grid(sp.moment<1>(), prefix + "-moment_1");
        load_grid(sp.moment<2>(), prefix + "-moment_2");
    }

    // step count
    auto tks = comm.bcast(step_count, all_ranks);
    return comm.recv<long>(master);
    //std::for_each(std::make_move_iterator(begin(tks)), std::make_move_iterator(end(tks)), std::mem_fn(&ticket_t::wait));
}
long P1D::Snapshot::load_worker(Domain &domain) const&
{
    // B & E
    unpack(*comm.recv<1>(master), domain.bfield);
    unpack(*comm.recv<1>(master), domain.efield);

    // particles
    for (PartSpecies &sp : domain.part_species) {
        unpack(*comm.recv<3>(master), sp);
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
