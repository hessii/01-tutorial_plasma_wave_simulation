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
#include "../Utility/GridQ.h"
#include "../Utility/Scalar.h"
#include "../Utility/Vector.h"
#include "../Utility/Tensor.h"

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
    explicit MasterDelegate(); // set all flags at init

    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;

public:
    std::array<std::unique_ptr<WorkerDelegate>, Input::n_workers> workers;

public:
    using MomentTriplet = std::tuple<GridQ<Scalar>, GridQ<Vector>, GridQ<Tensor>>;

    std::tuple<
        std::array<std::pair<std::atomic_flag, GridQ<Scalar>>, Input::n_workers>,
        std::array<std::pair<std::atomic_flag, GridQ<Vector>>, Input::n_workers>,
        std::array<std::pair<std::atomic_flag, MomentTriplet>, Input::n_workers>
    > provider;
};
HYBRID1D_END_NAMESPACE

#endif /* MasterDelegate_h */
