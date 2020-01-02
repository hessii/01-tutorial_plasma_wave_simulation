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
#include <memory>

PIC1D_BEGIN_NAMESPACE
class MasterDelegate final : public Delegate {
    std::vector<decltype(std::declval<WorkerDelegate>().constant_comm)::Ticket> tickets{};
public:
    std::array<WorkerDelegate, ParamSet::number_of_particle_parallism - 1> workers{};
    Delegate *const delegate; // serial version

    ~MasterDelegate();
    MasterDelegate(Delegate *const delegate) noexcept;

private:
    void once(Domain &) override;
#if defined(PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && PIC1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
    void pass(Domain const&, PartSpecies &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Current &) override;
#endif
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, PartSpecies &) override;

private: // helpers
    template <class T, long N>
    void broadcast_to_workers(GridQ<T, N> const &payload);
    template <class T, long N>
    void collect_from_workers(GridQ<T, N> &buffer);
};
PIC1D_END_NAMESPACE

#endif /* MasterDelegate_h */
