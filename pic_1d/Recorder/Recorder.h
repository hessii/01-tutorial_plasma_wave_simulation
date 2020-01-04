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
#include "../Predefined.h"
#include "../Macros.h"

PIC1D_BEGIN_NAMESPACE
class Domain;

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

    static message_dispatch_t dispatch;
    interthread_comm_t const comm;
    unsigned const size;
    static constexpr unsigned master = 0;
    bool is_master() const noexcept { return master == comm.rank(); }

protected:
    long const recording_frequency;
    explicit Recorder(unsigned const recording_frequency, unsigned const rank, unsigned const size) noexcept;

    template <class T, class Op>
    T reduce(T x, Op op) {
        if (is_master()) {
            // reduce
            for (unsigned i = 1; i < size; ++i) {
                x = dispatch.recv<T>({-1, -1}).unpack(op, x);
            }
            // broadcase
            for (unsigned i = 1; i < size; ++i) {
                dispatch.send({-2, -2}, x).wait();
            }
            return x;
        } else {
            dispatch.send({-1, -1}, x).wait();
            return dispatch.recv<T>({-2, -2});
        }
    }
};
PIC1D_END_NAMESPACE

#endif /* Recorder_h */
