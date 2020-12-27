//
//  Snapshot.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/27/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Snapshot_h
#define Snapshot_h

#include "../Utility/MessageDispatch.h"
#include "../Core/Domain.h"

#include <vector>
#include <memory>
#include <string>
#include <string_view>

PIC1D_BEGIN_NAMESPACE
class Snapshot {
    void (Snapshot::*save)(Domain const &domain) const&;
    long (Snapshot::*load)(Domain &domain) const&;
    long const step_count;
    std::size_t const signature;
    std::vector<unsigned> all_ranks;

    std::string filepath(std::string const &wd, std::string_view const basename) const;
public:
    using message_dispatch_t = MessageDispatch<
        std::vector<Scalar>, std::vector<Vector>, std::vector<Tensor>,
        std::shared_ptr<std::vector<Particle> const>, std::vector<Particle>, long
    >;
    using interthread_comm_t = message_dispatch_t::Communicator;
    using ticket_t = message_dispatch_t::Ticket;

    static message_dispatch_t dispatch;
    interthread_comm_t const comm;
    unsigned const size;
    static constexpr unsigned master = 0;
    [[nodiscard]] bool is_master() const noexcept { return master == comm.rank(); }

public:
    explicit Snapshot(unsigned const rank, unsigned const size, ParamSet const &params, long const step_count);

private: // load/save
    void save_master(Domain const &domain) const&;
    void save_worker(Domain const &domain) const&;
    [[nodiscard]] long load_master(Domain &domain) const&;
    [[nodiscard]] long load_worker(Domain &domain) const&;

private: // load/save interface
    friend void operator<<(Snapshot &&snapshot, Domain const &domain) {
        return (snapshot.*snapshot.save)(domain);
    }
    [[nodiscard]] friend long operator>>(Snapshot &&snapshot, Domain &domain) {
        return (snapshot.*snapshot.load)(domain);
    }
};
PIC1D_END_NAMESPACE

#endif /* Snapshot_h */
