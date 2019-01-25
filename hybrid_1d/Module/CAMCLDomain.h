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
    bool is_recurring_pass{};

public:
    explicit CAMCLDomain();

private:
    void advance_by(unsigned const n_steps) override;
    inline void cycle(Domain const &domain);
};
HYBRID1D_END_NAMESPACE

#endif /* CAMCLDomain_h */
