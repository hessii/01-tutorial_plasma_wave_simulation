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
#include <vector>

PIC1D_BEGIN_NAMESPACE
class MasterDelegate final : public Delegate {
    std::vector<WorkerDelegate::message_dispatch_t::Ticket> tickets{};
public:
    std::array<WorkerDelegate, ParamSet::number_of_particle_parallism - 1> workers{};
    WorkerDelegate::message_dispatch_t dispatch{};
    WorkerDelegate::interthread_comm_t comm{};
    Delegate *const delegate; // serial version

    ~MasterDelegate();
    MasterDelegate(Delegate *const delegate) noexcept;

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
    void broadcast_to_workers(GridQ<T, N> const &payload);
    template <class T, long N>
    void collect_from_workers(GridQ<T, N> &buffer);

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
