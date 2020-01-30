//
//  WorkerDelegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef WorkerDelegate_h
#define WorkerDelegate_h

#include "Delegate.h"
#include "../Utility/MessageDispatch.h"

#include <utility>
#include <functional>

PIC1D_BEGIN_NAMESPACE
class MasterDelegate;

class WorkerDelegate final : public Delegate {
public:
    using message_dispatch_t = MessageDispatch<
        std::pair<PartBucket*, PartBucket*>, PartBucket,
        ScalarGrid      *, VectorGrid      *, TensorGrid      *,
        ScalarGrid const*, VectorGrid const*, TensorGrid const*
    >;
    using interthread_comm_t = message_dispatch_t::Communicator;
    //
    MasterDelegate *master{};
    interthread_comm_t comm{};

private:
    void once(Domain &) override;
    void prologue(Domain const&, long const) override;
    void epilogue(Domain const&, long const) override;
    void pass(Domain const&, PartSpecies &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Current &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, PartSpecies &) override;

private: // helpers
    template <class T, long N>
    void recv_from_master(GridQ<T, N> &buffer);
    template <class T, long N>
    void reduce_to_master(GridQ<T, N> &payload);
    template <class T, long N>
    void reduce_divide_and_conquer(GridQ<T, N> &payload);
    template <class T, long N>
    void accumulate_by_worker(GridQ<T, N> const &payload);

public: // wrap the loop with setup/teardown logic included
    template <class F, class... Args>
    [[nodiscard]] auto wrap_loop(F&& f, Args&&... args) {
        return [this, f, args...](Domain *domain) mutable { // intentional capture by copy
            setup(*domain);
            std::invoke(std::move(f), std::move(args)...); // hence move is used
            teardown(*domain);
        };
    }
private:
    void setup(Domain &);
    void teardown(Domain &);
};
PIC1D_END_NAMESPACE

#endif /* WorkerDelegate_h */
