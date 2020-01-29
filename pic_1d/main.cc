//
//  main.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/14/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "./Driver.h"
#include "./Utility/println.h"
#include "./InputWrapper.h"

#include <set>
#include <array>
#include <chrono>
#include <future>
#include <cstdlib> // std::abort
#include <utility>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <string_view>

#include "./VDF/BitReversedPattern.h"
#include "./Utility/MessageDispatch.h"

std::set<std::string_view> cmd_arg_set;

[[noreturn]] void lippincott() noexcept
try {
    throw;
} catch (std::exception const &e) {
    println(std::cerr, "Uncaught exception: \n\t", e.what());
    std::abort();
} catch (...) {
    println(std::cerr, "Unknown exception");
    std::abort();
}
namespace {
    void parse_cmd_args(int argc, const char * argv[]) {
        for (int i = 1; i < argc; ++i) {
            cmd_arg_set.emplace(argv[i]);
        }
        if (cmd_arg_set.count("-resume")) {
            cmd_arg_set.emplace("-load");
        }
    }
    //
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
    parse_cmd_args(argc, argv);
    //
    {
        constexpr unsigned size = P1D::ParamSet::number_of_subdomains;
        auto task = [](unsigned const rank) {
            // construction of Driver should be done on their own thread
            return P1D::Driver{rank, size}();
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
//    P1D::test_BitReversedPattern();
//    P1D::test_message_queue();
//    P1D::test_inter_thread_comm();
    //
    return 0;
} catch (...) {
    lippincott();
}
