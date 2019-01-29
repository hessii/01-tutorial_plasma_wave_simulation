//
//  EnergyRecorder.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/29/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef EnergyRecorder_h
#define EnergyRecorder_h

#include "./Recorder.h"

#include <fstream>
#include <string>

HYBRID1D_BEGIN_NAMESPACE
class EnergyRecorder : public Recorder {
    std::ofstream os;

    static std::string filepath();
public:
    explicit EnergyRecorder();

private:
    void record(Domain const &domain, long const step_count) override;

    static Vector dump(BField const &bfield) noexcept;
    static Vector dump(EField const &efield) noexcept;
    static Tensor dump(Species const &sp) noexcept;
};
HYBRID1D_END_NAMESPACE

#endif /* EnergyRecorder_h */
