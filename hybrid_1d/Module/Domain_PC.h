//
//  Domain_PC.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Domain_PC_h
#define Domain_PC_h

#include "./Domain.h"

HYBRID1D_BEGIN_NAMESPACE
/// predictor-corrector method
///
class Domain_PC : public Domain {
    // workspaces
    //
    Species predictor{};
    BField bfield_1{};
    EField efield_1{};
    bool is_recurring_pass{};

public:
    explicit Domain_PC(Delegate *delegate) : Domain(delegate) {}

private:
    void advance_by(unsigned const n_steps) override;
    inline void cycle(Domain const &domain);
    void predictor_step(Domain const &domain);
    void corrector_step(Domain const &domain);
};
HYBRID1D_END_NAMESPACE

#endif /* Domain_PC_h */
