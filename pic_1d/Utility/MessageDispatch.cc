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
#include <numeric>
#include <chrono>
#include <future>
#include <array>

using P1D::MessageDispatch;

#if defined(DEBUG)
namespace {
    void dispatch_test_1() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        struct S { std::string s; };
        S const s1{__FUNCTION__};
        MessageDispatch<long, std::unique_ptr<std::string>, S> q;
        auto tk1 = q.send<0>(long{1}, {0, 1});
        auto tk2 = q.send(std::make_unique<std::string>(__FUNCTION__), {0, 1});
        auto tk3 = q.send(s1, {0, 1});
        long const i = q.recv<long>({0, 1});
        auto const str = **q.recv<1>({0, 1});
        S const s2 = q.recv<S>({0, 1});
        std::move(tk1).wait();
        std::move(tk2).wait();
        std::move(tk3).wait();
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
    void dispatch_test_3() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        MessageDispatch<long> q;
        auto f1 = std::async(std::launch::async, [&q]{
            auto tk = q.send<0>(1, {1, 1});
            //std::move(tk).wait();
            println(std::cerr, "1st msg sent");
        });
        auto f2 = std::async(std::launch::async, [&q]{
            q.send<0>(2, {1, 1}).wait();
            println(std::cerr, "2nd msg sent");
        });
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
        }
        q.recv<0>({1, 1}).unpack([](long const payload) {
            println(std::cerr, "msg recv'ed: ", payload);
        });
        q.recv<0>({1, 1}).unpack([](long const payload) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1s);
            println(std::cerr, "msg recv'ed: ", payload);
        });

        f1.get(), f2.get();
    }
    void dispatch_test_4() { // collective communication test
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        MessageDispatch<std::unique_ptr<long>, long> dispatch;
        using Env = decltype(dispatch)::Envelope;
        using Payloads = std::map<Env, std::unique_ptr<long>>;
        constexpr long magic = 6;

        auto f1 = std::async(std::launch::async, [&dispatch]{
            dispatch.send<0>(dispatch.recv<0>({0, 1}), {1, 0}).wait();
            if (magic != *dispatch.recv<long>({0, 1})) {
                throw std::runtime_error{__PRETTY_FUNCTION__};
            }
        });
        auto f2 = std::async(std::launch::async, [&dispatch]{
            dispatch.send<0>(dispatch.recv<0>({0, 2}), {2, 0}).wait();
            if (magic != *dispatch.recv<long>({0, 2})) {
                throw std::runtime_error{__PRETTY_FUNCTION__};
            }
        });

        Payloads payloads; {
            payloads[{0, 0}] = std::make_unique<long>(1);
            payloads[{0, 1}] = std::make_unique<long>(2);
            payloads[{0, 2}] = std::make_unique<long>(3);
        }
        dispatch.scatter(std::move(payloads)).clear();
        (void)dispatch.send<0>(dispatch.recv<0>({0, 0}), {0, 0});
        long const sum = [](auto pkgs) {
            return std::accumulate(std::make_move_iterator(begin(pkgs)),
                                   std::make_move_iterator(end(pkgs)),
                                   long{}, [](long const &a, Payloads::mapped_type b) {
                return a + *b;
            });
        }(dispatch.gather<0>({Env{0, 0}, Env{1, 0}, Env{2, 0}}));
        dispatch.bcast(sum, {Env{0, 0}, Env{0, 1}, Env{0, 2}}).clear();
        if (magic != *dispatch.recv<long>({0, 0})) {
            throw std::runtime_error{__PRETTY_FUNCTION__};
        }

        f1.get(), f2.get();
    }
}
void P1D::test_message_queue() {
    dispatch_test_1();
    dispatch_test_2();
    dispatch_test_3();
    dispatch_test_4();
}
#else
void P1D::test_message_queue() {
}
#endif

#if defined(DEBUG)
namespace {
    void comm_test_1() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        struct S { long i; };
        MessageDispatch<S> md;
        auto const comm = md.comm(1U);

        auto tk = comm.send<S const&>(S{1}, 1);
        S const s = *comm.recv<S>(1);
        std::move(tk).wait();
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
    void comm_test_4() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 4, magic = 10;
        MessageDispatch<std::unique_ptr<long>, long> md;
        std::array<std::future<long>, N> flist;
        for (unsigned i = 0; i < flist.size(); ++i) {
            flist[i] = std::async(std::launch::async, [&md](unsigned const rank){
                constexpr unsigned master = 0;
                auto const comm = md.comm(rank);
                if (master == rank) { // master
                    std::set<unsigned> extents;
                    std::map<unsigned, std::unique_ptr<long>> payloads;
                    for (unsigned rank = 0; rank < N; ++rank) {
                        extents.emplace_hint(extents.end(), rank);
                        payloads.try_emplace(payloads.end(), rank, std::make_unique<long>(rank + 1));
                    }
                    comm.scatter(std::move(payloads)).clear();
                    long const sum = [](auto pkgs) {
                        return std::accumulate(std::make_move_iterator(begin(pkgs)),
                                               std::make_move_iterator(end(pkgs)),
                                               long{}, [](long const &a, auto b) {
                            return a + std::move(b).unpack([](auto ptr){ return *ptr; });
                        });
                    }(comm.gather<0>(extents));
                    comm.bcast(sum, extents).clear();
                } else { // worker
                    comm.send(*comm.recv<0>(master), master).wait();
                }
                return *comm.recv<long>(master);
            }, i);
        }
        for (auto &f : flist) {
            if (magic != f.get()) {
                throw std::runtime_error{__PRETTY_FUNCTION__};
            }
        }
    }
    void comm_test_5() {
        println(std::cout, "in ", __PRETTY_FUNCTION__);

        constexpr int N = 4, magic = 10;
        MessageDispatch<int, std::string, char const*> md;
        std::set<int> participants;
        for (int i = 1; i <= N; ++i) {
            (void)md.comm(i).send<0>(*participants.insert(i).first, 0);
            (void)md.comm(i).send("a", 0);
            (void)md.comm(i).send(std::to_string(i), 0);
        }
        long const sum = *md.comm(0).reduce<0>(participants, std::make_unique<int>(0), [](auto a, auto b){ *b += a; return b; });
        if (magic != sum) {
            throw std::runtime_error{__PRETTY_FUNCTION__};
        }
        md.comm(0).for_each<char const*>(participants, &std::puts);
        md.comm(0).for_each<std::string>(participants, [](std::string s) { println(std::cout, s); });
    }
}
void P1D::test_inter_thread_comm() {
    comm_test_1();
    comm_test_2();
    comm_test_3();
    comm_test_4();
    comm_test_5();
}
#else
void P1D::test_inter_thread_comm() {
}
#endif
