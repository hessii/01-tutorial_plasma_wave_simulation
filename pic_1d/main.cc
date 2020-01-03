//
//  main.cc
//  pic_1d
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
#include <functional>

//#include "VDF/BitReversedPattern.h"

namespace {
    template <class F>
    void measure(F &&callee) {
        auto const start = std::chrono::steady_clock::now();
        {
            std::invoke(std::forward<F>(callee));
        }
        auto const end = std::chrono::steady_clock::now();
        std::chrono::duration<double> const diff = end - start;
        println(std::cout, "%% time elapsed: ", diff.count(), 's');
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] const char * argv[]) {
    try {
        measure(P1D::Driver{});
//        P1D::test_BitReversedPattern();
//        P1D::test_message_queue();
    } catch (std::exception const &e) {
        println(std::cerr, "Uncaught exception: \n\t", e.what());
    } catch (...) {
        println(std::cerr, "Unknown exception");
    }
    return 0;
}
