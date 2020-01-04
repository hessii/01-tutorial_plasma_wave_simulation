//
//  MessageDispatch.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/3/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "MessageDispatch.h"
#include "../Utility/println.h"

#include <iostream>
#include <memory>
#include <chrono>
#include <array>

using P1D::MessageDispatch;

namespace {
    void test_1() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        MessageDispatch<long, std::unique_ptr<std::string>> q;
        auto tk1 = q.send<0>({0, 1}, long{1});
        auto tk2 = q.send({0, 1}, std::make_unique<std::string>(__PRETTY_FUNCTION__));
        long const i = q.recv<long>({0, 1});
        auto const s = **q.recv<1>({0, 1});
        tk1.wait();
        tk2.wait();
        println(std::cout, i, ", ", s);
    }
    void test_2() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 10;
        MessageDispatch<std::unique_ptr<long>, long> q;
        auto f = [&q](int const i) -> long {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
            q.send({i + 1, 0}, std::make_unique<long>(i + 1)).wait();
            std::this_thread::sleep_for(1s);
            return q.recv<long>({0, i + 1});
        };

        std::array<std::future<long>, N> flist;
        for (unsigned i = 0; i < N; ++i) {
            flist[i] = std::async(std::launch::async, f, i);
        }

        long sum = 0;
        for (int i = 0; i < N; ++i) {
            q.recv<0>({i + 1, 0}).unpack([](auto&& ptr, long& sum){ sum += *ptr; }, sum);
        }
        for (unsigned i = 0; i < N; ++i) {
            q.send({0U, i + 1}, static_cast<long const>(sum)).wait();
        }

        for (auto &f : flist) {
            println(std::cout, f.get());
        }
    }
}

void P1D::test_message_queue() {
    test_1();
    test_2();
}
