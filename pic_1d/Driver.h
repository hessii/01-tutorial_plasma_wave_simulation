//
//  Driver.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Driver_h
#define Driver_h

#include "./Core/Domain.h"
#include "./Recorder/Recorder.h"
#include "./Boundary/MasterDelegate.h"
#include "./Boundary/SubdomainDelegate.h"
#include "./ParamSet.h"

#include <future>
#include <memory>
#include <string>
#include <array>
#include <map>

PIC1D_BEGIN_NAMESPACE
class [[nodiscard]] Driver {
    long iteration_count{};
    unsigned const rank, size;
    ParamSet const params;
    std::unique_ptr<Domain> domain;
    std::unique_ptr<MasterDelegate> master;
    std::unique_ptr<SubdomainDelegate> delegate;
    std::map<std::string, std::unique_ptr<Recorder>> recorders;

    struct [[nodiscard]] Worker {
        long iteration_count;
        Driver const *driver;
        WorkerDelegate *delegate;
        std::future<void> handle;
        std::unique_ptr<Domain> domain;
        //
        ~Worker();
        void operator()();
        Worker() noexcept = default;
        Worker(Worker&&) noexcept = default;
    };
    std::array<Worker, ParamSet::number_of_particle_parallelism - 1> workers;

public:
    ~Driver();
    Driver(unsigned const rank, unsigned const size, ParamSet const &params);
    Driver(Driver&&) = default;

    void operator()();
private:
    void master_loop();
    [[nodiscard]] static std::unique_ptr<Domain> make_domain(ParamSet const &params, Delegate *delegate);
};
PIC1D_END_NAMESPACE

#endif /* Driver_h */
