//
//  MasterDelegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef MasterDelegate_h
#define MasterDelegate_h

#include "../Module/Delegate.h"

#include <array>
#include <atomic>
#include <tuple>
#include <utility>
#include <memory>

HYBRID1D_BEGIN_NAMESPACE
class WorkerDelegate;

class MasterDelegate : public Delegate {
    /// operation type tags
    ///
    enum Tag : long {
        gather_charge = 0,
        gather_current = 1,
        gather_species = 2
    };

public:
    ~MasterDelegate();
    explicit MasterDelegate() noexcept; // set all flags at init

    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;

public:
    std::array<std::unique_ptr<WorkerDelegate>, Input::n_workers> workers;

public:
    using Flags = std::array<std::atomic_flag, Input::n_workers>;
    std::tuple<
        std::pair<Flags, Charge const*>,
        std::pair<Flags, Current const*>,
        std::pair<Flags, Species const*>
    > provider;
};
HYBRID1D_END_NAMESPACE

#endif /* MasterDelegate_h */
