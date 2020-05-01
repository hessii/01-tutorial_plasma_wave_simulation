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
        ScalarGrid const*, VectorGrid const*, TensorGrid const*
    >;
    using interthread_comm_t = message_dispatch_t::Communicator;
    //
    MasterDelegate *master{};
    interthread_comm_t comm{};

private:
    void once(Domain &) const override;
    void prologue(Domain const&, long const) const override;
    void epilogue(Domain const&, long const) const override;
    void pass(Domain const&, PartSpecies &) const override;
    void pass(Domain const&, BField &) const override;
    void pass(Domain const&, EField &) const override;
    void pass(Domain const&, Current &) const override;
    void gather(Domain const&, Current &) const override;
    void gather(Domain const&, PartSpecies &) const override;

private: // helpers
    template <class T, long N>
    void recv_from_master(GridQ<T, N> &buffer) const;
    template <class T, long N>
    void reduce_to_master(GridQ<T, N> const &payload) const;

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
