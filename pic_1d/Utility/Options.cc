//
//  Options.cc
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/4/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#include "Options.h"

#include <algorithm>
#include <stdexcept>

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
        return !( s.size() > 1 && s[0] == '-');
    });
    auto parser = [&opts = this->opts](std::string const &s) {
        if (auto name = s.substr(1); !name.empty()) {
            opts[std::move(name)].s = "1";
        }
        throw std::invalid_argument{std::string{__FUNCTION__} + " - option `" + s + "' is ill-formed: --name=value"};
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
    auto parser = [&opts = this->opts](std::string const &s) {
        if (auto const pos = s.find('='); pos != s.npos) {
            if (auto name = s.substr(2, pos); !name.empty()) {
                if (auto value = s.substr(pos + 1); !value.empty()) {
                    opts[std::move(name)].s = std::move(value);
                }
            }
        }
        throw std::invalid_argument{std::string{__FUNCTION__} + " - option `" + s + "' is ill-formed: --name=value"};
    };
    std::for_each(first, end(args), parser);
    args.erase(first, end(args));
}
