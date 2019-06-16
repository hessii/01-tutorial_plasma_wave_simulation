//
//  MasterWrapper.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef MasterWrapper_h
#define MasterWrapper_h

#include "WorkerWrapper.h"
#include "../InputWrapper.h"

#include <array>
#include <vector>

HYBRID1D_BEGIN_NAMESPACE
class MasterWrapper : public Delegate {
    using Ticket = InterThreadComm<MasterWrapper, WorkerWrapper, WorkerWrapper::NChs::value>::Ticket;
    std::vector<Ticket> tickets{};
public:
    std::array<WorkerWrapper, Input::n_workers> workers{};
    std::unique_ptr<Delegate> const delegate; // serial version

    ~MasterWrapper();
    MasterWrapper(std::unique_ptr<Delegate> delegate) noexcept;

private:
#if defined(HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS) && HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS
    void pass(Domain const&, Species &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Charge &) override;
    void pass(Domain const&, Current &) override;
#endif
    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;

    template <long i, class Payload>
    void broadcast_to_workers(std::integral_constant<long, i> tag, Payload const &payload);
    template <long i, class Payload>
    void collect_from_workers(std::integral_constant<long, i> tag, Payload &buffer);
};
HYBRID1D_END_NAMESPACE

#endif /* MasterWrapper_h */
