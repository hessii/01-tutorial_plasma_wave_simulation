//
//  WorkerWrapper.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 6/15/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef WorkerWrapper_h
#define WorkerWrapper_h

#include "InterThreadComm.h"
#include "Delegate.h"

HYBRID1D_BEGIN_NAMESPACE
class WorkerWrapper : public Delegate {
public:
    InterThreadComm comm{};

private:
#if defined(HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS) && HYBRID1D_MULTI_THREAD_DELEGATE_ENABLE_PASS
    void pass(Domain const&, Species &) override;
    void pass(Domain const&, BField &) override;
    void pass(Domain const&, EField &) override;
    void pass(Domain const&, Charge &) override;
    void pass(Domain const&, Current &) override;
#endif
    void gather(Domain const&, Charge &) override;
    void gather(Domain const&, Current &) override;
    void gather(Domain const&, Species &) override;
};
HYBRID1D_END_NAMESPACE

#endif /* WorkerWrapper_h */
