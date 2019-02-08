//
//  main.cc
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./Driver.h"
#include "./Utility/println.h"

#include <chrono>
#include <utility>
#include <iostream>

namespace {
    template <class F>
    void measure(F &&callee) {
        auto const start = std::chrono::steady_clock::now();
        {
            std::forward<F>(callee)();
        }
        auto const end = std::chrono::steady_clock::now();
        std::chrono::duration<double> const diff = end - start;
        println(std::cout, "%% time elapsed: ", diff.count(), "s");
    }
}

int main(int argc, const char * argv[]) {
    {
        measure(H1D::Driver{});
    }
    return 0;
}
