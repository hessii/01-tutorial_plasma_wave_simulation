//
//  Options.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/4/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Options.h"
#include "../Utility/println.h"

#include <stdexcept>
#include <iostream>

namespace {
    [[nodiscard]] std::string trim(std::string s) noexcept(noexcept(s.erase(end(s), end(s)))) {
        auto const pred = [](auto const c) noexcept { return ' ' != c; };
        s.erase(begin(s), std::find_if(begin(s), end(s), pred)); // leading whitespace(s)
        s.erase(std::find_if(rbegin(s), rend(s), pred).base(), end(s)); // trailing whitespace(s)
        return s;
    }
}
std::vector<std::string> P1D::Options::parse(std::vector<std::string> args)
{
    args = transform_long_style(std::move(args));
    args = parse_short_options(std::move(args), opts);
    args = parse_long_options(std::move(args), opts);
    return args;
}
std::vector<std::string> P1D::Options::transform_long_style(std::vector<std::string> args)
{
    if (size(args) > 1) { // at least two entries
        for (auto key = rbegin(args), val = key++;
             key != rend(args); val = key++) {
            if (key->size() > 2 && ((*key)[0] == '-' & (*key)[1] == '-') &&
                key->find('=') == key->npos) {
                if (val->size() >= 2 && ((*val)[0] == '-' & (*val)[1] == '-')) {
                    throw std::invalid_argument{std::string{__FUNCTION__} + " - long-style option `" + *key + ' ' + *val + "' is ill-formed"};
                }
                // append value part to the key and erase it
                //
                (*key += '=') += *val;
                args.erase(key.base());
            }
        }
    }
    return args;
}
std::vector<std::string> P1D::Options::parse_short_options(std::vector<std::string> args, std::map<std::string, Value> &opts)
{
    // parse short options whose form is -opt_name which is equivalent to --opt_name=1
    //
    auto const first = std::stable_partition(begin(args), end(args), [](std::string const &s) {
        return !( s.size() > 1 && (s[0] == '-' && s[1] != '-') );
    });
    auto parser = [&opts](std::string const &s) -> void {
        if (auto name = trim(s.substr(1)); !name.empty()) {
            opts[std::move(name)] = {"1", short_};
            return;
        }
        throw std::invalid_argument{std::string{__FUNCTION__} + " - short-style option `" + s + "' is ill-formed"};
    };
    std::for_each(first, end(args), parser);
    args.erase(first, end(args));
    //
    return args;
}
std::vector<std::string> P1D::Options::parse_long_options(std::vector<std::string> args, std::map<std::string, Value> &opts)
{
    // parse long options whose form is --opt_name=value
    //
    auto const first = std::stable_partition(begin(args), end(args), [](std::string const &s) {
        return !( s.size() > 2 && (s[0] == '-' & s[1] == '-') );
    });
    auto parser = [&opts](std::string s) -> void {
        s = s.substr(2);
        if (auto const pos = s.find('='); pos != s.npos) {
            if (auto name = trim(s.substr(0, pos)); !name.empty()) {
                if (auto value = trim(s.substr(pos + 1)); !value.empty()) {
                    opts[std::move(name)] = {std::move(value), long_};
                    return;
                }
            }
        }
        throw std::invalid_argument{std::string{__FUNCTION__} + " - long-style option `--" + s + "' is ill-formed"};
    };
    std::for_each(first, end(args), parser);
    args.erase(first, end(args));
    //
    return args;
}

void P1D::test_option_parser() {
#if defined(DEBUG)
    println(std::cout, "in ", __PRETTY_FUNCTION__);

    Options opts{{"--save=0", "--long=3", "--dir", "~"}};
    auto const unparsed = opts.parse({{"a", "- save  ", "b", "-", "--", "--load=0", "--long = -3", "--str= s", "-abc xyz"}});

    print(std::cout, "unparsed arguments = \"");
    for (auto const &arg : unparsed) {
        print(std::cout, arg, ", ");
    }
    println(std::cout, "\"\n");

    println(std::cout, "options = ", opts);

    println(std::cout, "string = ", opts->at("str").cast<char const*>());
    println(std::cout, "save = ", opts->at("save").cast<bool>());
    println(std::cout, "load = ", opts->at("load").cast<bool>());
    println(std::cout, "long = ", opts->at("long").cast<long>());
    println(std::cout, "dir = ", opts->at("dir").cast<char const*>());
#endif
}
