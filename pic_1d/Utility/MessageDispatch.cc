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
    void dispatch_test_1() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        struct S { std::string s; };
        S const s1{__FUNCTION__};
        MessageDispatch<long, std::unique_ptr<std::string>, S> q;
        auto tk1 = q.send<0>(long{1}, {0, 1});
        auto tk2 = q.send(std::make_unique<std::string>(__FUNCTION__), {0, 1});
        auto tk3 = q.send(s1, {0, 1});
        S const s2 = q.recv<S>({0, 1});
        long const i = q.recv<long>({0, 1});
        auto const str = **q.recv<1>({0, 1});
        tk1.wait();
        tk2.wait();
        tk3.wait();
        println(std::cout, i, ", ", str, ", ", s2.s);
    }
    void dispatch_test_2() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 10;
        MessageDispatch<std::unique_ptr<long>, long> q;
        auto f = [&q](int const i) -> long {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
            q.send(std::make_unique<long>(i + 1), {i + 1, 0}).wait();
            std::this_thread::sleep_for(1s);
            return q.recv<long>({0, i + 1});
        };

        std::array<std::future<long>, N> flist;
        for (unsigned i = 0; i < N; ++i) {
            flist[i] = std::async(std::launch::async, f, i);
        }

        long sum = 0;
        for (int i = 0; i < N; ++i) {
            q.recv<0>({i + 1, 0}).unpack([](auto ptr, long& sum){ sum += *ptr; }, sum);
        }
        for (unsigned i = 0; i < N; ++i) {
            q.send(static_cast<long const>(sum), {0U, i + 1}).wait();
        }

        for (auto &f : flist) {
            println(std::cout, f.get());
        }
    }
}
void P1D::test_message_queue() {
    dispatch_test_1();
    dispatch_test_2();
}

namespace {
    void comm_test_1() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        struct S { long i; };
        MessageDispatch<S> md;
        auto const comm = md.comm(1U);

        auto tk = comm.send<S const&>(S{1}, 1);
        S const s = *comm.recv<S>(1);
        tk.wait();
        println(std::cout, s.i);
    }
    void comm_test_2() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 10;
        MessageDispatch<long> md;
        std::array<std::future<long>, N> flist;
        for (unsigned i = 0; i < flist.size(); ++i) {
            flist[i] = std::async(std::launch::async, [&md](unsigned i)->long {
                auto const comm = md.comm(i + 1);
                return comm.recv<long>(0U);
            }, i);
        }
        for (int i = 0; i < N; ++i) {
            md.comm(0U).send(long{i}, i + 1).wait();
        }
        for (auto &f : flist) {
            println(std::cout, f.get());
        }
    }
    void comm_test_3() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 10;
        MessageDispatch<long> md;
        std::array<std::future<void>, N> flist;
        for (unsigned i = 0; i < flist.size(); ++i) {
            flist[i] = std::async(std::launch::async, [&md](unsigned i){
                auto const comm = md.comm(i + 1);
                comm.send(*comm.recv<long>(0), 0).wait();
            }, i);
        }
        for (int i = 0; i < N; ++i) {
            md.comm(0).send(long{i}, i + 1).wait();
        }
        for (int i = 0; i < N; ++i) {
            println(std::cout, *md.comm(0).recv<long>(i + 1));
        }
    }
}
void P1D::test_inter_thread_comm() {
    comm_test_1();
    comm_test_2();
    comm_test_3();
}
