//
//  Delegate.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/18/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Delegate_h
#define Delegate_h

#include "../Inputs.h"

HYBRID1D_BEGIN_NAMESPACE
class Domain;
class Species;
class BField;
class EField;
class Charge;
class Current;

class Delegate {
public:
    virtual ~Delegate() = default;

    // communication
    //
    virtual void pass(Domain const&, Species &) = 0;
    virtual void pass(Domain const&, BField &) = 0;
    virtual void pass(Domain const&, EField &) = 0;
    virtual void pass(Domain const&, Charge &) = 0;
    virtual void pass(Domain const&, Current &) = 0;
    virtual void gather(Domain const&, Charge &) = 0;
    virtual void gather(Domain const&, Current &) = 0;
    virtual void gather(Domain const&, Species &) = 0;
};
HYBRID1D_END_NAMESPACE

#endif /* Delegate_h */
