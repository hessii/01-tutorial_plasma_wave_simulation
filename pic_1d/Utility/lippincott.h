//
//  lippincott.h
//  pic_1d
//
//  Created by KYUNGGUK MIN on 1/31/20.
//  Copyright © 2020 Kyungguk Min & Kaijun Liu. All rights reserved.
//

#ifndef lippincott_h
#define lippincott_h

#include <exception>
#include <cstdlib>
#include <string>

// error handling routines
//
namespace {
    [[noreturn, maybe_unused]] void fatal_error(char const *reason) noexcept {
        std::puts(reason);
        std::abort();
    }
    [[noreturn, maybe_unused]] void fatal_error(std::string const &reason) noexcept {
        fatal_error(reason.c_str());
    }
    [[noreturn, maybe_unused]] void lippincott() noexcept
    try {
        throw;
    } catch (std::exception const &e) {
        fatal_error(e.what());
    } catch (...) {
        fatal_error("Unknown exception");
    }
}

#endif /* lippincott_h */
