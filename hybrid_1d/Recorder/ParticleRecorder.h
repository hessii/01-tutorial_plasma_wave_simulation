//
//  ParticleRecorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef ParticleRecorder_h
#define ParticleRecorder_h

#include "./Recorder.h"

#include <fstream>
#include <string>

HYBRID1D_BEGIN_NAMESPACE
class ParticleRecorder : public Recorder {
    std::ofstream os;

public:
    explicit ParticleRecorder();

private:
    void record(Domain const &domain, long const step_count) override;

    std::string filename(long const step_count) const;
};
HYBRID1D_END_NAMESPACE

#endif /* ParticleRecorder_h */
