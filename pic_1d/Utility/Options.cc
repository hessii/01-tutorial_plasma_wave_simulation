//
//  Options.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/4/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Options.h"
#include "../Utility/println.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>

P1D::Options::Options(std::vector<std::string> _args)
: args{std::move(_args)} {
    parse_short_options();
    parse_long_options();
}
void P1D::Options::parse_short_options()
{
    // parse short options whose form is -opt_name which is equivalent to --opt_name=1
    //
    auto const first = std::stable_partition(begin(args), end(args), [](std::string const &s) {
        return !( s.size() > 1 && (s[0] == '-' && s[1] != '-') );
    });
    auto parser = [&opts = this->opts](std::string s) -> void {
        s = s.substr(1);
        if (auto &&name = s; !name.empty()) {
            opts[std::move(name)].s = "1";
            return;
        }
        throw std::invalid_argument{std::string{__FUNCTION__} + " - option `" + s + "' is ill-formed: -name"};
    };
    std::for_each(first, end(args), parser);
    args.erase(first, end(args));
}
void P1D::Options::parse_long_options()
{
    // parse long options whose form is --opt_name=value
    //
    auto const first = std::stable_partition(begin(args), end(args), [](std::string const &s) {
        return !( s.size() > 2 && (s[0] == '-' & s[1] == '-') );
    });
    auto parser = [&opts = this->opts](std::string s) -> void {
        s = s.substr(2);
        if (auto const pos = s.find('='); pos != s.npos) {
            if (auto name = s.substr(0, pos); !name.empty()) {
                if (auto value = s.substr(pos + 1); !value.empty()) {
                    opts[std::move(name)].s = std::move(value);
                    return;
                }
            }
        }
        throw std::invalid_argument{std::string{__FUNCTION__} + " - option `" + s + "' is ill-formed: --name=value (no whitespace in name)"};
    };
    std::for_each(first, end(args), parser);
    args.erase(first, end(args));
}

void P1D::test_option_parser() {
#if defined(DEBUG)
    println(std::cout, "in ", __PRETTY_FUNCTION__);

    Options const opts{{
        "a", "-save", "b", "-", "--", "--load=1", "--long= -3", "--str= s"
    }};

    print(std::cout, "argument list = {");
    for (auto const &arg : opts.argment_list()) {
        print(std::cout, arg, ", ");
    }
    println(std::cout, "}\n");

    print(std::cout, "options = {");
    for (auto const &[key, val] : opts.option_list()) {
        print(std::cout, key, "->", val.cast<std::string>(), ", ");
    }
    println(std::cout, "}\n");

    println(std::cout, "string = ", opts["str"].cast<char const*>());
    println(std::cout, "save = ", opts["save"].cast<bool>());
    println(std::cout, "load = ", opts["load"].cast<bool>());
    println(std::cout, "long = ", opts["long"].cast<long>());
#endif
}
