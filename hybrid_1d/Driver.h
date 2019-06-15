//
//  Driver.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Driver_h
#define Driver_h

#include "./Module/Domain.h"
#include "./Recorder/Recorder.h"
#include "./Boundary/MasterWrapper.h"

#include <future>
#include <memory>
#include <string>
#include <array>
#include <map>

HYBRID1D_BEGIN_NAMESPACE
class Driver {
    std::unique_ptr<Domain> domain;
    std::unique_ptr<MasterWrapper> master;
    std::map<std::string, std::unique_ptr<Recorder>> recorders;

    struct Worker {
        std::future<void> handle;
        std::unique_ptr<Domain> domain;

        ~Worker();
        void operator()() const;
    };
    std::array<Worker, Input::n_workers> workers;

public:
    ~Driver();
    explicit Driver();

    void operator()();
};
HYBRID1D_END_NAMESPACE

#endif /* Driver_h */
