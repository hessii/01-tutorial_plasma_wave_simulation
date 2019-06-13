//
//  MasterDelegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef MasterDelegate_h
#define MasterDelegate_h

#include "./Delegate.h"

HYBRID1D_BEGIN_NAMESPACE
class MasterDelegate : public Delegate {
public:
    explicit MasterDelegate();

    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;
};
HYBRID1D_END_NAMESPACE

#endif /* MasterDelegate_h */
