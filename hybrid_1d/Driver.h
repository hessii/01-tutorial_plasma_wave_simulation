//
//  Driver.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/28/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Driver_h
#define Driver_h

#include "./Module/Delegate.h"
#include "./Module/Domain.h"
#include "./Recorder/Recorder.h"

#include <memory>
#include <string>
#include <map>

HYBRID1D_BEGIN_NAMESPACE
class Driver : public Delegate {
    std::unique_ptr<Domain> domain;
    std::map<std::string, std::unique_ptr<Recorder>> recorders;

public:
    ~Driver();
    explicit Driver();

    void operator()() const;
};
HYBRID1D_END_NAMESPACE

#endif /* Driver_h */
