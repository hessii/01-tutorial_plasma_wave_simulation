//
//  Options.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 2/4/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef Options_h
#define Options_h

#include "../Macros.h"

#include <map>
#include <vector>
#include <string>
#include <utility>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <type_traits>

PIC1D_BEGIN_NAMESPACE
/// option parser from command-line arguments
///
/// Parsed are short-style options in the form '-opt_name', which are interpreted as boolean true (represented numeric numbers other than 0), and
/// long-style options in the form '--opt_name=value' or '--opt_name value', which are interpreted as key-value pairs.
/// Values in the second form of the long-style options must not preceed with '--'.
///
/// Any number of leading/trailing, but not interspersed, whitespaces in 'opt_name' and 'value' are removed before parsing.
/// An empty string, after removing the whitespaces, as opt_name and/or value is ill-formed.
///
class [[nodiscard]] Options {
public:
    enum Style : long {
        short_ = 1, //!< tag for short-style option
        long_ = 2 //!< tag for long-style option
    };

    // option value parser
    //
    struct Value {
        friend Options;
        std::string s;
        Style style{long_};
    public: // cast operators
        explicit operator std::string const &() const noexcept { return s; }
        explicit operator char const *() const noexcept { return s.c_str(); }
        explicit operator bool() const { return static_cast<int>(*this); }
        explicit operator int() const { return std::stoi(s); }
        explicit operator long() const { return std::stol(s); }
        explicit operator unsigned long() const { return std::stoul(s); }
        explicit operator float() const { return std::stof(s); }
        explicit operator double() const { return std::stod(s); }
    public:
        template <class T> [[nodiscard]]
        auto cast() const { return static_cast<std::decay_t<T>>(*this); };
        template <class T>
        void operator()(T *p) const { *p = this->template cast<T>(); };
    };

private:
    std::map<std::string, Value> opts;

public:
    std::map<std::string, Value> const *operator->() const& noexcept {
        return &opts;
    }
    std::map<std::string, Value> const &operator*() const& noexcept {
        return opts;
    }

    Options() noexcept = default;
    Options(std::vector<std::string> args) { parse(std::move(args)); }

    /// parses options in the argument list and returns unparsed, order-preserved, arguments
    ///
    /// multiple calls will override/append to the options already parsed previously
    ///
    std::vector<std::string> parse(std::vector<std::string> args);
private:
    std::vector<std::string> parse_short_options(std::vector<std::string> args);
    std::vector<std::string> parse_long_options(std::vector<std::string> args);

    // pretty print
    //
    template <class CharT, class Traits>
    friend decltype(auto) operator<<(std::basic_ostream<CharT, Traits> &os, Options const &opts) {
        auto const printer = [](decltype(os) os, auto const &kv) -> decltype(auto) {
            auto const &[key, val] = kv;
            return os << key << " : " << val.s;
        };
        os << '{';
        if (!opts->empty()) {
            printer(os, *begin(*opts));
            std::for_each(std::next(begin(*opts)), end(*opts), [&os, printer](auto const &kv) { printer(os << ", ", kv); });
        }
        return os << '}';
    }
};

// not for public use
//
void test_option_parser();
PIC1D_END_NAMESPACE

#endif /* Options_h */
