//
//  Driver.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Driver_h
#define Driver_h

#include "./Module/Domain.h"
#include "./Recorder/Recorder.h"
#include "./Boundary/MasterDelegate.h"
#include "./Boundary/SubdomainDelegate.h"

#include <future>
#include <memory>
#include <string>
#include <array>
#include <map>

PIC1D_BEGIN_NAMESPACE
class [[nodiscard]] Driver {
    long iteration_count{};
    std::unique_ptr<Domain> domain;
    std::unique_ptr<MasterDelegate> master;
    std::unique_ptr<SubdomainDelegate> delegate;
    std::map<std::string, std::unique_ptr<Recorder>> recorders;

    struct [[nodiscard]] Worker {
        std::future<void> handle;
        std::unique_ptr<Domain> domain;
        //
        ~Worker();
        void operator()() const;
        Worker() noexcept = default;
        Worker(Worker&&) noexcept = default;
    };
    std::array<Worker, ParamSet::number_of_particle_parallism - 1> workers;

public:
    ~Driver();
    Driver(unsigned const rank, unsigned const size);

    void operator()();

    Driver(Driver&&) noexcept = default;
};
PIC1D_END_NAMESPACE

#endif /* Driver_h */
