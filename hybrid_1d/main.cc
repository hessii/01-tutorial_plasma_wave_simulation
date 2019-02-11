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
#include <stdexcept>

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
    try {
        measure(H1D::Driver{});
    } catch (std::exception const &e) {
        println(std::cerr, "Uncaught exception: \n\t", e.what());
    } catch (...) {
        println(std::cerr, "Unknown exception");
    }
    return 0;
}
