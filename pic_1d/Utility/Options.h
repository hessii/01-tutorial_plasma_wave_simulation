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
#include <type_traits>

PIC1D_BEGIN_NAMESPACE
class [[nodiscard]] Options {
public:
    class Value {
        friend Options;
        std::string s;
    public:
        explicit operator std::string const &() const noexcept { return s; }
        explicit operator char const *() const noexcept { return s.c_str(); }
        explicit operator bool() const { return static_cast<int>(*this); }
        explicit operator int() const { return s.empty() ? false : std::stoi(s); }
        explicit operator long() const { return s.empty() ? false : std::stol(s); }
        explicit operator unsigned long() const { return s.empty() ? false : std::stoul(s); }
        explicit operator float() const { return s.empty() ? false : std::stof(s); }
        explicit operator double() const { return s.empty() ? false : std::stod(s); }
    public:
        template <class T> [[nodiscard]]
        T cast() const { return static_cast<std::decay_t<T>>(*this); };
        template <class T>
        void operator()(T *p) const { *p = this->template cast<T>(); };
    };
private:
    std::vector<std::string> args;
    std::map<std::string, Value> opts;

public:
    [[nodiscard]] std::vector<std::string> const &argment_list() const noexcept {
        return args; // list of non-option arguments
    }
    [[nodiscard]] std::map<std::string, Value> const &option_list() const noexcept {
        return opts;
    }
    [[nodiscard]] Value const &operator[](std::string s) const {
        return opts.at(std::move(s));
    }

    Options() noexcept = default;
    Options(std::vector<std::string> args);
private:
    void parse_short_options();
    void parse_long_options();
};

// not for public use
//
void test_option_parser();
PIC1D_END_NAMESPACE

#endif /* Options_h */
