//
//  WorkerDelegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/13/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef WorkerDelegate_h
#define WorkerDelegate_h

#include "../Module/Delegate.h"

HYBRID1D_BEGIN_NAMESPACE
class WorkerDelegate : public Delegate {
public:
    explicit WorkerDelegate();

    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;
};
HYBRID1D_END_NAMESPACE

#endif /* WorkerDelegate_h */
