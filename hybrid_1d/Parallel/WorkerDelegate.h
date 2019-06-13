//
//  WorkerDelegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef WorkerDelegate_h
#define WorkerDelegate_h

#include "../Module/Delegate.h"

#include <array>
#include <atomic>
#include <tuple>
#include <utility>

HYBRID1D_BEGIN_NAMESPACE
class MasterDelegate;

class WorkerDelegate : public Delegate {
    /// operation type tags
    ///
    enum Tag : long {
        gather_charge = 0,
        gather_current = 1,
        gather_species = 2
    };

public:
    ~WorkerDelegate();
    explicit WorkerDelegate(MasterDelegate *master, unsigned const id) noexcept; // set flags at init

    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;

private:
    MasterDelegate *master;
    unsigned long id;

public:
    std::tuple<
        std::pair<std::atomic_flag, Charge const*>,
        std::pair<std::atomic_flag, Current const*>,
        std::pair<std::atomic_flag, Species const*>
    > provider;
};
HYBRID1D_END_NAMESPACE

#endif /* WorkerDelegate_h */
