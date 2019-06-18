//
//  MasterDelegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef MasterDelegate_h
#define MasterDelegate_h

#include "WorkerDelegate.h"

#include <array>

HYBRID1D_BEGIN_NAMESPACE
class MasterDelegate : public Delegate {
    std::vector<decltype(std::declval<WorkerDelegate>().constant_comm)::Ticket> tickets{};
public:
    std::array<WorkerDelegate, Input::number_of_worker_threads> workers{};
    std::unique_ptr<Delegate> const delegate; // serial version

    ~MasterDelegate();
    MasterDelegate(std::unique_ptr<Delegate> delegate) noexcept;

private:
#if defined(HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS) && HYBRID1D_MULTI_THREAD_FUNNEL_BOUNDARY_PASS
    void pass(Domain const&, Species &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Charge &) override;
    void pass(Domain const&, Current &) override;
#endif
    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;

    template <long i, class T>
    void broadcast_to_workers(std::integral_constant<long, i> tag, GridQ<T> const &payload);
    template <long i, class T>
    void collect_from_workers(std::integral_constant<long, i> tag, GridQ<T> &buffer);
};
HYBRID1D_END_NAMESPACE

#endif /* MasterDelegate_h */
