//
//  Recorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Recorder_h
#define Recorder_h

#include "../Module/Domain.h"
#include "../Utility/Vector.h"
#include "../Predefined.h"
#include "../Macros.h"

HYBRID1D_BEGIN_NAMESPACE
class Domain;

class Recorder {
public:
    virtual ~Recorder() = default;
    virtual void record(Domain const &domain, long const step_count) = 0;

protected:
    long const recording_period;
    explicit Recorder(unsigned const recording_period) noexcept;

    // field-aligned unit vectors
    //
    static Vector const e1;
    static Vector const e2;
    static Vector const e3;
};
HYBRID1D_END_NAMESPACE

#endif /* Recorder_h */
