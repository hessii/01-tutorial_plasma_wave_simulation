//
//  println.h
//  hybrid_1d
//
//  Created by KYUNGGUK MIN on 1/30/19.
//  Copyright © 2019 kyungguk.com. All rights reserved.
//

#ifndef println_h
#define println_h

#include <utility>
#include <ostream>

// synthetic sugar for << operator
//
namespace {
    template <class CharT, class Traits, class Arg>
    std::basic_ostream<CharT, Traits> &print(std::basic_ostream<CharT, Traits> &os, Arg&& arg) {
        return os << arg;
    }
    template <class CharT, class Traits, class First, class... Rest>
    std::basic_ostream<CharT, Traits> &print(std::basic_ostream<CharT, Traits> &os, First&& _1, Rest&&... _n) {
        return print(os << _1, std::forward<Rest>(_n)...);
    }
    template <class CharT, class Traits, class... Args>
    std::basic_ostream<CharT, Traits> &println(std::basic_ostream<CharT, Traits> &os, Args&&... args) {
        return print(os, std::forward<Args>(args)...) << '\n';
    }
}

#endif /* println_h */
