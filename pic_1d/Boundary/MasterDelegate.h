//
//  MasterDelegate.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MasterDelegate_h
#define MasterDelegate_h

#include "WorkerDelegate.h"

#include <array>

PIC1D_BEGIN_NAMESPACE
class MasterDelegate final : public Delegate {
    using ticket_t = WorkerDelegate::message_dispatch_t::Ticket;
public:
    std::array<WorkerDelegate, ParamSet::number_of_particle_parallelism - 1> workers{};
    mutable // access of methods in message dispatcher is thread-safe
    WorkerDelegate::message_dispatch_t dispatch{ParamSet::number_of_particle_parallelism}; // each master thread in domain decomposition must have its own message dispatcher
    WorkerDelegate::interthread_comm_t comm{};
    Delegate *const delegate; // serial version
    std::vector<unsigned> all_but_master;

    ~MasterDelegate();
    MasterDelegate(Delegate *const delegate);

private:
    void once(Domain &) const override;
    void prologue(Domain const&, long const) const override;
    void epilogue(Domain const&, long const) const override;
    void pass(Domain const&, PartSpecies &) const override;
    void pass(Domain const&, ColdSpecies &) const override;
    void pass(Domain const&, BField &) const override;
    void pass(Domain const&, EField &) const override;
    void pass(Domain const&, Current &) const override;
    void gather(Domain const&, Current &) const override;
    void gather(Domain const&, PartSpecies &) const override;

private: // helpers
    template <class T, long N>
    void broadcast_to_workers(GridQ<T, N> const &payload) const;
    template <class T, long N>
    void collect_from_workers(GridQ<T, N> &buffer) const;

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

#endif /* MasterDelegate_h */
