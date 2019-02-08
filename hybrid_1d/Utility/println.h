//
//  println.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/30/19.
//  Copyright © 2019 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef println_h
#define println_h

#include <initializer_list>
#include <utility>
#include <ostream>

// synthetic sugar for output stream operator (<<)
//
namespace {
    template <class CharT, class Traits, class... Args>
    std::basic_ostream<CharT, Traits> &print(std::basic_ostream<CharT, Traits> &os, Args&&... args) {
        (void)std::initializer_list<int>{
            (os << std::forward<Args>(args), 0)...
        }; // a neat trick for parameter pack expansion
        return os;
    }
    template <class CharT, class Traits, class... Args>
    std::basic_ostream<CharT, Traits> &println(std::basic_ostream<CharT, Traits> &os, Args&&... args) {
        return print(os, std::forward<Args>(args)...) << '\n';
    }
}

#endif /* println_h */
