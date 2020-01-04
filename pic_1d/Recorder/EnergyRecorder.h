//
//  EnergyRecorder.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef EnergyRecorder_h
#define EnergyRecorder_h

#include "./Recorder.h"

#include <fstream>
#include <string>

PIC1D_BEGIN_NAMESPACE
/// spatial average of field and ion energy density recorder
/// field-aligned components are recorded;
/// suffix 1, 2, and 3 means 3 field-aligned components:
///     1 : parallel, 2 : perpendicular, and 3 : out-of-plane
///
class EnergyRecorder : public Recorder {
    std::ofstream os;

    std::string filepath() const;
public:
    explicit EnergyRecorder(unsigned const rank, unsigned const size);

private:
    void record(Domain const &domain, long const step_count) override;

    static Vector dump(BField const &bfield) noexcept;
    static Vector dump(EField const &efield) noexcept;
    static Tensor dump(Species const &sp) noexcept;
};
PIC1D_END_NAMESPACE

#endif /* EnergyRecorder_h */
