//
//  Recorder.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Recorder_h
#define Recorder_h

#include "../Utility/MessageDispatch.h"
#include "../Module/Domain.h"

#include <vector>

PIC1D_BEGIN_NAMESPACE
class Recorder {
public:
    static constexpr char null_dev[] = "/dev/null";

    virtual ~Recorder() = default;
    virtual void record(Domain const &domain, long const step_count) = 0;

    using PartBucket = PartSpecies::bucket_type;
    using message_dispatch_t = MessageDispatch<
        Scalar, Vector, Tensor, PartBucket,
        std::pair<Vector const*, Vector const*>,
        std::pair<PartSpecies const*, ColdSpecies const*>
    >;
    using interthread_comm_t = message_dispatch_t::Communicator;
    using ticket_t = message_dispatch_t::Ticket;

    static message_dispatch_t dispatch;
    interthread_comm_t const comm;
    unsigned const size;
    static constexpr unsigned master = 0;
    [[nodiscard]] bool is_master() const noexcept { return master == comm.rank(); }

protected:
    long const recording_frequency;
    explicit Recorder(unsigned const recording_frequency, unsigned const rank, unsigned const size) noexcept;

    template <class T, class Op>
    T reduce(T x, Op op) {
        if (is_master()) {
            // reduce; skip collecting master's value, cuz it is used as initial value
            for (unsigned rank = 0; rank < size; ++rank) {
                if (master != rank) x = comm.recv<T>(rank).unpack(op, x);
            }
            // broadcase
            std::vector<ticket_t> tks(size);
            for (unsigned rank = 0; rank < size; ++rank) {
                tks.at(rank) = comm.send(x, rank);
            }
            return comm.recv<T>(master);
            // assume that tk.wait() is called on destruction
        } else {
            comm.send(x, master).wait();
            return comm.recv<T>(master);
        }
    }
};
PIC1D_END_NAMESPACE

#endif /* Recorder_h */
