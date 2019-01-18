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
    Species predictor;
    BField bfield_1;
    EField efield_1;

public:
    using Domain::Domain;

private:
    void advance_by(unsigned const n_steps) override;
    inline void cycle();
    void predictor_step();
    void corrector_step();
};
HYBRID1D_END_NAMESPACE

#endif /* PCDomain_h */
