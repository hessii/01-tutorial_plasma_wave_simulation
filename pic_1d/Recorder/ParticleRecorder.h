//
//  ParticleRecorder.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef ParticleRecorder_h
#define ParticleRecorder_h

#include "./Recorder.h"

#include <fstream>
#include <string>
#include <random>

PIC1D_BEGIN_NAMESPACE
/// marker particle recorder
/// field-aligned components are recorded;
/// suffix 1, 2, and 3 means 3 field-aligned components:
///     1 : parallel, 2 : perpendicular, and 3 : out-of-plane
///
class ParticleRecorder : public Recorder {
    std::mt19937 urbg;
    std::ofstream os;

    std::string filepath(long const step_count, unsigned const sp_id) const;
public:
    explicit ParticleRecorder(unsigned const rank, unsigned const size);

private:
    void record(Domain const &domain, long const step_count) override;
    void record(PartSpecies const &sp, unsigned const max_count);
};
PIC1D_END_NAMESPACE

#endif /* ParticleRecorder_h */
