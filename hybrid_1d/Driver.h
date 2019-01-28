//
//  Driver.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef Driver_h
#define Driver_h

#include "./Module/Delegate.h"
#include "./Module/Domain.h"
#include "./Recorder/Recorder.h"
#include "./Inputs.h"

#include <memory>
#include <string>
#include <map>

HYBRID1D_BEGIN_NAMESPACE
class Driver : public Delegate {
    long step_count{};
    std::unique_ptr<Domain> domain;
    std::map<std::string, std::unique_ptr<Recorder>> recorders;

public:
    ~Driver();
    explicit Driver();

    void run();

private:
    void dump_energy();
    void dump_fields();
    void dump_moment();
    void dump_vdf();
    void dump_particles();
};
HYBRID1D_END_NAMESPACE

#endif /* Driver_h */
