//
//  VDFRecorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef VDFRecorder_h
#define VDFRecorder_h

#include "./Recorder.h"

#include <fstream>

HYBRID1D_BEGIN_NAMESPACE
class VDFRecorder : public Recorder {
    std::ofstream os;

public:
    explicit VDFRecorder();
    ~VDFRecorder();

    void record(Domain const &domain, long const step_count) override;
};
HYBRID1D_END_NAMESPACE

#endif /* VDFRecorder_h */
