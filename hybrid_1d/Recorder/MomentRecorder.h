//
//  MomentRecorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef MomentRecorder_h
#define MomentRecorder_h

#include "./Recorder.h"

#include <fstream>
#include <string>

HYBRID1D_BEGIN_NAMESPACE
class MomentRecorder : public Recorder {
    std::ofstream os;
    GridQ<Vector> ws;

    static std::string filepath();
public:
    explicit MomentRecorder();

private:
    void record(Domain const &domain, long const step_count) override;

    GridQ<Scalar> const &dump(GridQ<Scalar> const &mom0) noexcept { return mom0; } // simply passthrough
    template <class T>
    GridQ<Vector> const &dump(GridQ<T> const &mom12) noexcept;
};
HYBRID1D_END_NAMESPACE

#endif /* MomentRecorder_h */
