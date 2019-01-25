//
//  CAMCLDomain.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/25/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef CAMCLDomain_h
#define CAMCLDomain_h

#include "./Domain.h"

HYBRID1D_BEGIN_NAMESPACE
/// predictor-corrector method
///
class CAMCLDomain : public Domain {
    // workspaces
    //
    BField bfield_1{};
    Current current_1{};
    Charge charge_1{};
    Lambda lambda{};
    Gamma gamma{};
    bool is_recurring_pass{};

public:
    explicit CAMCLDomain();

private:
    void advance_by(unsigned const n_steps) override;
    void cycle(Domain const &domain);
    inline void subcycle(Domain const &, Charge const &charge, Current const &current, Real const dt);
};
HYBRID1D_END_NAMESPACE

#endif /* CAMCLDomain_h */
