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
#include "../Module/Domain.h"

#include <vector>

PIC1D_BEGIN_NAMESPACE
class Snapshot {
    void (Snapshot::*save)(Domain const &domain) &;
    long (Snapshot::*load)(Domain &domain) const &;
    long const step_count;
    long signature;

public:
    using PartBucket = PartSpecies::bucket_type;
    using message_dispatch_t = MessageDispatch<std::vector<Scalar>, std::vector<Vector>, std::vector<Tensor>, PartBucket, long>;
    using interthread_comm_t = message_dispatch_t::Communicator;

    static message_dispatch_t dispatch;
    interthread_comm_t const comm;
    unsigned const size;
    static constexpr unsigned master = 0;
    bool is_master() const noexcept { return master == comm.rank(); }

public:
    explicit Snapshot(unsigned const rank, unsigned const size, ParamSet const &params, long const step_count);

private: // load/save
    void save_master(Domain const &domain) &;
    void save_worker(Domain const &domain) &;
    long load_master(Domain &domain) const &;
    long load_worker(Domain &domain) const &;

private: // load/save interface
    friend void operator<<(Snapshot &snapshot, Domain const &domain) {
        return (snapshot.*snapshot.save)(domain);
    }
    friend long operator>>(Snapshot const &snapshot, Domain &domain) {
        return (snapshot.*snapshot.load)(domain);
    }
};
PIC1D_END_NAMESPACE

#endif /* Snapshot_h */
