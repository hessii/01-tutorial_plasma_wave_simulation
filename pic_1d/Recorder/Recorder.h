//
//  Recorder.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Recorder_h
#define Recorder_h

#include "../Module/Domain.h"
#include "../Predefined.h"
#include "../Macros.h"

PIC1D_BEGIN_NAMESPACE
class Domain;

class Recorder {
public:
    virtual ~Recorder() = default;
    virtual void record(Domain const &domain, long const step_count) = 0;

protected:
    long const recording_frequency;
    explicit Recorder(unsigned const recording_frequency) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* Recorder_h */
