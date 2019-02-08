//
//  FieldRecorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef FieldRecorder_h
#define FieldRecorder_h

#include "./Recorder.h"

#include <fstream>
#include <string>

HYBRID1D_BEGIN_NAMESPACE
/// fluctuating (w/o background) electric and magnetic field recorder
/// field-aligned components are recorded;
/// suffix 1, 2, and 3 means 3 field-aligned components:
///     1 : parallel, 2 : perpendicular, and 3 : out-of-plane
///
class FieldRecorder : public Recorder {
    std::ofstream os;

    static std::string filepath(long const step_count);
public:
    explicit FieldRecorder();

private:
    void record(Domain const &domain, long const step_count) override;
};
HYBRID1D_END_NAMESPACE

#endif /* FieldRecorder_h */
