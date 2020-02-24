//
//  main.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./Driver.h"
#include "./Utility/println.h"
#include "./Utility/lippincott.h"
#include "./InputWrapper.h"

#include <array>
#include <chrono>
#include <future>
#include <utility>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <type_traits>

#if defined(DEBUG)
#include "./VDF/LossconeVDF.h"
#include "./VDF/BitReversedPattern.h"
#include "./Utility/MessageDispatch.h"
#include "./Utility/Options.h"
#endif

namespace {
    template <class F, class... Args>
    void measure(F&& f, Args&&... args) {
        static_assert(std::is_invocable_v<F&&, Args&&...>);
        auto const start = std::chrono::steady_clock::now();
        {
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }
        auto const end = std::chrono::steady_clock::now();
        std::chrono::duration<double> const diff = end - start;
        println(std::cout, "%% time elapsed: ", diff.count(), 's');
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] const char * argv[])
try {
    using namespace P1D;
    {
        constexpr unsigned size = Input::number_of_subdomains;
        auto task = [opts = Options{{argv, argv + argc}}](unsigned const rank) {
            // construction of Driver should be done on their own thread
            return Driver{rank, size, {rank, opts}}();
        };
        //
        std::array<std::future<void>, size> workers;
        std::packaged_task<void(unsigned)> main_task{task};
        workers.at(0) = main_task.get_future();
        for (unsigned rank = 1; rank < size; ++rank) {
            workers.at(rank) = std::async(std::launch::async, task, rank);
        }
        measure(main_task, 0);
        //
        for (auto &f : workers) {
            f.get();
        }
    }
    //
#if defined(DEBUG)
//    test_BitReversedPattern();
//    test_message_queue();
//    test_inter_thread_comm();
//    test_option_parser();
//    test_LossconeVDF();
#endif
    //
    return 0;
} catch (...) {
    lippincott();
}
