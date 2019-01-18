//
//  PCDomain.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef PCDomain_h
#define PCDomain_h

#include "./Domain.h"

HYBRID1D_BEGIN_NAMESPACE
/// predictor-corrector method
///
class PCDomain : public Domain {
    // workspaces
    //
    Species predictor{};
    BField bfield_1{};
    EField efield_1{};
    bool is_recurring_pass{};

public:
    explicit PCDomain();

private:
    void advance_by(unsigned const n_steps) override;
    inline void cycle(Domain const &domain);
    void predictor_step(Domain const &domain);
    void corrector_step(Domain const &domain);
};
HYBRID1D_END_NAMESPACE

#endif /* PCDomain_h */
