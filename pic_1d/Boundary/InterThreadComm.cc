//
//  InterThreadComm.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/3/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "InterThreadComm.h"
#include "../Utility/println.h"

#include <iostream>
#include <array>

using P1D::MessageDispatch;
using P1D::InterThreadComm_;

namespace {
    void test_1() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        MessageDispatch<long> md;
        InterThreadComm_<long> const comm{&md, 0};

        auto tk = comm.send(0, long{1});
        long const i = comm.recv<long>(0);
        tk.wait();
        println(std::cout, i);
    }
    void test_2() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 10;
        MessageDispatch<long> md;
        std::array<std::future<long>, N> flist;
        for (int i = 0; i < N; ++i) {
            flist[unsigned(i)] = std::async(std::launch::async, [&md](int i)->long {
                InterThreadComm_<long> comm(&md, i + 1);
                return comm.recv<long>(0);
            }, i);
        }
        for (int i = 0; i < N; ++i) {
            InterThreadComm_<long>{&md, 0}.send(i + 1, long{i}).wait();
        }
        for (auto &f : flist) {
            println(std::cout, f.get());
        }
    }
    void test_3() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 10;
        MessageDispatch<long> md;
        std::array<std::future<void>, N> flist;
        for (int i = 0; i < N; ++i) {
            flist[unsigned(i)] = std::async(std::launch::async, [&md](int i){
                InterThreadComm_<long> comm(&md, i + 1);
                comm.send(0, *comm.recv<long>(0)).wait();
            }, i);
        }
        for (int i = 0; i < N; ++i) {
            InterThreadComm_<long>{&md, 0}.send(i + 1, long{i}).wait();
        }
        for (int i = 0; i < N; ++i) {
            println(std::cout, *InterThreadComm_<long>{&md, 0}.recv<long>(i + 1));
        }
    }
}

void P1D::test_inter_thread_comm() {
    test_1();
    test_2();
    test_3();
}
