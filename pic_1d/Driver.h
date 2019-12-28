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
#include "./Boundary/Delegate.h"

#include <memory>
#include <string>
#include <map>

PIC1D_BEGIN_NAMESPACE
class [[nodiscard]] Driver {
    std::unique_ptr<Domain> domain;
    std::unique_ptr<Delegate> delegate;
    std::map<std::string, std::unique_ptr<Recorder>> recorders;
    long iteration_count{};

public:
    ~Driver();
    explicit Driver();

    void operator()();
};
PIC1D_END_NAMESPACE

#endif /* Driver_h */
